set(WIZnet_DIR ${CMAKE_SOURCE_DIR}/lib/vendor/ioLibrary_Driver)
set(WIZnet_port_DIR ${CMAKE_SOURCE_DIR}/lib/ioLibrary_Driver_port)

if (NOT DEFINED WIZCHIP)
    set(WIZCHIP W5500)
endif()
if (NOT DEFINED WIZCHIP_SPI_SCLK_SPEED)
    set(WIZCHIP_SPI_SCLK_SPEED 43)
endif()
# add_compile_definitions(
#     _WIZCHIP_=${WIZCHIP}
#     _WIZCHIP_SPI_SCLK_SPEED=${WIZCHIP_SPI_SCLK_SPEED}
# )

# ioLibrary port (IOLIBRARY_FILES)
add_library(WIZnet_port STATIC)
add_library(WIZnet::port ALIAS WIZnet_port)
target_sources(WIZnet_port
    PRIVATE
        ${WIZnet_port_DIR}/wizchip_spi.c
        ${WIZnet_port_DIR}/wizchip_gpio_irq.c
)
if (${BOARD_NAME} STREQUAL W55RP20_EVB_PICO OR ${WIZCHIP} STREQUAL W6300)
    pico_generate_pio_header(WIZnet_port ${WIZnet_port_DIR}/wizchip_qspi_pio.pio)
    target_sources(WIZnet_port PUBLIC ${WIZnet_port_DIR}/wizchip_qspi_pio.c)
endif()
target_include_directories(WIZnet_port
    PUBLIC
        ${WIZnet_port_DIR}
)
target_link_libraries(WIZnet_port
    PUBLIC
        pico_stdlib
        hardware_pio
        hardware_spi
        hardware_dma
        hardware_clocks
        FreeRTOS-Config
        FreeRTOS-Kernel-Heap4
        WIZnet_COMMON
)


add_library(WIZnet_COMMON INTERFACE)
target_include_directories(WIZnet_COMMON
    INTERFACE
        ${WIZnet_DIR}/Ethernet
        ${WIZnet_DIR}/Ethernet/$<UPPER_CASE:${WIZCHIP}>
)
target_compile_definitions(WIZnet_COMMON INTERFACE
    _WIZCHIP_=${WIZCHIP}
    _WIZCHIP_SPI_SCLK_SPEED=${WIZCHIP_SPI_SCLK_SPEED}
    # # DEVICE_BOARD_NAME=W5500_EVB_PICO
)




add_library(WIZnet_ETHERNET STATIC)
add_library(WIZnet::Ethernet ALIAS WIZnet_ETHERNET)
target_sources(WIZnet_ETHERNET
    PUBLIC
        ${WIZnet_DIR}/Ethernet/socket.c
        ${WIZnet_DIR}/Ethernet/wizchip_conf.c
        ${WIZnet_DIR}/Ethernet/$<UPPER_CASE:${WIZCHIP}>/$<LOWER_CASE:${WIZCHIP}>.c
)
target_include_directories(WIZnet_ETHERNET
    PUBLIC
        ${WIZnet_DIR}/Ethernet
        ${WIZnet_DIR}/Ethernet/$<UPPER_CASE:${WIZCHIP}>
)
target_link_libraries(WIZnet_ETHERNET PUBLIC WIZnet_COMMON)



add_library(WIZnet_LOOPBACK STATIC)
add_library(WIZnet::Loopback ALIAS WIZnet_LOOPBACK)
target_sources(WIZnet_LOOPBACK
    PRIVATE
        ${WIZnet_DIR}/Application/loopback/loopback.c
)
target_include_directories(WIZnet_LOOPBACK
    PUBLIC
        ${WIZnet_DIR}/Application/loopback
)
target_link_libraries(WIZnet_LOOPBACK PUBLIC WIZnet_COMMON)



add_library(WIZnet_MULTICAST STATIC)
add_library(WIZnet::Multicast ALIAS WIZnet_MULTICAST)
target_sources(WIZnet_MULTICAST
    PRIVATE
        ${WIZnet_DIR}/Application/multicast/multicast.c
)
target_include_directories(WIZnet_MULTICAST
    PUBLIC
        ${WIZnet_DIR}/Application/multicast
)
target_link_libraries(WIZnet_MULTICAST PUBLIC WIZnet_COMMON)



add_library(WIZnet_AAC STATIC)
add_library(WIZnet::AAC ALIAS WIZnet_AAC)
target_sources(WIZnet_AAC
    PRIVATE
        ${WIZnet_DIR}/Internet/AAC/AddressAutoConfig.c
)
target_include_directories(WIZnet_AAC
    PUBLIC
        ${WIZnet_DIR}/Internet/AAC
)
target_link_libraries(WIZnet_AAC PUBLIC WIZnet_COMMON)



add_library(WIZnet_DHCP STATIC)
add_library(WIZnet::DHCP ALIAS WIZnet_DHCP)
target_sources(WIZnet_DHCP
    PRIVATE
        ${WIZnet_DIR}/Internet/DHCP/dhcp.c
)
target_include_directories(WIZnet_DHCP
    PUBLIC
        ${WIZnet_DIR}/Internet/DHCP
)
target_link_libraries(WIZnet_DHCP PUBLIC WIZnet_COMMON)



