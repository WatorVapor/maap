#include <Arduino.h>
#include <ArduinoJson.h>
#include <map>
#include <HTTPClient.h>
#include <Preferences.h>
#include <WebSocketsClient.h>

#include "mbedtls/sha1.h"
#include "mybase64.hpp"
#include "base32/Base32.h"
extern "C" {
  #include <tweetnacl.h>
}
#include "debug.hpp"
#include "pref.hpp"

static Preferences preferences;
static WebSocketsClient webSocket;

static String createJWTRequest(const std::string &ts);
static String createDateRequest(void);

void onWSMsg(const StaticJsonDocument<512> &doc) {
  if(doc.containsKey("date")) {
    std::string dateStr = doc["date"].as<std::string>();
    createJWTRequest(dateStr);
  }
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
  case WStype_DISCONNECTED:
    {
      LOG_I(WStype_DISCONNECTED);
    }
    break;
  case WStype_CONNECTED:
    {
      LOG_I(WStype_CONNECTED);
      auto token = createDateRequest();
      webSocket.sendTXT(token);
    }
    break;
  case WStype_TEXT:
    {
      LOG_I(payload);
      LOG_I(length);
      std::string payloadStr((char*)payload,length);
      LOG_S(payloadStr);
      StaticJsonDocument<512> doc;
      DeserializationError error = deserializeJson(doc, payloadStr);
      DUMP_S(error);
      if(error == DeserializationError::Ok) {
        onWSMsg(doc);
      }
    }
    break;
  case WStype_BIN:
    break;
  case WStype_ERROR:			
  case WStype_FRAGMENT_TEXT_START:
  case WStype_FRAGMENT_BIN_START:
  case WStype_FRAGMENT:
  case WStype_FRAGMENT_FIN:
    break;
  }
}

StaticJsonDocument<512> signMsg(StaticJsonDocument<512> &msg,const std::string &ts);

static String createJWTRequest(const std::string &ts) {
  auto goodPref = preferences.begin(preferencesZone);
  LOG_I(goodPref);
  auto address = preferences.getString(strConstEdtokenAddressKey);
  preferences.end();
  StaticJsonDocument<512> doc;
  doc["jwt"]["address"] = address;
  auto signedRequest = signMsg(doc,ts);
  String request;
  serializeJson(signedRequest, request);
  return request;
}

static String createDateRequest(void) {
  StaticJsonDocument<256> doc;
  doc["date"] = "request";
  String request;
  serializeJson(doc, request);
  return request;
}


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

  webSocket.begin(jwt_host.c_str(),(uint16_t)jwt_port,jwt_path.c_str());
  webSocket.onEvent(webSocketEvent);
  for(;;) {
    webSocket.loop();
    delay(1);
  }
}
