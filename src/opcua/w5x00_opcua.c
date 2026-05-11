/**
 * Minimal OPC UA Binary server for WIZnet RP2040-HAT-FREERTOS-C.
 *
 * Test node: ns=2;s=TestValue
 * Value: Int32 42
 * Endpoint: opc.tcp://<board-ip>:4840
 * Security: None, anonymous session
 *
 * This is a compact demo implementation. It is intentionally limited to the
 * services needed to open an unencrypted channel/session and read one value:
 * HEL/ACK, OpenSecureChannel, FindServers, GetEndpoints, CreateSession,
 * ActivateSession, Read, CloseSession/CloseSecureChannel.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <FreeRTOS.h>
#include <task.h>

// #include "port_common.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico/critical_section.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "hardware/clocks.h"

#include "wizchip_conf.h"
#include "wizchip_spi.h"
#include "socket.h"
#include "timer.h"

/* -------------------------------------------------------------------------------------------------
 * Configuration
 * ------------------------------------------------------------------------------------------------- */
#define OPCUA_TASK_STACK_SIZE 4096
#define OPCUA_TASK_PRIORITY   10

#define PLL_SYS_KHZ (133 * 1000)

#define SOCKET_OPCUA 0
#define PORT_OPCUA   4840

#define UA_RX_BUF_SIZE 8192
#define UA_TX_BUF_SIZE 8192

#define UA_RECV_TIMEOUT_MS 5000
#define UA_SEND_TIMEOUT_MS 5000

#define UA_CHANNEL_ID 1u
#define UA_TOKEN_ID   1u

#define UA_NODE_TEST_NS      2u
#define UA_NODE_TEST_STRING  "TestValue"
#define UA_TEST_VALUE        43

#define UA_SECURITY_POLICY_NONE "http://opcfoundation.org/UA/SecurityPolicy#None"
#define UA_TRANSPORT_PROFILE    "http://opcfoundation.org/UA-Profile/Transport/uatcp-uasc-uabinary"
#define UA_APP_URI              "urn:rp2040:opcua:demo"
#define UA_PRODUCT_URI          "urn:rp2040:opcua:demo"
#define UA_APP_NAME             "RP2040 OPC UA Demo"

/* OPC UA service DefaultBinary NodeIds */
#define UA_NS0ID_SERVICEFAULT_RESPONSE     397u
#define UA_NS0ID_FINDSERVERS_REQUEST       422u
#define UA_NS0ID_FINDSERVERS_RESPONSE      425u
#define UA_NS0ID_GETENDPOINTS_REQUEST      428u
#define UA_NS0ID_GETENDPOINTS_RESPONSE     431u
#define UA_NS0ID_OPENSECURECHANNEL_REQUEST 446u
#define UA_NS0ID_OPENSECURECHANNEL_RESPONSE 449u
#define UA_NS0ID_CREATESESSION_REQUEST     461u
#define UA_NS0ID_CREATESESSION_RESPONSE    464u
#define UA_NS0ID_ACTIVATESESSION_REQUEST   467u
#define UA_NS0ID_ACTIVATESESSION_RESPONSE  470u
#define UA_NS0ID_CLOSESESSION_REQUEST      473u
#define UA_NS0ID_CLOSESESSION_RESPONSE     476u
#define UA_NS0ID_READ_REQUEST              631u
#define UA_NS0ID_READ_RESPONSE             634u

/* StatusCodes used by the demo */
#define UA_STATUS_GOOD                  0x00000000u
#define UA_STATUS_BAD_SERVICEUNSUPPORTED 0x800B0000u
#define UA_STATUS_BAD_DECODINGERROR     0x80070000u
#define UA_STATUS_BAD_NODEIDUNKNOWN     0x80340000u
#define UA_STATUS_BAD_NOTREADABLE       0x803A0000u

#define UA_ATTRIBUTEID_VALUE 13u

/* Network */
static wiz_NetInfo g_net_info =
    {
        .mac = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56},
        .ip = {192, 168, 1, 2},
        .sn = {255, 255, 255, 0},
        .gw = {192, 168, 1, 1},
        .dns = {8, 8, 8, 8},
        .dhcp = NETINFO_STATIC
};

static uint8_t g_rx_buf[UA_RX_BUF_SIZE];
static uint8_t g_tx_buf[UA_TX_BUF_SIZE];
static volatile uint32_t g_msec_cnt = 0;

static uint32_t g_secure_sequence_number = 1;
static char g_endpoint_url[96] = "opc.tcp://192.168.1.2:4840";

/* -------------------------------------------------------------------------------------------------
 * Small OPC UA binary encoder/decoder
 * ------------------------------------------------------------------------------------------------- */
typedef struct
{
    uint8_t *data;
    uint32_t len;
    uint32_t cap;
    bool overflow;
} ua_buf_t;

typedef struct
{
    uint8_t encoding;
    uint16_t ns;
    uint32_t numeric;
    char string_id[48];
} ua_nodeid_t;

static void ua_buf_init(ua_buf_t *b, uint8_t *data, uint32_t cap)
{
    b->data = data;
    b->len = 0;
    b->cap = cap;
    b->overflow = false;
}

