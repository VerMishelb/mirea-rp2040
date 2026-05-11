set(TGNAM pio_ws2812_simple)
add_executable(${TGNAM})

file(MAKE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/generated)
# generate the header file into the source tree as it is included in the RP2040 datasheet
pico_generate_pio_header(${TGNAM} ${CMAKE_CURRENT_LIST_DIR}/ws2812_simple.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)

target_sources(${TGNAM} PRIVATE ws2812_simple.c)

target_link_libraries(${TGNAM} PRIVATE pico_stdlib hardware_pio)
# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(${TGNAM})
pico_set_program_url(${TGNAM} "${TGNAM}")
