add_library(open62541 STATIC)
target_sources(open62541 PRIVATE open62541.c
PUBLIC FILE_SET HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR}/include FILES 
${CMAKE_CURRENT_LIST_DIR}/include/open62541.h
${CMAKE_CURRENT_LIST_DIR}/include/open62541config.h
${CMAKE_CURRENT_LIST_DIR}/include/opc_add_temperature.h
${CMAKE_CURRENT_LIST_DIR}/include/opc_freertos_status.h
${CMAKE_CURRENT_LIST_DIR}/include/FreeRTOSConfig.h
)

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
    # FreeRTOS-Config
    FreeRTOS-FreeRTOS-Kernel-Heap4
    WIZnet::port
    WIZnet::Ethernet
    pico_stdlib
    # pico_lwip_contrib_freertos
)
