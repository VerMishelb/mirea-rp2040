cmake_minimum_required(VERSION 4.1)

set(BOARD_NAME W5500_EVB_PICO)
# set(WIZCHIP W5500)
# set(WIZCHIP_SPI_SCLK_SPEED 43)



include(pico_sdk_import.cmake) # Must be before project()
set(FREERTOS_KERNEL_PATH ${CMAKE_SOURCE_DIR}/lib/vendor/FreeRTOS-Kernel) # Must be set before freertos import

project(rp2040-freertos-lwip-opcua C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# This does not help.
# list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
# message(STATUS "AAAAAAAAA ${CMAKE_CURRENT_SOURCE_DIR} \n ${CMAKE_MODULE_PATH} \n AAAAAAAAA")

pico_sdk_init()

include(cmake/FreeRTOS_Kernel_import.cmake) # FreeRTOS-Kernel-Heap[1-4]/-Static
include(cmake/FreeRTOS-Config.cmake) # FreeRTOS-Config
include(cmake/IoLibrary_Driver.cmake) # IoLibrary
include(cmake/mbedtls_config.cmake) # mbedtls_config
include(cmake/timer.cmake) # timer_port
include(cmake/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico.cmake)
add_subdirectory(lib/lwip-config)
# add_subdirectory(lib/open62541)


add_library(minidebug INTERFACE)
target_sources(minidebug INTERFACE FILE_SET HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR}/src FILES src/minidebug.h)

add_subdirectory(src/hello_world)
add_subdirectory(src/freertos_example)
add_subdirectory(src/dhcp_mqtt)
add_subdirectory(src/ws2812)
add_subdirectory(src/ws2812_simple)
add_subdirectory(src/onboard_temperature)
# add_subdirectory(src/opcua)
add_subdirectory(src/uart_input_test)
# add_subdirectory(src/opcua62541)
add_subdirectory(src/sd)
add_subdirectory(src/sd_command_line)