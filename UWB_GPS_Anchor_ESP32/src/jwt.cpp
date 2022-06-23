#include <Arduino.h>
#include <ArduinoJson.h>
#include <map>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Preferences.h>




//#include <base64.hpp>
#include "mbedtls/sha1.h"
#include "base32/Base32.h"
extern "C" {
  #include <tweetnacl.h>
}
#include "debug.hpp"
#include "pref.hpp"

static Preferences preferences;

void JWTTask( void * parameter){
  int core = xPortGetCoreID();
  LOG_I(core);
  auto goodPref = preferences.begin(preferencesZone);
  LOG_I(goodPref);
  auto jwt_url = preferences.getString(strConstMqttJWTKey);
  LOG_S(jwt_url);
  auto jwt_host = preferences.getString(strConstMqttJWTHostKey);
  LOG_S(jwt_host);
  auto jwt_port = preferences.getInt(strConstMqttJWTPortKey);
  LOG_I(jwt_port);
  auto jwt_path = preferences.getString(strConstMqttJWTPathKey);
  LOG_S(jwt_path);
  preferences.end();
  
  WiFiClientSecure client;
  client.setInsecure();
  client.connect(jwt_host.c_str(),(uint16_t)jwt_port);
  for(;;) {
    delay(1);
  }
}
