# https://github.com/carlk3/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico
set(no-OS-FatFS-SD-SDIO-SPI-RPi-Pico_DIR "${CMAKE_SOURCE_DIR}/lib/vendor/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico-3.6.2")

add_subdirectory(${no-OS-FatFS-SD-SDIO-SPI-RPi-Pico_DIR}/src build)
add_compile_definitions(
    USE_PRINTF
    USE_DEBUG_PRINTF
)
message(STATUS "Imported no-OS-FatFS-SD-SDIO-SPI-RPi-Pico.")
