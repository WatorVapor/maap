#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#include <ArduinoJson.h>
#include <Preferences.h>

#include <sstream>
#include <string>
#include <mutex>
#include <list>
#include "debug.hpp"
#include "pref.hpp"

BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      LOG_I(deviceConnected);
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      LOG_I(deviceConnected);
    }
};

static Preferences preferences;

extern bool isPreferenceAllow;
static void savePref(const char * key,const std::string &value){
  LOG_SC(key);
  LOG_S(value);
  int waitPressCounter = 10;
  while(waitPressCounter -- > 0) {
    if(isPreferenceAllow){
      LOG_I(isPreferenceAllow);
      auto retPref = preferences.putString(key,value.c_str());
      LOG_I(retPref);
      return;
    }
    delay(1000);
  }
}
static void onSettingMqttTopic(const JsonObject &topic) {
  if(topic.containsKey("out")) {
    std::string outStr;
    serializeJson(topic["out"], outStr);
    savePref(strConstMqttTopicOutKey,outStr);
  }
}

static void onSettingCommand(const JsonObject &setting) {
  preferences.begin(preferencesZone);
  if(setting.containsKey("wifi")) {
    auto wifi = setting["wifi"];
    std::string ssid;
    if(wifi.containsKey("ssid")) {
      ssid = wifi["ssid"].as<std::string>();
    }
    std::string password;
    if(wifi.containsKey("password")) {
      password = wifi["password"].as<std::string>();
    }
    if(ssid.empty() == false && password.empty() == false) {
      savePref(strConstWifiSsidKey,ssid);
      savePref(strConstWifiPasswordKey,password);
    }
  }
  if(setting.containsKey("mqtt")) {
    auto mqtt = setting["mqtt"];
    if(mqtt.containsKey("server")) {
      auto server = mqtt["server"];
      if(server.containsKey("server")) {
        auto url = server["url"].as<std::string>();
        savePref(strConstMqttURLKey,url);
      }
      if(server.containsKey("jwt")) {
        auto jwt = server["jwt"].as<std::string>();
        savePref(strConstMqttURLKey,jwt);
      }
    }
    if(mqtt.containsKey("topic")) {
      auto topic = mqtt["topic"].as<JsonObject>();
      onSettingMqttTopic(topic);
    }
  }
  preferences.end();
}



void onExternalCommand(StaticJsonDocument<256> &doc) {
  if(doc.containsKey("uwb")) {
    auto uwb = doc["uwb"];
    if(uwb.containsKey("AT")) {
      auto atCmd = uwb["AT"].as<std::string>();
      LOG_S(atCmd);
    }
  }
  if(doc.containsKey("setting")) {
    auto setting = doc["setting"].as<JsonObject>();
    onSettingCommand(setting);
  }
}

class MyCallbacks: public BLECharacteristicCallbacks {
  void onRead(BLECharacteristic *pCharacteristic) {
    //pCharacteristic->setValue("Hello World!");
    std::string value = pCharacteristic->getValue();
  }

  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    LOG_S(value);
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, value);
    LOG_S(error);
    if(error == DeserializationError::Ok) {
      onExternalCommand(doc);
    }
  }
};


#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_TX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"


void setupBLE(void) {
  BLEDevice::init("UWB_GPS_Anchor ESP32");  // local name
  pServer = BLEDevice::createServer();  // Create the BLE Device
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pTxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX,BLECharacteristic::PROPERTY_NOTIFY);
  pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX,BLECharacteristic::PROPERTY_WRITE										);
  pRxCharacteristic->setCallbacks(new MyCallbacks());

  pService->start();
  pServer->getAdvertising()->start();

  Serial.println("Waiting a client connection to notify...");
}

std::string my_to_string(int i) {
  std::stringstream ss;
  ss << i;
  return ss.str();
}

std::string my_to_string_f(float f) {
  std::stringstream ss;
  ss << f;
  return ss.str();
}

extern std::mutex gGpsLineMtx;
extern std::list <std::string> gGPSLineBuff;
void reportGPS(void)
{
  std::lock_guard<std::mutex> lock(gGpsLineMtx);
  if(gGPSLineBuff.empty() == false) {
    auto line = gGPSLineBuff.front();
    pTxCharacteristic->setValue(line);
    pTxCharacteristic->notify();
    gGPSLineBuff.pop_front();
  }
}
void discardGPS(void) {
  std::lock_guard<std::mutex> lock(gGpsLineMtx);
  if(gGPSLineBuff.empty() == false) {
    gGPSLineBuff.clear();
  }  
}

extern std::mutex gUWBLineMtx;
extern std::list <std::string> gUWBLineBuff;

void reportUWB(void)
{
  std::lock_guard<std::mutex> lock(gUWBLineMtx);
  if(gUWBLineBuff.empty() == false) {
    auto line = gUWBLineBuff.front();
    pTxCharacteristic->setValue(line);
    pTxCharacteristic->notify();
    gUWBLineBuff.pop_front();
  }
}
void discardUWB(void) {
  std::lock_guard<std::mutex> lock(gUWBLineMtx);
  if(gUWBLineBuff.empty() == false) {
    gUWBLineBuff.clear();
  }
}


void runBleTransimit(void)
{
  if (deviceConnected) {
    reportGPS();
    reportUWB();
  } else {
    discardGPS();
    discardUWB();
  }
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
  // do stuff here on connecting
      oldDeviceConnected = deviceConnected;
  }
}

void BLETask( void * parameter) {
  int core = xPortGetCoreID();
  LOG_I(core);
  setupBLE();
  for(;;) {//
    runBleTransimit();
    delay(1);
  }
}
