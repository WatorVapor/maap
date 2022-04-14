#include <Arduino.h>
#include <SPI.h>

#define PIN_SCK  2
#define PIN_MOSI 3
#define PIN_MISO 4
#define PIN_CS   5

static int32_t SPI_CLOCK_HIGH = 32 * 1000 * 1000;

void uwb_setup (void) {
    SPISettings mySPISettings = SPISettings(SPI_CLOCK_HIGH, MSBFIRST, SPI_MODE0);
    SPI.begin();
}
void uwb_main(void) {
    uwb_setup();
    while(true) {

    }
}