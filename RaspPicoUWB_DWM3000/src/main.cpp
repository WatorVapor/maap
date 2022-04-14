#include <Arduino.h>
#include "pico/multicore.h"

void uwb_setup(void);
void uwb_one_loop(void);
void c(void);
void setup() {
  Serial.begin(115200);
  Serial.print("core0:start....\r\n");
  uwb_setup();
}

void loop() {
  Serial.print("core0:start....\r\n");
  uwb_one_loop();
  delay(100);
}

