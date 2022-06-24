#include <Arduino.h>
#include <ArduinoJson.h>
#include <map>
#include <HTTPClient.h>
#include <Preferences.h>
#include <WebSocketsClient.h>

//#include <base64.hpp>
#include "mbedtls/sha1.h"
#include "base32/Base32.h"
extern "C" {
  #include <tweetnacl.h>
}
#include "debug.hpp"
#include "pref.hpp"

static Preferences preferences;
static WebSocketsClient webSocket;


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
            // send message to server when Connected
            //webSocket.sendTXT("Connected");
          }
          break;
      case WStype_TEXT:

    // send message to server
    // webSocket.sendTXT("message here");
          break;
      case WStype_BIN:
          // send data to server
          // webSocket.sendBIN(payload, length);
          break;
  case WStype_ERROR:			
  case WStype_FRAGMENT_TEXT_START:
  case WStype_FRAGMENT_BIN_START:
  case WStype_FRAGMENT:
  case WStype_FRAGMENT_FIN:
    break;
  }
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
