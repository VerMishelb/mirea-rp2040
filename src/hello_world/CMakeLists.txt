set(TGNAM hello_world)

add_executable(${TGNAM})
target_sources(${TGNAM} PRIVATE main.c)
target_link_libraries(${TGNAM} pico_stdlib pico_stdlib hardware_adc hardware_pio)

pico_enable_stdio_usb(${TGNAM} 1)
pico_enable_stdio_uart(${TGNAM} 0)
pico_add_extra_outputs(${TGNAM})