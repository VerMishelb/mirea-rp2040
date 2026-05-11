#ifndef WS2812_h_
#define WS2812_h_

/*
Использование:
В CMakeLists добавить
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/generated)
    pico_generate_pio_header(${TGNAM} ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)
Там же в target_include_directories дописать hardware_pio и добавить папку с кодом в список поиска для #include:
    target_link_libraries(${TGNAM} pico_stdlib hardware_adc hardware_pio)
    target_include_directories(${TGNAM} PRIVATE ${CMAKE_CURRENT_LIST_DIR})

В коде после stdio_init_all() поставить WS2812_INIT();
В конце программы поставить WS2812_FREE();
Между этими вызовами можно управлять диодом, используя ws2812_put_pixel.
*/

#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "generated/ws2812.pio.h"

#define IS_RGBW false

#ifdef PICO_DEFAULT_WS2812_PIN
#define WS2812_PIN PICO_DEFAULT_WS2812_PIN
#else
// default to pin 2 if the board doesn't have a default WS2812 pin defined
#define WS2812_PIN 2
#endif

// Check the pin is compatible with the platform
#if WS2812_PIN >= NUM_BANK0_GPIOS
#error Attempting to use a pin>=32 on a platform that does not support it
#endif

enum WS2812_GRB_COLOURS {
    COLOUR_GRB_OFF = 0,
    COLOUR_GRB_BRIGHT_RED = 0x00FF00,
    COLOUR_GRB_BRIGHT_GREEN = 0xFF0000,
    COLOUR_GRB_BRIGHT_BLUE = 0x0000FF,
    COLOUR_GRB_WHITE = 0x7F7F7F,
    COLOUR_GRB_RED = 0x007F00,
    COLOUR_GRB_GREEN = 0x7F0000,
    COLOUR_GRB_BLUE = 0x00007F,
    COLOUR_GRB_ORANGE = 0x1F7F00,
    COLOUR_GRB_YELLOW = 0x7F7F00,
    COLOUR_GRB_VIOLET = 0x001F7F
};

static inline void ws2812_put_pixel_(PIO pio, uint sm, uint32_t pixel_grb) {
    pio_sm_put_blocking(pio, sm, pixel_grb << 8u);
}
#define ws2812_put_pixel(x) ws2812_put_pixel_(pio_ws2812, sm_ws2812, (x))

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return
            ((uint32_t) (r) << 8) |
            ((uint32_t) (g) << 16) |
            (uint32_t) (b);
}

static inline uint32_t urgbw_u32(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
    return
            ((uint32_t) (r) << 8) |
            ((uint32_t) (g) << 16) |
            ((uint32_t) (w) << 24) |
            (uint32_t) (b);
}



#define WS2812_INIT() \
PIO pio_ws2812; \
uint sm_ws2812; \
uint offset_ws2812; \
bool success_ws2812 = pio_claim_free_sm_and_add_program_for_gpio_range(&ws2812_program, &pio_ws2812, &sm_ws2812, &offset_ws2812, WS2812_PIN, 1, true); \
hard_assert(success_ws2812); \
ws2812_program_init(pio_ws2812, sm_ws2812, offset_ws2812, WS2812_PIN, 800000, IS_RGBW);

#define WS2812_FREE() \
pio_remove_program_and_unclaim_sm(&ws2812_program, pio_ws2812, sm_ws2812, offset_ws2812);


#endif