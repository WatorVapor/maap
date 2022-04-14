#include <Arduino.h>
#include "pico/multicore.h"

void uwb_main(void);
void core1_main(void) {
  uwb_main();
}

void setup() {
  multicore_launch_core1(core1_main);
}

void loop() {
}

