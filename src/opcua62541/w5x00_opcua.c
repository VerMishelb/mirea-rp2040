/**
 * RP2040 + WIZnet OPC UA server based on open62541.
 *
 * This replaces the previous hand-written minimal OPC UA Binary prototype.
 * The protocol stack, sessions, services, browsing and monitored-items are
 * handled by open62541. The only hardware-specific part here is the WIZnet
 * network layer and the application data callbacks.
 */

#include <stdio.h>
#include <string.h>

#include <FreeRTOS.h>
#include <task.h>

#include "pico/stdlib.h"
#include "hardware/clocks.h"

#include "port_common.h"
#include "wizchip_conf.h"
#include "w5x00_spi.h"
#include "timer.h"

#include "open62541.h"
#include "w5x00_opcua_config.h"
#include "w5x00_open62541_net.h"
#include "opcua_model.h"

#define PLL_SYS_KHZ (133 * 1000)

static wiz_NetInfo g_net_info = {
    .mac = {OPCUA_BOARD_MAC_0, OPCUA_BOARD_MAC_1, OPCUA_BOARD_MAC_2,
            OPCUA_BOARD_MAC_3, OPCUA_BOARD_MAC_4, OPCUA_BOARD_MAC_5},
    .ip = {OPCUA_BOARD_IP_0, OPCUA_BOARD_IP_1, OPCUA_BOARD_IP_2, OPCUA_BOARD_IP_3},
    .sn = {OPCUA_NETMASK_0, OPCUA_NETMASK_1, OPCUA_NETMASK_2, OPCUA_NETMASK_3},
    .gw = {OPCUA_GATEWAY_0, OPCUA_GATEWAY_1, OPCUA_GATEWAY_2, OPCUA_GATEWAY_3},
    .dns = {OPCUA_DNS_0, OPCUA_DNS_1, OPCUA_DNS_2, OPCUA_DNS_3},
    .dhcp = NETINFO_STATIC
};

static volatile uint32_t g_msec_cnt = 0;

static void set_clock_khz(void);
static void opcua_task(void *argument);
static void repeating_timer_callback(void);
static void make_endpoint_url(char *out, size_t outSize);
static void configure_server(UA_Server *server, const char *endpointUrl);

int main(void) {
    set_clock_khz();
    stdio_init_all();

    wizchip_spi_initialize();
    wizchip_cris_initialize();
    wizchip_reset();
    wizchip_initialize();
    wizchip_check();
    wizchip_1ms_timer_initialize(repeating_timer_callback);

    opcua_model_init_runtime();

    xTaskCreate(opcua_task,
                "OPCUA_Task",
                OPCUA_TASK_STACK_SIZE,
                NULL,
                OPCUA_TASK_PRIORITY,
                NULL);

    vTaskStartScheduler();

    while(1) {
        tight_loop_contents();
    }
}

static void opcua_task(void *argument) {
    (void)argument;

    char endpointUrl[96];
    make_endpoint_url(endpointUrl, sizeof(endpointUrl));

    network_initialize(g_net_info);
    print_network_information(g_net_info);
    printf("OPC UA endpoint: %s\n", endpointUrl);

    UA_Server *server = UA_Server_new();
    if(!server) {
        printf("OPC UA: UA_Server_new failed\n");
        vTaskDelete(NULL);
        return;
    }

    configure_server(server, endpointUrl);

    UA_StatusCode rc = opcua_model_add_nodes(server);
    if(rc != UA_STATUSCODE_GOOD) {
        printf("OPC UA: opcua_model_add_nodes failed: 0x%08lx\n", (unsigned long)rc);
        UA_Server_delete(server);
        vTaskDelete(NULL);
        return;
    }

    UA_Boolean running = true;
    rc = UA_Server_run_startup(server);
    if(rc != UA_STATUSCODE_GOOD) {
        printf("OPC UA: UA_Server_run_startup failed: 0x%08lx\n", (unsigned long)rc);
        UA_Server_delete(server);
        vTaskDelete(NULL);
        return;
    }

    printf("OPC UA: open62541 server started\n");

    while(running) {
        UA_Server_run_iterate(server, false);
        opcua_model_periodic_update(server);
        vTaskDelay(pdMS_TO_TICKS(OPCUA_SERVER_ITERATE_DELAY_MS));
    }

    UA_Server_run_shutdown(server);
    UA_Server_delete(server);
    vTaskDelete(NULL);
}

static void configure_server(UA_Server *server, const char *endpointUrl) {
    UA_ServerConfig *config = UA_Server_getConfig(server);

    /*
     * UA_Server_new() creates a default config. For RP2040 + WIZnet we replace
     * the default network layer with our W5500 network layer. This uses the
     * official open62541 server network plugin API from the 1.3 release branch.
     */
    for(size_t i = 0; i < config->networkLayersSize; i++) {
        if(config->networkLayers[i].clear)
            config->networkLayers[i].clear(&config->networkLayers[i]);
    }

    config->networkLayers = (UA_ServerNetworkLayer *)UA_calloc(1, sizeof(UA_ServerNetworkLayer));
    config->networkLayersSize = 1;
    config->networkLayers[0] = UA_ServerNetworkLayerW5x00(OPCUA_W5X00_SOCKET,
                                                          OPCUA_TCP_PORT,
                                                          endpointUrl,
                                                          OPCUA_RECV_BUFFER_SIZE,
                                                          OPCUA_SEND_BUFFER_SIZE);

    UA_LocalizedText_clear(&config->applicationDescription.applicationName);
    config->applicationDescription.applicationName = UA_LOCALIZEDTEXT_ALLOC("en-US", OPCUA_APP_NAME);

    UA_String_clear(&config->applicationDescription.applicationUri);
    config->applicationDescription.applicationUri = UA_STRING_ALLOC(OPCUA_APP_URI);

    UA_String_clear(&config->applicationDescription.productUri);
    config->applicationDescription.productUri = UA_STRING_ALLOC(OPCUA_PRODUCT_URI);

    config->publishingIntervalLimits.min = 100.0;
    config->publishingIntervalLimits.max = 10000.0;
    config->samplingIntervalLimits.min = 100.0;
    config->samplingIntervalLimits.max = 10000.0;

    /* Keep SecurityPolicy#None / anonymous for the RP2040 demo. */
    printf("OPC UA: SecurityPolicy=None, anonymous access enabled\n");
}

static void make_endpoint_url(char *out, size_t outSize) {
    snprintf(out, outSize,
             "opc.tcp://%u.%u.%u.%u:%u",
             g_net_info.ip[0], g_net_info.ip[1], g_net_info.ip[2], g_net_info.ip[3],
             (unsigned)OPCUA_TCP_PORT);
}

static void set_clock_khz(void) {
    set_sys_clock_khz(PLL_SYS_KHZ, true);
    clock_configure(clk_peri,
                    0,
                    CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
                    PLL_SYS_KHZ * 1000,
                    PLL_SYS_KHZ * 1000);
}

static void repeating_timer_callback(void) {
    g_msec_cnt++;
    MilliTimer_Handler();
}
