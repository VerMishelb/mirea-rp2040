add_library(open62541 STATIC)
target_sources(open62541 PRIVATE open62541.c clock_freertos.c PUBLIC FILE_SET HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES open62541.h)
target_include_directories(open62541 PUBLIC ${CMAKE_SOURCE_DIR}/lib/mbedtls)
target_link_libraries(open62541
PUBLIC
    lwipopts
    pico_lwip
    # pico_lwip_nosys
    pico_lwip_freertos
    pico_lwip_mbedtls
    pico_mbedtls
    # pico_mbedtls_headers
    FreeRTOS-Config
    FreeRTOS-Kernel-Heap4
    WIZnet::port
    WIZnet::Ethernet
    pico_stdlib
    # pico_lwip_contrib_freertos
)
