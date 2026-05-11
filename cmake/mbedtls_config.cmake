set(mbedtls_config_DIR ${CMAKE_SOURCE_DIR}/lib/mbedtls)

add_library(mbedtls_config INTERFACE)
target_sources(mbedtls_config
    INTERFACE
    FILE_SET HEADERS
    BASE_DIRS ${mbedtls_config_DIR}
    FILES
        ${mbedtls_config_DIR}/ssl_config.h
)

target_link_libraries(mbedtls_config
    INTERFACE
        pico_stdlib
        hardware_pio
        hardware_spi
        hardware_dma
        hardware_clocks
)
target_compile_definitions(mbedtls_config
    INTERFACE
        MBEDTLS_CONFIG_FILE="${mbedtls_config_DIR}/ssl_config.h"
        SET_TRUSTED_CERT_IN_SAMPLES
)
message(STATUS "Imported mbedtls_config.")
