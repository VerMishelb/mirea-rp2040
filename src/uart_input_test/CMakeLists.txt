set(TGNAM uart_input_test)

add_executable(${TGNAM})
file(MAKE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/generated)
pico_generate_pio_header(${TGNAM} ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)
target_sources(${TGNAM} PRIVATE main.c)
target_include_directories(${TGNAM} PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(${TGNAM} pico_stdlib minidebug hardware_pio)

pico_enable_stdio_uart(${TGNAM} 0)
pico_enable_stdio_usb(${TGNAM} 1)
pico_add_extra_outputs(${TGNAM})
pico_set_program_url(${TGNAM} "${TGNAM}")
