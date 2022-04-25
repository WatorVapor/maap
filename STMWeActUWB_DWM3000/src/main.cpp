#include <Arduino.h>

#include "debug.hpp"

void uwb_setup(void);
void uwb_loop(void);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(PC13, OUTPUT);
	delay(1000);
  Serial.printf("Go Go Go!!!!\r\n");
  uwb_setup();
}

void loop() {
  // put your main code here, to run repeatedly:
	digitalWrite(PC13, HIGH);
	delay(10);
	digitalWrite(PC13, LOW);
	delay(10);
  uwb_loop();
}
