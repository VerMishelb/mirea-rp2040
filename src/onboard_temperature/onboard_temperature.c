/**
 * Copyright (c) 2021 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "ws2812.h"

#include <minidebug.h>
#line 15 "src/onboard_temperature/onboard_temperature.c"

/* Choose 'C' for Celsius or 'F' for Fahrenheit. */
#define TEMPERATURE_UNITS 'C'
//#define TEMPERATURE_DEBUG_OUTPUT

const int16_t temperature_calibration = 0;

/* References for this implementation:
 * raspberry-pi-pico-c-sdk.pdf, Section '4.1.1. hardware_adc'
 * pico-examples/adc/adc_console/adc_console.c */
float read_onboard_temperature(const char unit) {
    
    /* 12-bit conversion, assume max value == ADC_VREF == 3.3 V */
    const float conversionFactor = 3.3f / (1 << 12); // 0,0008056640625
    
    uint16_t val_adc = adc_read();
    float adc = ((float)(val_adc + temperature_calibration)) * conversionFactor;
    float tempC = 27.0f - (adc - 0.706f) / 0.001721f;
    #ifdef TEMPERATURE_DEBUG_OUTPUT
    mish_printf("adc = %.04X (%.09f)\n", val_adc, adc);
    #endif

    if (unit == 'C') {
        return tempC;
    }
    else if (unit == 'F') {
        return tempC * 9 / 5 + 32;
    }

    return -1.0f;
}

int main() {
    stdio_init_all();
    WS2812_INIT();

    /* Initialize hardware AD converter, enable onboard temperature sensor and
     *   select its channel (do this once for efficiency, but beware that this
     *   is a global operation). */
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4);

    while (true) {
        float temperature = read_onboard_temperature(TEMPERATURE_UNITS);
        printf("Onboard temperature = %.02f %c\n", temperature, TEMPERATURE_UNITS);
        miniprintf("Onboard temperature = %.02f %c\n", temperature, TEMPERATURE_UNITS);
        miniprintfval("%.02f %c", temperature, TEMPERATURE_UNITS);
        miniprintfloc();
        #ifdef TEMPERATURE_DEBUG_OUTPUT
        ws2812_put_pixel(COLOUR_GRB_YELLOW);
        #else
        ws2812_put_pixel(COLOUR_GRB_VIOLET);
        #endif
        sleep_ms(10);
        ws2812_put_pixel(0);

        sleep_ms(990);
    }

    WS2812_FREE();
}
