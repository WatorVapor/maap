#include <Arduino.h>
#include <ArduinoJson.h>
#include <map>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

#include <Preferences.h>

#include "mbedtls/sha1.h"
#include "mybase64.hpp"
#include "base32/Base32.h"
extern "C" {
  #include <tweetnacl.h>
}
#include "debug.hpp"
#include "pref.hpp"

/*
static const char* ssid = "mayingkuiG";
static const char* password = "xuanxuanhaohao";
*/



static StaticJsonDocument<1024> gMattMsgDoc;


static unsigned char gBase64TempBinary[512];
static unsigned char gOpenedTempMsg[512];

static uint8_t gSignBinary[512];
static uint8_t gPublicKeyBinary[32];
static const int SHA512_HASH_BIN_MAX = 512/8;
static unsigned char gCalcTempHash[SHA512_HASH_BIN_MAX];


bool verifySign(const std::string &pub,const std::string &sign,const std::string &sha);
bool isAuthedMessage(const JsonVariant &msg,const std::string &topic,const std::string &payloadStr);

void ExecGpio(int port,int level) {
  LOG_I(port);
  LOG_I(level);
  pinMode(port,OUTPUT);
  digitalWrite(port,level);
}

void onMqttDigitalOut(const JsonVariant &d_out) {
  if(d_out.containsKey("port")) {
    int port = d_out["port"].as<int>();
    if(d_out.containsKey("level")) {
      int level = d_out["level"].as<int>();
      ExecGpio(port,level);
    }
  }
}


void onMqttAuthedMsg(const JsonVariant &payload) {
  if(payload.containsKey("d_out")) {
    onMqttDigitalOut(payload["d_out"]);
  }
}

void execMqttMsg(const std::string &msg,const std::string &topic) {
  DUMP_S(msg);
  DUMP_S(topic);
  DeserializationError error = deserializeJson(gMattMsgDoc, msg);
  DUMP_S(error);
  if(error == DeserializationError::Ok) {
    std::string payloadStr;
    if(gMattMsgDoc.containsKey("payload")) {
      payloadStr = gMattMsgDoc["payload"].as<std::string>();
      DUMP_S(payloadStr);
    }

    if(gMattMsgDoc.containsKey("auth")) {
      JsonVariant auth = gMattMsgDoc["auth"];
      bool isGood = isAuthedMessage(auth,topic,payloadStr);
      if(isGood) {
        LOG_I(isGood);
        if(gMattMsgDoc.containsKey("payload")) {
          JsonVariant payload = gMattMsgDoc["payload"];
          onMqttAuthedMsg(payload);
        }
      } else {
        LOG_I(isGood);
        LOG_S(msg);
      }
    }
  }
}

static std::map<std::string,std::string> gChannelTempMsg;

void insertTopicMsg(const std::string &msg,const std::string &topic){
    auto ir = gChannelTempMsg.find(topic);
    if(ir == gChannelTempMsg.end()) {
      gChannelTempMsg[topic] = msg;
    } else {
      ir->second += msg;
    }
}
void processOneMqttMsg(const std::string &topic){
    auto ir = gChannelTempMsg.find(topic);
    if(ir != gChannelTempMsg.end()) {
      execMqttMsg(ir->second,ir->first);
      gChannelTempMsg.erase(ir);
    }
}


void onMqttMsg(StaticJsonDocument<256> &doc,const std::string &topic ){
  if(doc.containsKey("buff")) {
    std::string buffStr = doc["buff"].as<std::string>();
    DUMP_S(buffStr);
    if(doc.containsKey("finnish")) {
      bool finnish = doc["finnish"].as<bool>();
      insertTopicMsg(buffStr,topic);
      if(finnish) {
        processOneMqttMsg(topic);
      }
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  std::string topicStr(topic);
  DUMP_S(topicStr);
  std::string payloadStr((char*)payload,length);
  DUMP_S(payloadStr);
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, payloadStr);
  DUMP_S(error);
  if(error == DeserializationError::Ok) {
    onMqttMsg(doc,topicStr);
  }
}

static Preferences preferences;
extern bool isPreferenceAllow;

static void savePref2(const char * key,const std::string &value){
  LOG_SC(key);
  LOG_S(value);
  auto retPref = preferences.putString(key,value.c_str());
  LOG_I(retPref);
  return;
}
void miningAddress(void);

static String gAddress;

void setupMQTT(void) {
  auto goodPref = preferences.begin(preferencesZone);
  LOG_I(goodPref);
  gAddress = preferences.getString(strConstEdtokenAddressKey);
  auto pubKeyB64 = preferences.getString(strConstEdtokenPublicKey);
  auto secKeyB64 = preferences.getString(strConstEdtokenSecretKey);

  auto ssid = preferences.getString(strConstWifiSsidKey);
  auto password = preferences.getString(strConstWifiPasswordKey);
  LOG_S(ssid);
  LOG_S(password);

  preferences.end();
  if(gAddress.isEmpty() || pubKeyB64.isEmpty() || secKeyB64.isEmpty()) {
    miningAddress();
    auto goodPref2 = preferences.begin(preferencesZone);
    LOG_I(goodPref2);
    gAddress = preferences.getString(strConstEdtokenAddressKey);
    pubKeyB64 = preferences.getString(strConstEdtokenPublicKey);
    secKeyB64 = preferences.getString(strConstEdtokenSecretKey);
    preferences.end();
  }
  LOG_S(gAddress);
  LOG_S(pubKeyB64);
  LOG_S(secKeyB64);


  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  while ( true ) {
    auto result = WiFi.waitForConnectResult();
    LOG_I(result);
    if(result == WL_CONNECTED){
      break;
    }
    delay(1000);
  }
  LOG_S(WiFi.localIP().toString());
  LOG_S(WiFi.localIPv6().toString());
  Serial.println("Wifi Is Ready");
}



#include <list>


void subscribeAtConnected(PubSubClient client) {
  client.subscribe(gAddress.c_str(),1);
}

void reconnect(PubSubClient &client) {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "public-iot";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    auto rc = client.connect(clientId.c_str());
    if (rc) {
      LOG_I(client.connected());
      // ... and resubscribe
      subscribeAtConnected(client);
    } else {
      LOG_I(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void JWTTask( void * parameter);
void MQTTTask( void * parameter){
  int core = xPortGetCoreID();
  LOG_I(core);
  setupMQTT();
  
  xTaskCreatePinnedToCore(JWTTask, "JWTTask", 10000, nullptr, 1, nullptr,  1); 

  auto goodPref = preferences.begin(preferencesZone);
  auto mqtt_host = preferences.getString(strConstMqttURLHostKey);
  auto mqtt_port = preferences.getInt(strConstMqttURLPortKey);
  auto mqtt_path = preferences.getString(strConstMqttURLPathKey);
  preferences.end();

  

/*  
  WebSocketClient250 wsClient(wiFiClient,mqtt_host.c_str(),mqtt_port);
  WebSocketStreamClient wsStreamClient(wsClient,mqtt_path.c_str());
*/
  WiFiClientSecure wiFiClient;
  static PubSubClient client(wiFiClient);

  client.setServer(mqtt_host.c_str(), (uint16_t)mqtt_port);
  client.setCallback(callback);

  for(;;) {//
    #if 0
    if (!client.connected()) {
      reconnect(client);
    }
    client.loop();
    #endif
    delay(1);
  }
}