static void ua_put_bytes(ua_buf_t *b, const uint8_t *src, uint32_t n)
{
    if (b->overflow || b->len + n > b->cap)
    {
        b->overflow = true;
        return;
    }

    if (n > 0 && src != NULL)
    {
        memcpy(&b->data[b->len], src, n);
    }
    else if (n > 0)
    {
        memset(&b->data[b->len], 0, n);
    }

    b->len += n;
}

static void ua_put_u8(ua_buf_t *b, uint8_t v)
{
    ua_put_bytes(b, &v, 1);
}

static void ua_put_u16(ua_buf_t *b, uint16_t v)
{
    uint8_t tmp[2] = {(uint8_t)(v & 0xff), (uint8_t)((v >> 8) & 0xff)};
    ua_put_bytes(b, tmp, 2);
}

static void ua_put_u32(ua_buf_t *b, uint32_t v)
{
    uint8_t tmp[4] = {
        (uint8_t)(v & 0xff),
        (uint8_t)((v >> 8) & 0xff),
        (uint8_t)((v >> 16) & 0xff),
        (uint8_t)((v >> 24) & 0xff)};
    ua_put_bytes(b, tmp, 4);
}

static void ua_put_i32(ua_buf_t *b, int32_t v)
{
    ua_put_u32(b, (uint32_t)v);
}

static void ua_put_u64(ua_buf_t *b, uint64_t v)
{
    for (uint32_t i = 0; i < 8; i++)
    {
        ua_put_u8(b, (uint8_t)((v >> (8 * i)) & 0xff));
    }
}

static void ua_put_double(ua_buf_t *b, double v)
{
    union
    {
        double d;
        uint8_t bytes[8];
    } u;
    u.d = v;
    ua_put_bytes(b, u.bytes, 8);
}

static void ua_patch_u32(ua_buf_t *b, uint32_t offset, uint32_t v)
{
    if (offset + 4 > b->len)
    {
        b->overflow = true;
        return;
    }

    b->data[offset + 0] = (uint8_t)(v & 0xff);
    b->data[offset + 1] = (uint8_t)((v >> 8) & 0xff);
    b->data[offset + 2] = (uint8_t)((v >> 16) & 0xff);
    b->data[offset + 3] = (uint8_t)((v >> 24) & 0xff);
}

static void ua_put_string(ua_buf_t *b, const char *s)
{
    if (s == NULL)
    {
        ua_put_i32(b, -1);
        return;
    }

    uint32_t len = (uint32_t)strlen(s);
    ua_put_i32(b, (int32_t)len);
    ua_put_bytes(b, (const uint8_t *)s, len);
}

static void ua_put_bytestring(ua_buf_t *b, const uint8_t *data, int32_t len)
{
    if (data == NULL || len < 0)
    {
        ua_put_i32(b, -1);
        return;
    }

    ua_put_i32(b, len);
    ua_put_bytes(b, data, (uint32_t)len);
}

static void ua_put_nodeid_numeric(ua_buf_t *b, uint16_t ns, uint32_t id)
{
    if (ns == 0 && id <= 255)
    {
        ua_put_u8(b, 0x00); /* TwoByte NodeId */
        ua_put_u8(b, (uint8_t)id);
    }
    else if (ns <= 255 && id <= 65535)
    {
        ua_put_u8(b, 0x01); /* FourByte NodeId */
        ua_put_u8(b, (uint8_t)ns);
        ua_put_u16(b, (uint16_t)id);
    }
    else
    {
        ua_put_u8(b, 0x02); /* Numeric NodeId */
        ua_put_u16(b, ns);
        ua_put_u32(b, id);
    }
}

static void ua_put_nodeid_string(ua_buf_t *b, uint16_t ns, const char *id)
{
    ua_put_u8(b, 0x03);
    ua_put_u16(b, ns);
    ua_put_string(b, id);
}

static void ua_put_expanded_nodeid_null(ua_buf_t *b)
{
    ua_put_u8(b, 0x00);
    ua_put_u8(b, 0x00);
}

static void ua_put_extensionobject_null(ua_buf_t *b)
{
    ua_put_expanded_nodeid_null(b);
    ua_put_u8(b, 0x00); /* no body */
}

static void ua_put_diagnosticinfo_null(ua_buf_t *b)
{
    ua_put_u8(b, 0x00);
}

static void ua_put_response_header(ua_buf_t *b, uint32_t request_handle, uint32_t status)
{
    ua_put_u64(b, 0);                 /* timestamp; board has no RTC by default */
    ua_put_u32(b, request_handle);    /* requestHandle */
    ua_put_u32(b, status);            /* serviceResult */
    ua_put_diagnosticinfo_null(b);    /* serviceDiagnostics */
    ua_put_i32(b, -1);                /* stringTable */
    ua_put_extensionobject_null(b);   /* additionalHeader */
}

static void ua_put_localized_text(ua_buf_t *b, const char *locale, const char *text)
{
    uint8_t mask = 0;
    if (locale != NULL) mask |= 0x01;
    if (text != NULL) mask |= 0x02;

    ua_put_u8(b, mask);
    if (locale != NULL) ua_put_string(b, locale);
    if (text != NULL) ua_put_string(b, text);
}

