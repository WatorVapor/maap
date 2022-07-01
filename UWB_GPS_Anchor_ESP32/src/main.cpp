#include <Arduino.h>
#include <string>
#include <mutex>
#include <list>


#define GPS_ Serial1
#define UWB_ Serial2

void BLETask( void * parameter);
void MQTTTask( void * parameter);

static const int GPS_TX_PIN = 27; 
static const int GPS_RX_PIN = 26; 
static const int UWB_TX_PIN = 16; 
static const int UWB_RX_PIN = 17; 

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("UWB GPS Anchor start");
  GPS_.begin(115200,SERIAL_8N1,GPS_RX_PIN,GPS_TX_PIN);
  UWB_.begin(115200,SERIAL_8N1,UWB_RX_PIN,UWB_TX_PIN);

  xTaskCreatePinnedToCore(BLETask, "BLETask", 10000, nullptr, 1, nullptr,  1); 
  xTaskCreatePinnedToCore(MQTTTask, "MQTTTask", 10000, nullptr, 1, nullptr,  1); 

  delay(5000);
  UWB_.print("AT+switchdis=1\r\n");
}


bool isPreferenceAllow = false;

static const int iConstGPSLineMax = 1024;
static std::string gpsLine;
std::mutex gGpsLineMtx;
std::list <std::string> gGPSLineBuff;
static const int iConstUWBLineMax = 128;
static std::string uwbLine;
std::mutex gUWBLineMtx;
std::list <std::string> gUWBLineBuff;

//#define UART_DIR 1


void loop() {
  // put your main code here, to run repeatedly:
  if(GPS_.available()) {
    auto ch = GPS_.read();
#ifdef UART_DIR
    Serial.write(ch);
#endif
    gpsLine.push_back(ch);
    if(gpsLine.size() > iConstGPSLineMax) {
      gpsLine.clear();
    }
    if(ch == '\n') {
      std::lock_guard<std::mutex> lock(gGpsLineMtx);
      gGPSLineBuff.push_back(gpsLine);
      gpsLine.clear();
    }
  }
  if(UWB_.available()) {
    auto ch = UWB_.read();
#ifdef UART_DIR
    Serial.write(ch);
#endif
    {
      std::lock_guard<std::mutex> lock(gUWBLineMtx);
      uwbLine.push_back(ch);
      if(uwbLine.size() > iConstUWBLineMax) {
        uwbLine.clear();
      }
    }
    if(ch == '\n') {
      std::lock_guard<std::mutex> lock(gUWBLineMtx);
      gUWBLineBuff.push_back(uwbLine);
      uwbLine.clear();   
    }
  }
  isPreferenceAllow = true;
}