add_library(WIZnet_DHCP6 STATIC)
add_library(WIZnet::DHCP6 ALIAS WIZnet_DHCP6)
target_sources(WIZnet_DHCP6
    PRIVATE
        ${WIZnet_DIR}/Internet/DHCP6/dhcpv6.c
)
target_include_directories(WIZnet_DHCP6
    PUBLIC
        ${WIZnet_DIR}/Internet/DHCP6
)
target_link_libraries(WIZnet_DHCP6 PUBLIC WIZnet_COMMON)



add_library(WIZnet_DNS STATIC)
add_library(WIZnet::DNS ALIAS WIZnet_DNS)
target_sources(WIZnet_DNS
    PRIVATE
        ${WIZnet_DIR}/Internet/DNS/dns.c
)
target_include_directories(WIZnet_DNS
    PUBLIC
        ${WIZnet_DIR}/Internet/DNS
)
target_link_libraries(WIZnet_DNS PUBLIC WIZnet_COMMON)



add_library(WIZnet_HTTPSERVER STATIC)
add_library(WIZnet::HTTPServer ALIAS WIZnet_HTTPSERVER)
target_sources(WIZnet_HTTPSERVER
    PRIVATE
        ${WIZnet_DIR}/Internet/httpServer/httpParser.c
        ${WIZnet_DIR}/Internet/httpServer/httpServer.c
        ${WIZnet_DIR}/Internet/httpServer/httpUtil.c
)
target_include_directories(WIZnet_HTTPSERVER
    PUBLIC
        ${WIZnet_DIR}/Internet/httpServer
)
target_link_libraries(WIZnet_HTTPSERVER PUBLIC WIZnet_COMMON)



add_library(WIZnet_MQTT STATIC)
add_library(WIZnet::MQTT ALIAS WIZnet_MQTT)
target_sources(WIZnet_MQTT
    PRIVATE
        ${WIZnet_DIR}/Internet/MQTT/mqtt_interface.c
        ${WIZnet_DIR}/Internet/MQTT/MQTTClient.c
        ${WIZnet_DIR}/Internet/MQTT/MQTTPacket/src/MQTTConnectClient.c
        ${WIZnet_DIR}/Internet/MQTT/MQTTPacket/src/MQTTConnectServer.c
        ${WIZnet_DIR}/Internet/MQTT/MQTTPacket/src/MQTTDeserializePublish.c
        ${WIZnet_DIR}/Internet/MQTT/MQTTPacket/src/MQTTFormat.c
        ${WIZnet_DIR}/Internet/MQTT/MQTTPacket/src/MQTTPacket.c
        ${WIZnet_DIR}/Internet/MQTT/MQTTPacket/src/MQTTSerializePublish.c
        ${WIZnet_DIR}/Internet/MQTT/MQTTPacket/src/MQTTSubscribeClient.c
        ${WIZnet_DIR}/Internet/MQTT/MQTTPacket/src/MQTTSubscribeServer.c
        ${WIZnet_DIR}/Internet/MQTT/MQTTPacket/src/MQTTUnsubscribeClient.c
        ${WIZnet_DIR}/Internet/MQTT/MQTTPacket/src/MQTTUnsubscribeServer.c
)
target_include_directories(WIZnet_MQTT
    PUBLIC
        ${WIZnet_DIR}/Internet/MQTT
        ${WIZnet_DIR}/Internet/MQTTPacket/src
)
target_link_libraries(WIZnet_MQTT PUBLIC WIZnet_COMMON)



add_library(WIZnet_SNMP STATIC)
add_library(WIZnet::SNMP ALIAS WIZnet_SNMP)
target_sources(WIZnet_SNMP
    PRIVATE
        ${WIZnet_DIR}/Internet/SNMP/snmp.c
)
target_include_directories(WIZnet_SNMP
    PUBLIC
        ${WIZnet_DIR}/Internet/SNMP
)
target_link_libraries(WIZnet_SNMP PUBLIC WIZnet_COMMON)



add_library(WIZnet_SNTP STATIC)
add_library(WIZnet::SNTP ALIAS WIZnet_SNTP)
target_sources(WIZnet_SNTP
    PRIVATE
        ${WIZnet_DIR}/Internet/SNTP/sntp.c
)
target_include_directories(WIZnet_SNTP
    PUBLIC
        ${WIZnet_DIR}/Internet/SNTP
)
target_link_libraries(WIZnet_SNTP PUBLIC WIZnet_COMMON)


# Судя по всему сломан. В исходнике тоже был закомменчен.
# add_library(WIZnet_TFTP STATIC)
# add_library(WIZnet::TFTP ALIAS WIZnet_TFTP)
# target_sources(WIZnet_TFTP
#     PRIVATE
#         ${WIZnet_DIR}/Internet/TFTP/tftp.c
#         ${WIZnet_DIR}/Internet/TFTP/netutil.c
# )
# target_include_directories(WIZnet_TFTP
#     PUBLIC
#         ${WIZnet_DIR}/Internet/TFTP
# )
# target_link_libraries(WIZnet_TFTP PUBLIC WIZnet_COMMON)


message(STATUS 
"Imported IoLibrary & IoLibrary port.
    _WIZCHIP_=${WIZCHIP}
    _WIZHIP_SPI_SCLK_SPEED=${WIZCHIP_SPI_SCLK_SPEED}")