static void ua_put_qualified_name(ua_buf_t *b, uint16_t ns, const char *name)
{
    ua_put_u16(b, ns);
    ua_put_string(b, name);
}

static void ua_put_application_description(ua_buf_t *b)
{
    ua_put_string(b, UA_APP_URI);
    ua_put_string(b, UA_PRODUCT_URI);
    ua_put_localized_text(b, "en-US", UA_APP_NAME);
    ua_put_i32(b, 0); /* ApplicationType_Server */
    ua_put_string(b, NULL);
    ua_put_string(b, NULL);
    ua_put_i32(b, 1); /* discoveryUrls */
    ua_put_string(b, g_endpoint_url);
}

static void ua_put_endpoint_description(ua_buf_t *b)
{
    ua_put_string(b, g_endpoint_url);
    ua_put_application_description(b);
    ua_put_bytestring(b, NULL, -1); /* serverCertificate */
    ua_put_i32(b, 1);               /* MessageSecurityMode_None */
    ua_put_string(b, UA_SECURITY_POLICY_NONE);

    ua_put_i32(b, 1);               /* UserTokenPolicy[] */
    ua_put_string(b, "anonymous"); /* policyId */
    ua_put_i32(b, 0);               /* UserTokenType_Anonymous */
    ua_put_string(b, NULL);         /* issuedTokenType */
    ua_put_string(b, NULL);         /* issuerEndpointUrl */
    ua_put_string(b, NULL);         /* securityPolicyUri */

    ua_put_string(b, UA_TRANSPORT_PROFILE);
    ua_put_u8(b, 0);                /* securityLevel */
}

static void ua_put_signature_data_null(ua_buf_t *b)
{
    ua_put_string(b, NULL);
    ua_put_bytestring(b, NULL, -1);
}

static bool ua_get_u8(const uint8_t *data, uint32_t len, uint32_t *off, uint8_t *v)
{
    if (*off + 1 > len) return false;
    *v = data[*off];
    *off += 1;
    return true;
}

static bool ua_get_u16(const uint8_t *data, uint32_t len, uint32_t *off, uint16_t *v)
{
    if (*off + 2 > len) return false;
    *v = (uint16_t)data[*off] | ((uint16_t)data[*off + 1] << 8);
    *off += 2;
    return true;
}

static bool ua_get_u32(const uint8_t *data, uint32_t len, uint32_t *off, uint32_t *v)
{
    if (*off + 4 > len) return false;
    *v = (uint32_t)data[*off] |
         ((uint32_t)data[*off + 1] << 8) |
         ((uint32_t)data[*off + 2] << 16) |
         ((uint32_t)data[*off + 3] << 24);
    *off += 4;
    return true;
}

static bool ua_skip_bytes(const uint8_t *data, uint32_t len, uint32_t *off, uint32_t n)
{
    (void)data;
    if (*off + n > len) return false;
    *off += n;
    return true;
}

static bool ua_read_string_copy(const uint8_t *data, uint32_t len, uint32_t *off, char *out, uint32_t out_size)
{
    uint32_t raw_len;
    if (!ua_get_u32(data, len, off, &raw_len)) return false;

    if ((int32_t)raw_len < 0)
    {
        if (out != NULL && out_size > 0) out[0] = '\0';
        return true;
    }

    if (*off + raw_len > len) return false;

    if (out != NULL && out_size > 0)
    {
        uint32_t copy_len = raw_len;
        if (copy_len >= out_size) copy_len = out_size - 1;
        memcpy(out, &data[*off], copy_len);
        out[copy_len] = '\0';
    }

    *off += raw_len;
    return true;
}

static bool ua_skip_string(const uint8_t *data, uint32_t len, uint32_t *off)
{
    return ua_read_string_copy(data, len, off, NULL, 0);
}

static bool ua_skip_bytestring(const uint8_t *data, uint32_t len, uint32_t *off)
{
    uint32_t raw_len;
    if (!ua_get_u32(data, len, off, &raw_len)) return false;
    if ((int32_t)raw_len < 0) return true;
    return ua_skip_bytes(data, len, off, raw_len);
}

static bool ua_parse_nodeid(const uint8_t *data, uint32_t len, uint32_t *off, ua_nodeid_t *nodeid)
{
    uint8_t enc;
    if (!ua_get_u8(data, len, off, &enc)) return false;

    memset(nodeid, 0, sizeof(*nodeid));
    nodeid->encoding = enc & 0x3f;

    switch (nodeid->encoding)
    {
    case 0x00: /* TwoByte */
    {
        uint8_t id;
        if (!ua_get_u8(data, len, off, &id)) return false;
        nodeid->ns = 0;
        nodeid->numeric = id;
        break;
    }
    case 0x01: /* FourByte */
    {
        uint8_t ns;
        uint16_t id;
        if (!ua_get_u8(data, len, off, &ns)) return false;
        if (!ua_get_u16(data, len, off, &id)) return false;
        nodeid->ns = ns;
        nodeid->numeric = id;
        break;
    }
    case 0x02: /* Numeric */
    {
        uint16_t ns;
        uint32_t id;
        if (!ua_get_u16(data, len, off, &ns)) return false;
        if (!ua_get_u32(data, len, off, &id)) return false;
        nodeid->ns = ns;
        nodeid->numeric = id;
        break;
    }
    case 0x03: /* String */
    {
        uint16_t ns;
        if (!ua_get_u16(data, len, off, &ns)) return false;
        nodeid->ns = ns;
        if (!ua_read_string_copy(data, len, off, nodeid->string_id, sizeof(nodeid->string_id))) return false;
        break;
    }
    default:
        return false;
    }

    return true;
}

