#include <Arduino.h>
#include "port.h"
void uwb_setup (void) {
    peripherals_init();
    spi_peripheral_init();
}
void uwb_one_loop(void) {
}