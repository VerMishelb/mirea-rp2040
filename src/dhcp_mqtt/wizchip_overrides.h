#pragma once


// wizchip_spi.h
#undef SPI_PORT
#define SPI_PORT spi0
#undef PIN_SCK
#define PIN_SCK 2 // SCK
#undef PIN_MOSI
#define PIN_MOSI 3 // TX
#undef PIN_MISO
#define PIN_MISO 4 // RX
#undef PIN_CS
#define PIN_CS 5 // CSn
#undef PIN_RST
#define PIN_RST 29