static bool ua_skip_expanded_nodeid(const uint8_t *data, uint32_t len, uint32_t *off)
{
    uint32_t start = *off;
    ua_nodeid_t tmp;
    if (!ua_parse_nodeid(data, len, off, &tmp)) return false;

    uint8_t enc = data[start];
    if ((enc & 0x80) != 0)
    {
        if (!ua_skip_string(data, len, off)) return false;
    }
    if ((enc & 0x40) != 0)
    {
        uint32_t server_index;
        if (!ua_get_u32(data, len, off, &server_index)) return false;
    }
    return true;
}

static bool ua_skip_extensionobject(const uint8_t *data, uint32_t len, uint32_t *off)
{
    uint8_t encoding;
    if (!ua_skip_expanded_nodeid(data, len, off)) return false;
    if (!ua_get_u8(data, len, off, &encoding)) return false;

    if ((encoding & 0x03) != 0)
    {
        return ua_skip_bytestring(data, len, off);
    }

    return true;
}

static bool ua_parse_request_header(const uint8_t *data, uint32_t len, uint32_t start,
                                    uint32_t *after_header, uint32_t *request_handle)
{
    uint32_t off = start;
    ua_nodeid_t auth_token;
    uint32_t tmp;

    if (!ua_parse_nodeid(data, len, &off, &auth_token)) return false;
    if (!ua_skip_bytes(data, len, &off, 8)) return false;  /* timestamp */
    if (!ua_get_u32(data, len, &off, request_handle)) return false;
    if (!ua_get_u32(data, len, &off, &tmp)) return false;  /* returnDiagnostics */
    if (!ua_skip_string(data, len, &off)) return false;    /* auditEntryId */
    if (!ua_get_u32(data, len, &off, &tmp)) return false;  /* timeoutHint */
    if (!ua_skip_extensionobject(data, len, &off)) return false;

    if (after_header != NULL) *after_header = off;
    return true;
}

static bool ua_parse_read_request_count(const uint8_t *data, uint32_t len, uint32_t request_header_start,
                                        uint32_t *nodes_offset, uint32_t *nodes_count)
{
    uint32_t off;
    uint32_t handle;
    uint32_t timestamps_to_return;
    uint32_t count;

    if (!ua_parse_request_header(data, len, request_header_start, &off, &handle)) return false;
    if (!ua_skip_bytes(data, len, &off, 8)) return false;  /* maxAge Double */
    if (!ua_get_u32(data, len, &off, &timestamps_to_return)) return false;
    if (!ua_get_u32(data, len, &off, &count)) return false;

    *nodes_offset = off;
    *nodes_count = count;
    return true;
}

static bool ua_parse_read_value_id(const uint8_t *data, uint32_t len, uint32_t *off,
                                   ua_nodeid_t *nodeid, uint32_t *attribute_id)
{
    if (!ua_parse_nodeid(data, len, off, nodeid)) return false;
    if (!ua_get_u32(data, len, off, attribute_id)) return false;
    if (!ua_skip_string(data, len, off)) return false;    /* indexRange */
    if (!ua_skip_bytes(data, len, off, 2)) return false;  /* QualifiedName.NamespaceIndex */
    if (!ua_skip_string(data, len, off)) return false;    /* QualifiedName.Name */
    return true;
}

static bool ua_node_is_test_value(const ua_nodeid_t *nodeid, uint32_t attribute_id)
{
    if (attribute_id != UA_ATTRIBUTEID_VALUE) return false;

    if (nodeid->encoding == 0x03 && nodeid->ns == UA_NODE_TEST_NS &&
        strcmp(nodeid->string_id, UA_NODE_TEST_STRING) == 0)
    {
        return true;
    }

    /* Optional numeric alias for simple clients: ns=2;i=1001 */
    if ((nodeid->encoding == 0x01 || nodeid->encoding == 0x02) &&
        nodeid->ns == UA_NODE_TEST_NS && nodeid->numeric == 1001)
    {
        return true;
    }

    return false;
}

/* -------------------------------------------------------------------------------------------------
 * Socket helpers
 * ------------------------------------------------------------------------------------------------- */
static int32_t tcp_send_all(uint8_t sn, const uint8_t *data, uint32_t len)
{
    uint32_t sent = 0;
    uint32_t start = g_msec_cnt;

    while (sent < len)
    {
        if (getSn_SR(sn) != SOCK_ESTABLISHED)
        {
            return -1;
        }

        uint16_t chunk = (uint16_t)(len - sent);
        if (chunk > 2048) chunk = 2048;

        int32_t ret = send(sn, (uint8_t *)&data[sent], chunk);
        if (ret > 0)
        {
            sent += (uint32_t)ret;
            start = g_msec_cnt;
        }
        else
        {
            if ((g_msec_cnt - start) > UA_SEND_TIMEOUT_MS) return -2;
            vTaskDelay(1);
        }
    }

    return (int32_t)sent;
}

