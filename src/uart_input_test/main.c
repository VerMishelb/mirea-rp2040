#include <stdio.h>
#include "pico/stdlib.h"
#include "minidebug.h"
#include "ws2812.h"

void wait_for_enter(void) {
    miniprintf("Press Enter to continue\n");
    while (getchar() != '\n');
}

int main(void) {
    stdio_init_all();
    WS2812_INIT();
    ws2812_put_pixel(COLOUR_GRB_YELLOW);
    printf("Hello, world!\n");
    sleep_ms(5000);
    ws2812_put_pixel(COLOUR_GRB_GREEN);
    printf("Hello, world!\n");
    miniprintf("Hello mdbg!\n");
    miniprintf(minidebug_SGR(minidebug_SGR_COLOUR_F_MAGENTA) "Colour test (magenta)" minidebug_SGR(minidebug_SGR_COLOUR_DEFAULT) "\n");
    miniprintf("Тест кириллицы (UTF-8)\n");
    /* Должно проскочить паузу, если нет связи с ПК. */
    if (stdio_usb_connected()) {
        ws2812_put_pixel(COLOUR_GRB_VIOLET);
        wait_for_enter();
    }
    miniprintf("text!\n");
    while (1) {
        ws2812_put_pixel(COLOUR_GRB_BLUE);
        miniprintf("text!\n");
        sleep_ms(50);
        ws2812_put_pixel(COLOUR_GRB_OFF);
        sleep_ms(500-50);
    }
    WS2812_FREE();
    return 0;
}