#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <map>
#include <PubSubClient.h>
#include <Preferences.h>

#include <base64.hpp>
#include "base32/Base32.h"
extern "C" {
  #include <tweetnacl.h>
}
#include "debug.hpp"
#include "pref.hpp"


static const char* ssid = "mayingkuiG";
static const char* password = "xuanxuanhaohao";


static const char* mqtt_server = "broker.emqx.io";
static WiFiClient espClient;
static PubSubClient client(espClient);

static StaticJsonDocument<1024> gMattMsgDoc;


static unsigned char gBase64TempBinary[512];
static unsigned char gOpenedTempMsg[512];

static uint8_t gSignBinary[512];
static uint8_t gPublicKeyBinary[32];
static const int SHA512_HASH_BIN_MAX = 512/8;
static unsigned char gCalcTempHash[SHA512_HASH_BIN_MAX];


static bool verifySign(const std::string &pub,const std::string &sign,const std::string &sha){
  DUMP_S(pub);
  DUMP_S(sign);
  DUMP_S(sha);
  int pubRet = decode_base64((unsigned char*)pub.c_str(),pub.size(),gBase64TempBinary);
  DUMP_I(pubRet);
  memcpy(gPublicKeyBinary,gBase64TempBinary,sizeof(gPublicKeyBinary));
  DUMP_H(gPublicKeyBinary,sizeof(gPublicKeyBinary));
  int signRet = decode_base64((unsigned char*)sign.c_str(),sign.size(),gBase64TempBinary);
  DUMP_I(signRet);
  memcpy(gSignBinary,gBase64TempBinary,signRet);
  DUMP_H(gSignBinary,signRet);
  
  unsigned long long mSize = 0;
  unsigned long long signSize = signRet;
  int openRet = crypto_sign_open(gOpenedTempMsg,&mSize,gSignBinary,signSize,gPublicKeyBinary);
  if(openRet == 0){
    int shaRet = encode_base64(gOpenedTempMsg,SHA512_HASH_BIN_MAX,gBase64TempBinary);
    std::string shaOpened((char*)gBase64TempBinary,shaRet);
    if(shaOpened ==sha) {
      return true;
    }
    LOG_S(shaOpened);
    LOG_S(sha);
  }
  return false;
}
bool isAuthedMessage(const JsonVariant &msg,const std::string &topic,const std::string &payloadStr) {
  auto hashRet = crypto_hash_sha512(gCalcTempHash,(unsigned char*)payloadStr.c_str(),payloadStr.size());
  DUMP_I(hashRet);
  int shaRet = encode_base64(gCalcTempHash,SHA512_HASH_BIN_MAX,gBase64TempBinary);
  DUMP_I(shaRet);
  std::string calHash((char*)gBase64TempBinary,shaRet);
  DUMP_S(calHash);

  std::string pubStr;
  std::string signStr;
  std::string shaStr;
  if(msg.containsKey("pub")){
    pubStr = msg["pub"].as<std::string>();
    DUMP_S(pubStr);
  }
  if(msg.containsKey("sign")){
    signStr = msg["sign"].as<std::string>();
    DUMP_S(signStr);
  }
  if(msg.containsKey("sha")){
    shaStr = msg["sha"].as<std::string>();
    DUMP_S(shaStr);
  }
  if(shaStr != calHash) {
    return false;
  }
  if(pubStr.empty() == false && signStr.empty() == false && shaStr.empty() == false) {
    return verifySign(pubStr,signStr,shaStr);
  }
  return false;
}

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


void calcAddress(void) {
  byte secretKey[crypto_sign_BYTES] = {0};
  byte publicKey[crypto_sign_BYTES] = {0};
  DUMP_H(secretKey,sizeof(secretKey));
  DUMP_H(publicKey,sizeof(publicKey));
  //while(true)
  {
    crypto_sign_keypair(secretKey,publicKey);
    LOG_H(secretKey,sizeof(secretKey));
    LOG_H(publicKey,sizeof(publicKey));
    byte publicKeyB64[crypto_sign_BYTES*2] = {0};
    int shaRet = encode_base64(publicKey,crypto_sign_BYTES,publicKeyB64);
    LOG_I(shaRet);
    LOG_SC(publicKeyB64);
    //auto hashRet = crypto_hash_sha512(gCalcTempHash,(unsigned char*)payloadStr.c_str(),payloadStr.size());
  }
}

void setupMQTT(void) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
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
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}


//static const char *defaultTopics = "u51h7JJd6054erGLKjVvOqa6hhfzC/xWbLYhPZN3S0M=";

#include <list>

/*
extern std::list<std::string> gAllowTopics;
extern std::list<std::string> gDenyTopics;
*/

void subscribeAtConnected(void) {
  /*
  for(const auto topic :gAllowTopics) {
    for(const auto deny :gDenyTopics) {
      if(topic == deny) {
        continue;
      }
    }
    client.subscribe(topic.c_str(),1);
    // Once connected, publish an announcement...
    delay(1000);
    //client.publish(topic.c_str(), "hello world");
  }
  */
}

void reconnect() {
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
      subscribeAtConnected();
    } else {
      LOG_I(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void MQTTTask( void * parameter){
  int core = xPortGetCoreID();
  LOG_I(core);
  calcAddress();
  setupMQTT();
  for(;;) {//
    if (!client.connected()) {
      reconnect();
    }
    client.loop();
    delay(1);
  }
}