static int32_t tcp_recv_exact(uint8_t sn, uint8_t *data, uint32_t len)
{
    uint32_t got = 0;
    uint32_t start = g_msec_cnt;

    while (got < len)
    {
        uint8_t state = getSn_SR(sn);
        if (state == SOCK_CLOSE_WAIT || state == SOCK_CLOSED)
        {
            return -1;
        }

        uint16_t available = getSn_RX_RSR(sn);
        if (available > 0)
        {
            uint16_t chunk = (uint16_t)(len - got);
            if (chunk > available) chunk = available;

            int32_t ret = recv(sn, &data[got], chunk);
            if (ret > 0)
            {
                got += (uint32_t)ret;
                start = g_msec_cnt;
            }
        }
        else
        {
            if ((g_msec_cnt - start) > UA_RECV_TIMEOUT_MS) return -2;
            vTaskDelay(1);
        }
    }

    return (int32_t)got;
}

static bool opcua_recv_message(uint8_t sn, uint8_t *buf, uint32_t cap, uint32_t *message_len)
{
    if (tcp_recv_exact(sn, buf, 8) != 8) return false;

    uint32_t size = (uint32_t)buf[4] |
                    ((uint32_t)buf[5] << 8) |
                    ((uint32_t)buf[6] << 16) |
                    ((uint32_t)buf[7] << 24);

    if (size < 8 || size > cap)
    {
        return false;
    }

    if (size > 8)
    {
        if (tcp_recv_exact(sn, &buf[8], size - 8) != (int32_t)(size - 8)) return false;
    }

    *message_len = size;
    return true;
}

/* -------------------------------------------------------------------------------------------------
 * OPC UA response builders
 * ------------------------------------------------------------------------------------------------- */
static void ua_begin_message(ua_buf_t *b, const char *type)
{
    ua_put_u8(b, (uint8_t)type[0]);
    ua_put_u8(b, (uint8_t)type[1]);
    ua_put_u8(b, (uint8_t)type[2]);
    ua_put_u8(b, 'F');
    ua_put_u32(b, 0); /* patched at end */
}

static bool ua_finish_and_send(uint8_t sn, ua_buf_t *b)
{
    if (b->overflow) return false;
    ua_patch_u32(b, 4, b->len);
    if (b->overflow) return false;
    return tcp_send_all(sn, b->data, b->len) == (int32_t)b->len;
}

static bool opcua_send_ack(uint8_t sn)
{
    ua_buf_t b;
    ua_buf_init(&b, g_tx_buf, sizeof(g_tx_buf));

    ua_begin_message(&b, "ACK");
    ua_put_u32(&b, 0);       /* ProtocolVersion */
    ua_put_u32(&b, 8192);    /* ReceiveBufferSize */
    ua_put_u32(&b, 8192);    /* SendBufferSize */
    ua_put_u32(&b, 8192);    /* MaxMessageSize */
    ua_put_u32(&b, 1);       /* MaxChunkCount */

    printf(" OPC UA: ACK\n");
    return ua_finish_and_send(sn, &b);
}

static void ua_put_asymmetric_security_header_none(ua_buf_t *b)
{
    ua_put_string(b, UA_SECURITY_POLICY_NONE);
    ua_put_bytestring(b, NULL, -1); /* SenderCertificate */
    ua_put_bytestring(b, NULL, -1); /* ReceiverCertificateThumbprint */
}

static void ua_put_symmetric_security_header(ua_buf_t *b)
{
    ua_put_u32(b, UA_CHANNEL_ID);
    ua_put_u32(b, UA_TOKEN_ID);
}

static void ua_put_sequence_header(ua_buf_t *b, uint32_t request_id)
{
    ua_put_u32(b, g_secure_sequence_number++);
    ua_put_u32(b, request_id);
}

static bool opcua_send_opn_response(uint8_t sn, uint32_t request_id, uint32_t request_handle)
{
    ua_buf_t b;
    ua_buf_init(&b, g_tx_buf, sizeof(g_tx_buf));

    ua_begin_message(&b, "OPN");
    ua_put_u32(&b, UA_CHANNEL_ID);
    ua_put_asymmetric_security_header_none(&b);
    ua_put_sequence_header(&b, request_id);

    ua_put_nodeid_numeric(&b, 0, UA_NS0ID_OPENSECURECHANNEL_RESPONSE);
    ua_put_response_header(&b, request_handle, UA_STATUS_GOOD);
    ua_put_u32(&b, 0);             /* serverProtocolVersion */
    ua_put_u32(&b, UA_CHANNEL_ID); /* securityToken.channelId */
    ua_put_u32(&b, UA_TOKEN_ID);   /* securityToken.tokenId */
    ua_put_u64(&b, 0);             /* securityToken.createdAt */
    ua_put_u32(&b, 600000);        /* securityToken.revisedLifetime, ms */
    ua_put_bytestring(&b, NULL, -1); /* serverNonce */

    printf(" OPC UA: OpenSecureChannelResponse\n");
    return ua_finish_and_send(sn, &b);
}

