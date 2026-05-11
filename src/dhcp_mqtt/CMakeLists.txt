set(TARGET_NAME wizchip_dhcp_mqtt)

if (NOT FREERTOS_KERNEL_PATH AND NOT DEFINED ENV{FREERTOS_KERNEL_PATH})
    message("Skipping FreeRTOS examples as FREERTOS_KERNEL_PATH not defined")
    return()
endif()

add_executable(${TARGET_NAME})
target_sources(${TARGET_NAME}
PRIVATE
    wizchip_dhcp_mqtt.c
)
target_include_directories(${TARGET_NAME}
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)
target_link_libraries(${TARGET_NAME}
PRIVATE
    pico_async_context_freertos
    FreeRTOS-Kernel-Heap4
    FreeRTOS-Config
    pico_stdlib

    WIZnet::port
    WIZnet::Ethernet
    WIZnet::DHCP
    WIZnet::DNS
    WIZnet::MQTT
    timer
)
#[[
Чтобы не редактировать заголовочный файл платы, можно задать
значения прямо здесь. target_compile_definitions равноценен параметру -D.
Этот способ указан в документации:
RP-009085-KB-1-raspberry-pi-pico-c-sdk.pdf, с.550
#]]
target_compile_definitions(${TARGET_NAME}
PRIVATE
    PICO_FLASH_SPI_CLKDIV=2
)


pico_enable_stdio_usb(${TARGET_NAME} 1)
pico_enable_stdio_uart(${TARGET_NAME} 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(${TARGET_NAME})
pico_set_program_url(${TARGET_NAME} "${TARGET_NAME}")
