add_executable(onboard_temperature onboard_temperature.c)


file(MAKE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/generated)
pico_generate_pio_header(onboard_temperature ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)

target_link_libraries(onboard_temperature pico_stdlib hardware_adc hardware_pio minidebug)
target_include_directories(onboard_temperature PRIVATE ${CMAKE_CURRENT_LIST_DIR})

pico_enable_stdio_uart(onboard_temperature 0)
pico_enable_stdio_usb(onboard_temperature 1)
pico_add_extra_outputs(onboard_temperature)
pico_set_program_url(onboard_temperature "adc/onboard_temperature")

