#include <Arduino.h>

#include "debug.hpp"

void uwb_setup(void);
void uwb_loop(void);

void setup() {
  Serial.begin(115200);
	delay(1000);
  Serial.printf("Go Go Go!!!!\r\n");
  uwb_setup();
}

void loop() {
  uwb_loop();
}