static bool opcua_send_msg_response_begin(ua_buf_t *b, uint32_t request_id, uint32_t response_type_id)
{
    ua_buf_init(b, g_tx_buf, sizeof(g_tx_buf));
    ua_begin_message(b, "MSG");
    ua_put_symmetric_security_header(b);
    ua_put_sequence_header(b, request_id);
    ua_put_nodeid_numeric(b, 0, response_type_id);
    return !b->overflow;
}

static bool opcua_send_service_fault(uint8_t sn, uint32_t request_id, uint32_t request_handle, uint32_t status)
{
    ua_buf_t b;
    opcua_send_msg_response_begin(&b, request_id, UA_NS0ID_SERVICEFAULT_RESPONSE);
    ua_put_response_header(&b, request_handle, status);

    printf(" OPC UA: ServiceFault 0x%08lx\n", (unsigned long)status);
    return ua_finish_and_send(sn, &b);
}

static bool opcua_send_findservers_response(uint8_t sn, uint32_t request_id, uint32_t request_handle)
{
    ua_buf_t b;
    opcua_send_msg_response_begin(&b, request_id, UA_NS0ID_FINDSERVERS_RESPONSE);
    ua_put_response_header(&b, request_handle, UA_STATUS_GOOD);
    ua_put_i32(&b, 1); /* servers */
    ua_put_application_description(&b);

    printf(" OPC UA: FindServersResponse\n");
    return ua_finish_and_send(sn, &b);
}

static bool opcua_send_getendpoints_response(uint8_t sn, uint32_t request_id, uint32_t request_handle)
{
    ua_buf_t b;
    opcua_send_msg_response_begin(&b, request_id, UA_NS0ID_GETENDPOINTS_RESPONSE);
    ua_put_response_header(&b, request_handle, UA_STATUS_GOOD);
    ua_put_i32(&b, 1); /* endpoints */
    ua_put_endpoint_description(&b);

    printf(" OPC UA: GetEndpointsResponse\n");
    return ua_finish_and_send(sn, &b);
}

static bool opcua_send_createsession_response(uint8_t sn, uint32_t request_id, uint32_t request_handle)
{
    ua_buf_t b;
    opcua_send_msg_response_begin(&b, request_id, UA_NS0ID_CREATESESSION_RESPONSE);
    ua_put_response_header(&b, request_handle, UA_STATUS_GOOD);

    ua_put_nodeid_string(&b, 1, "session1"); /* sessionId */
    ua_put_nodeid_string(&b, 1, "auth1");    /* authenticationToken */
    ua_put_double(&b, 600000.0);              /* revisedSessionTimeout */
    ua_put_bytestring(&b, NULL, -1);          /* serverNonce */
    ua_put_bytestring(&b, NULL, -1);          /* serverCertificate */

    ua_put_i32(&b, 1);                        /* serverEndpoints */
    ua_put_endpoint_description(&b);

    ua_put_i32(&b, -1);                       /* serverSoftwareCertificates */
    ua_put_signature_data_null(&b);           /* serverSignature */
    ua_put_u32(&b, UA_RX_BUF_SIZE);           /* maxRequestMessageSize */

    printf(" OPC UA: CreateSessionResponse\n");
    return ua_finish_and_send(sn, &b);
}

static bool opcua_send_activatesession_response(uint8_t sn, uint32_t request_id, uint32_t request_handle)
{
    ua_buf_t b;
    opcua_send_msg_response_begin(&b, request_id, UA_NS0ID_ACTIVATESESSION_RESPONSE);
    ua_put_response_header(&b, request_handle, UA_STATUS_GOOD);
    ua_put_bytestring(&b, NULL, -1); /* serverNonce */
    ua_put_i32(&b, -1);              /* results */
    ua_put_i32(&b, -1);              /* diagnosticInfos */

    printf(" OPC UA: ActivateSessionResponse\n");
    return ua_finish_and_send(sn, &b);
}

static void ua_put_datavalue_int32(ua_buf_t *b, int32_t value)
{
    ua_put_u8(b, 0x03);               /* has Value + StatusCode */
    ua_put_u8(b, 0x06);               /* Variant: Int32 */
    ua_put_i32(b, value);
    ua_put_u32(b, UA_STATUS_GOOD);
}

static void ua_put_datavalue_status(ua_buf_t *b, uint32_t status)
{
    ua_put_u8(b, 0x02);               /* has StatusCode */
    ua_put_u32(b, status);
}

