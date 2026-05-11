#include <stdio.h>
#include "pico/stdlib.h"

int main(void) {
    setup_default_uart();
    sleep_ms(5000);
    printf("Hello, world!\n");
    return 0;
}