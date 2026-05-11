#ifndef W5X00_OPCUA_CONFIG_H
#define W5X00_OPCUA_CONFIG_H

#include <stdint.h>

/*
 * Network configuration for the RP2040 + WIZnet HAT.
 * Change BOARD_IP / GATEWAY if your PC is in another subnet.
 */
#define OPCUA_BOARD_MAC_0 0x00
#define OPCUA_BOARD_MAC_1 0x08
#define OPCUA_BOARD_MAC_2 0xDC
#define OPCUA_BOARD_MAC_3 0x12
#define OPCUA_BOARD_MAC_4 0x34
#define OPCUA_BOARD_MAC_5 0x56

#define OPCUA_BOARD_IP_0 192
#define OPCUA_BOARD_IP_1 168
#define OPCUA_BOARD_IP_2 1
#define OPCUA_BOARD_IP_3 2

#define OPCUA_NETMASK_0 255
#define OPCUA_NETMASK_1 255
#define OPCUA_NETMASK_2 255
#define OPCUA_NETMASK_3 0

#define OPCUA_GATEWAY_0 192
#define OPCUA_GATEWAY_1 168
#define OPCUA_GATEWAY_2 1
#define OPCUA_GATEWAY_3 1

#define OPCUA_DNS_0 8
#define OPCUA_DNS_1 8
#define OPCUA_DNS_2 8
#define OPCUA_DNS_3 8

#define OPCUA_TCP_PORT 4840u
#define OPCUA_W5X00_SOCKET 0u

/* open62541 needs noticeably more stack than the previous hand-written demo. */
#define OPCUA_TASK_STACK_SIZE (16u * 1024u)
#define OPCUA_TASK_PRIORITY   10u

/* W5500 buffers. For UaExpert / python-opcua browsing, 8 KiB is a practical minimum. */
#define OPCUA_RECV_BUFFER_SIZE (8u * 1024u)
#define OPCUA_SEND_BUFFER_SIZE (8u * 1024u)

/* Main UA_Server_run_iterate period. Smaller values improve responsiveness. */
#define OPCUA_SERVER_ITERATE_DELAY_MS 5u

/* Namespace used by the application model. */
#define OPCUA_APP_NAMESPACE_URI "urn:rp2040:wiznet:opcua"
#define OPCUA_APP_URI           "urn:rp2040:wiznet:open62541-server"
#define OPCUA_PRODUCT_URI       "urn:rp2040:wiznet:opcua-demo"
#define OPCUA_APP_NAME          "RP2040 WIZnet open62541 OPC UA Server"

/* NodeIds exported by the server. */
#define OPCUA_NODE_TEST_VALUE_STRING "TestValue"
#define OPCUA_NODE_UPTIME_STRING     "UptimeMs"
#define OPCUA_NODE_LED_STATE_STRING  "LedState"
#define OPCUA_NODE_RESET_METHOD      "ResetTestValue"

#define OPCUA_TEST_VALUE_DEFAULT 42

#endif /* W5X00_OPCUA_CONFIG_H */