static bool opcua_send_read_response(uint8_t sn, uint32_t request_id, uint32_t request_handle,
                                     const uint8_t *service_body, uint32_t service_body_len,
                                     uint32_t request_header_start)
{
    uint32_t nodes_offset = 0;
    uint32_t nodes_count = 1;
    bool parsed = ua_parse_read_request_count(service_body, service_body_len,
                                              request_header_start, &nodes_offset, &nodes_count);

    if (!parsed || nodes_count == 0 || nodes_count > 32)
    {
        nodes_count = 1;
        nodes_offset = service_body_len;
    }

    ua_buf_t b;
    opcua_send_msg_response_begin(&b, request_id, UA_NS0ID_READ_RESPONSE);
    ua_put_response_header(&b, request_handle, UA_STATUS_GOOD);
    ua_put_i32(&b, (int32_t)nodes_count); /* results */

    uint32_t off = nodes_offset;
    for (uint32_t i = 0; i < nodes_count; i++)
    {
        ua_nodeid_t nodeid;
        uint32_t attribute_id = 0;
        bool ok = parsed && ua_parse_read_value_id(service_body, service_body_len, &off, &nodeid, &attribute_id);

        if (ok && ua_node_is_test_value(&nodeid, attribute_id))
        {
            ua_put_datavalue_int32(&b, UA_TEST_VALUE);
        }
        else if (ok && attribute_id != UA_ATTRIBUTEID_VALUE)
        {
            ua_put_datavalue_status(&b, UA_STATUS_BAD_NOTREADABLE);
        }
        else
        {
            ua_put_datavalue_status(&b, UA_STATUS_BAD_NODEIDUNKNOWN);
        }
    }

    ua_put_i32(&b, -1); /* diagnosticInfos */

    printf(" OPC UA: ReadResponse, nodes=%lu\n", (unsigned long)nodes_count);
    return ua_finish_and_send(sn, &b);
}

static bool opcua_send_closesession_response(uint8_t sn, uint32_t request_id, uint32_t request_handle)
{
    ua_buf_t b;
    opcua_send_msg_response_begin(&b, request_id, UA_NS0ID_CLOSESESSION_RESPONSE);
    ua_put_response_header(&b, request_handle, UA_STATUS_GOOD);

    printf(" OPC UA: CloseSessionResponse\n");
    return ua_finish_and_send(sn, &b);
}

/* -------------------------------------------------------------------------------------------------
 * OPC UA request handling
 * ------------------------------------------------------------------------------------------------- */
static bool opcua_parse_hello_endpoint(const uint8_t *msg, uint32_t len)
{
    if (len < 28) return false;

    uint32_t off = 8 + 20; /* header + version/buffers/max size/chunks */
    char endpoint[sizeof(g_endpoint_url)];
    if (!ua_read_string_copy(msg, len, &off, endpoint, sizeof(endpoint))) return false;

    if (endpoint[0] != '\0')
    {
        strncpy(g_endpoint_url, endpoint, sizeof(g_endpoint_url) - 1);
        g_endpoint_url[sizeof(g_endpoint_url) - 1] = '\0';
    }

    return true;
}

static bool opcua_handle_opn(uint8_t sn, const uint8_t *msg, uint32_t len)
{
    uint32_t off = 12; /* UACP header + secureChannelId */
    uint32_t request_id = 1;
    uint32_t request_handle = 1;

    if (!ua_skip_string(msg, len, &off)) return false;      /* SecurityPolicyUri */
    if (!ua_skip_bytestring(msg, len, &off)) return false;  /* SenderCertificate */
    if (!ua_skip_bytestring(msg, len, &off)) return false;  /* ReceiverCertificateThumbprint */

    uint32_t sequence_number;
    if (!ua_get_u32(msg, len, &off, &sequence_number)) return false;
    if (!ua_get_u32(msg, len, &off, &request_id)) return false;

    ua_nodeid_t service_id;
    if (!ua_parse_nodeid(msg, len, &off, &service_id)) return false;

    if (service_id.ns != 0 || service_id.numeric != UA_NS0ID_OPENSECURECHANNEL_REQUEST)
    {
        return opcua_send_service_fault(sn, request_id, request_handle, UA_STATUS_BAD_SERVICEUNSUPPORTED);
    }

    uint32_t after_header;
    if (!ua_parse_request_header(msg, len, off, &after_header, &request_handle))
    {
        return opcua_send_service_fault(sn, request_id, request_handle, UA_STATUS_BAD_DECODINGERROR);
    }

    return opcua_send_opn_response(sn, request_id, request_handle);
}

static bool opcua_handle_msg(uint8_t sn, const uint8_t *msg, uint32_t len, bool *close_after_response)
{
    uint32_t off = 16; /* UACP header + secureChannelId + tokenId */
    uint32_t sequence_number;
    uint32_t request_id = 1;
    uint32_t request_handle = 1;
    uint32_t request_header_start;

    (void)close_after_response;

    if (!ua_get_u32(msg, len, &off, &sequence_number)) return false;
    if (!ua_get_u32(msg, len, &off, &request_id)) return false;

    ua_nodeid_t service_id;
    if (!ua_parse_nodeid(msg, len, &off, &service_id)) return false;

    request_header_start = off;
    uint32_t after_header;
    if (!ua_parse_request_header(msg, len, request_header_start, &after_header, &request_handle))
    {
        return opcua_send_service_fault(sn, request_id, request_handle, UA_STATUS_BAD_DECODINGERROR);
    }

    if (service_id.ns != 0)
    {
        return opcua_send_service_fault(sn, request_id, request_handle, UA_STATUS_BAD_SERVICEUNSUPPORTED);
    }

    switch (service_id.numeric)
    {
    case UA_NS0ID_FINDSERVERS_REQUEST:
        return opcua_send_findservers_response(sn, request_id, request_handle);

    case UA_NS0ID_GETENDPOINTS_REQUEST:
        return opcua_send_getendpoints_response(sn, request_id, request_handle);

    case UA_NS0ID_CREATESESSION_REQUEST:
        return opcua_send_createsession_response(sn, request_id, request_handle);

    case UA_NS0ID_ACTIVATESESSION_REQUEST:
        return opcua_send_activatesession_response(sn, request_id, request_handle);

    case UA_NS0ID_READ_REQUEST:
        return opcua_send_read_response(sn, request_id, request_handle, msg, len, request_header_start);

    case UA_NS0ID_CLOSESESSION_REQUEST:
        *close_after_response = true;
        return opcua_send_closesession_response(sn, request_id, request_handle);

    default:
        return opcua_send_service_fault(sn, request_id, request_handle, UA_STATUS_BAD_SERVICEUNSUPPORTED);
    }
}

