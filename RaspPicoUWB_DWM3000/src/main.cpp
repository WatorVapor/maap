#include <Arduino.h>
#include "debug.hpp"

void uwb_setup(void);
void uwb_one_loop(void);
void setup() {
  Serial.begin(115200);
  delay(5000);
  uwb_setup();
}

void loop() {
  //uwb_one_loop();
  //DUMP_VAR_I(1);
  delay(100);
}

