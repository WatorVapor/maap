#include <Arduino.h>

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(PC13, OUTPUT);
	delay(1000);
  Serial.printf("Go Go Go!!!!\r\n");
}

void loop() {
  // put your main code here, to run repeatedly:
	digitalWrite(PC13, HIGH);
	delay(500);
	digitalWrite(PC13, LOW);
	delay(500);
}
