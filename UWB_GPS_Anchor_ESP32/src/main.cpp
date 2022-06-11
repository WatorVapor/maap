#include <Arduino.h>

#define GPS_ Serial1
#define UWB_ Serial2

static const int GPS_TX_PIN = 26; 
static const int GPS_RX_PIN = 27; 
static const int UWB_TX_PIN = 17; 
static const int UWB_RX_PIN = 16; 

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("UWB GPS Anchor start");
  GPS_.begin(115200,SERIAL_8N1,GPS_TX_PIN,GPS_RX_PIN);
  UWB_.begin(115200,SERIAL_8N1,UWB_TX_PIN,UWB_RX_PIN);
  delay(5000);
  UWB_.print("AT+switchdis=1\r\n");
}

void loop() {
  // put your main code here, to run repeatedly:
  if(GPS_.available()) {
    auto ch = GPS_.read();
    Serial.write(ch);
  }
  if(UWB_.available()) {
    auto ch = UWB_.read();
    Serial.write(ch);
  }
}