static void opcua_client_loop(uint8_t sn)
{
    uint32_t msg_len = 0;
    bool close_after_response = false;
    g_secure_sequence_number = 1;

    while (1)
    {
        if (!opcua_recv_message(sn, g_rx_buf, sizeof(g_rx_buf), &msg_len))
        {
            printf(" OPC UA: receive failed or connection closed\n");
            break;
        }

        if (msg_len < 8) break;

        if (memcmp(g_rx_buf, "HEL", 3) == 0)
        {
            opcua_parse_hello_endpoint(g_rx_buf, msg_len);
            if (!opcua_send_ack(sn)) break;
        }
        else if (memcmp(g_rx_buf, "OPN", 3) == 0)
        {
            if (!opcua_handle_opn(sn, g_rx_buf, msg_len)) break;
        }
        else if (memcmp(g_rx_buf, "MSG", 3) == 0)
        {
            close_after_response = false;
            if (!opcua_handle_msg(sn, g_rx_buf, msg_len, &close_after_response)) break;
            if (close_after_response) break;
        }
        else if (memcmp(g_rx_buf, "CLO", 3) == 0)
        {
            printf(" OPC UA: CloseSecureChannel\n");
            break;
        }
        else
        {
            printf(" OPC UA: unsupported message type %.3s\n", g_rx_buf);
            break;
        }
    }
}

/* -------------------------------------------------------------------------------------------------
 * FreeRTOS task and board setup
 * ------------------------------------------------------------------------------------------------- */
static void set_clock_khz(void)
{
    set_sys_clock_khz(PLL_SYS_KHZ, true);

    clock_configure(
        clk_peri,
        0,
        CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
        PLL_SYS_KHZ * 1000,
        PLL_SYS_KHZ * 1000);
}

static void repeating_timer_callback(void)
{
    g_msec_cnt++;
}

static void opcua_task(void *argument)
{
    (void)argument;

    network_initialize(g_net_info);
    print_network_information(g_net_info);

    snprintf(g_endpoint_url, sizeof(g_endpoint_url), "opc.tcp://%u.%u.%u.%u:%u",
             g_net_info.ip[0], g_net_info.ip[1], g_net_info.ip[2], g_net_info.ip[3], PORT_OPCUA);

    printf(" OPC UA endpoint: %s\n", g_endpoint_url);
    printf(" OPC UA test node: ns=2;s=TestValue, Int32=%d\n", UA_TEST_VALUE);

    while (1)
    {
        close(SOCKET_OPCUA);

        int8_t ret = socket(SOCKET_OPCUA, Sn_MR_TCP, PORT_OPCUA, 0x00);
        if (ret != SOCKET_OPCUA)
        {
            printf(" OPC UA: socket() failed: %d\n", ret);
            vTaskDelay(1000);
            continue;
        }

        ret = listen(SOCKET_OPCUA);
        if (ret != SOCK_OK)
        {
            printf(" OPC UA: listen() failed: %d\n", ret);
            close(SOCKET_OPCUA);
            vTaskDelay(1000);
            continue;
        }

        printf(" OPC UA: listening on TCP %u\n", PORT_OPCUA);

        while (getSn_SR(SOCKET_OPCUA) != SOCK_ESTABLISHED)
        {
            uint8_t state = getSn_SR(SOCKET_OPCUA);
            if (state == SOCK_CLOSED || state == SOCK_CLOSE_WAIT)
            {
                break;
            }
            vTaskDelay(10);
        }

        if (getSn_SR(SOCKET_OPCUA) == SOCK_ESTABLISHED)
        {
            printf(" OPC UA: client connected\n");
            opcua_client_loop(SOCKET_OPCUA);
        }

        disconnect(SOCKET_OPCUA);
        close(SOCKET_OPCUA);
        printf(" OPC UA: client disconnected\n");
        vTaskDelay(100);
    }
}

int main(void)
{
    set_clock_khz();

    stdio_init_all();

    wizchip_spi_initialize();
    wizchip_cris_initialize();

    wizchip_reset();
    wizchip_initialize();
    wizchip_check();

    wizchip_1ms_timer_initialize(repeating_timer_callback);

    xTaskCreate(opcua_task, "OPCUA_Task", OPCUA_TASK_STACK_SIZE, NULL, OPCUA_TASK_PRIORITY, NULL);

    vTaskStartScheduler();

    while (1)
    {
        ;
    }
}
