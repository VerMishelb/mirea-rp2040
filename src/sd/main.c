/*
Читает в корне флешки test.txt, создаёт в корне флешки файл test_out.txt.

Модуль карты висит на SPI 1.
3v3     На модуле Ethernet
CS      13 (SPI1 CSn)
MOSI    15 (SPI1 TX)
CLK     14 (SPI1 SCK)
MISO    12 (SPI1 RX)
GND     GND
*/

#include <stdio.h>
#include "pico/stdlib.h"

#include "minidebug.h"
#include "ws2812.h"

// SD card (https://github.com/carlk3/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico)
#include "f_util.h"
#include "ff.h"
#include "hw_config.h" // Редактировать пины в hw_config.c. Пин "ss_gpio (SPI Slave) = SCn.
//                                                   !!! ^

const char* const filename = "test.txt";
const char* const filename_write = "test_out.txt";

#define LED(x) { ws2812_put_pixel((x)); sleep_us(50); }

void wait_for_enter(void) {
    miniprintf("Press Enter to continue\n");
    char c = 0;
    c = getchar();
}


int main(void)  {
    stdio_init_all();
    WS2812_INIT();
    LED(COLOUR_GRB_WHITE);

    // По какой-то причине пролетает это насквозь, даже если плата подключена. Поэтому здесь временно "1".
    if (1 || stdio_usb_connected()) {
        LED(COLOUR_GRB_VIOLET);
        wait_for_enter();
    }
    else {
        LED(COLOUR_GRB_ORANGE);
    }
    LED(COLOUR_GRB_GREEN);
    miniprintf("Starting...\n");

    FATFS fs;
    FRESULT fr = f_mount(&fs, "", 1);
    if (FR_OK != fr) {
        /* Кидает FR_NOT_READY с картой exFAT 128 ГБ.
        Библиотека не поддерживает SD <= 2 ГБ по какой-то причине (https://github.com/carlk3/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/issues/111).
        lib/vendor/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico-3.6.2/src/ff15/source/ff.c:3430
        lib/vendor/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico-3.6.2/src/ff15/source/diskio.c:61 */
        LED(COLOUR_GRB_RED);
        miniprintf("Померло.\n");
        panic("f_mount error: %s (%d)\n", FRESULT_str(fr), fr);
    }

    // Чтение
    FIL fil;
    fr = f_open(&fil, filename, FA_READ);
    if (FR_OK != fr && FR_EXIST != fr) {
        LED(COLOUR_GRB_RED);
        miniprintf("Померло.\n");
        panic("f_open(%s) error: %s (%d)\n", filename, FRESULT_str(fr), fr);
    }
    
    char buf[128] = {0};
    f_gets(buf, sizeof(buf), &fil);
    miniprintf("f_gets(buf, sizeof(buf), &fil): \"" minidebug_COLOUR_DEFAULT "%s" minidebug_COLOUR_MSG "\"\n", buf);
    
    fr = f_close(&fil);
    if (FR_OK != fr) {
        LED(COLOUR_GRB_RED);
        miniprintf("f_close: %s (%d)\n", FRESULT_str(fr), fr);
    }
    
    // Запись
    fr = f_open(&fil, filename_write, FA_OPEN_APPEND | FA_WRITE);
    if (FR_OK != fr && FR_EXIST != fr) {
        LED(COLOUR_GRB_RED);
        miniprintf("Померло.\n");
        panic("f_open(%s): %s (%d)\n", filename_write, FRESULT_str(fr), fr);
    }

    if (f_printf(&fil, "Hello, world!\n") < 0) {
        LED(COLOUR_GRB_RED);
        miniprintf("f_printf failed\n");
    }

    fr = f_close(&fil);
    if (FR_OK != fr) {
        LED(COLOUR_GRB_RED);
        miniprintf("f_close: %s (%d)\n", FRESULT_str(fr), fr);
    }

    miniprintf("Выключаемся.\n");

    fr = f_unmount("");
    if (fr != FR_OK) {
        LED(COLOUR_GRB_RED);
        miniprintf("f_unmount: %s (%d)\n", FRESULT_str(fr), fr);
    }

    WS2812_FREE();

    return 0;
}