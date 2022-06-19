#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <map>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <Preferences.h>

#include <base64.hpp>
#include "mbedtls/sha1.h"
#include "base32/Base32.h"
extern "C" {
  #include <tweetnacl.h>
}
#include "debug.hpp"
#include "pref.hpp"
#include "WebSocketClient250.h"
#include "WebSocketStreamClient.h"

/*
static const char* ssid = "mayingkuiG";
static const char* password = "xuanxuanhaohao";
*/


static const char* mqtt_server = "broker.emqx.io";

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

std::string sha1Address(byte *msg,size_t length) {
  mbedtls_sha1_context sha1_ctx;
  mbedtls_sha1_init(&sha1_ctx);
  mbedtls_sha1_starts(&sha1_ctx);
  mbedtls_sha1_update(&sha1_ctx, msg, length);
  byte digest[20] = {0};
  mbedtls_sha1_finish(&sha1_ctx, digest);
  Base32 base32;
  byte *digestB32 = NULL;
  auto b32Ret = base32.toBase32(digest,sizeof(digest),digestB32,true);
  DUMP_I(b32Ret);
  std::string result((char*)digestB32,b32Ret);
  return result;
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

void miningAddress(void) {
  Serial.println("Waiting mining address ...");
  byte secretKey[crypto_sign_SECRETKEYBYTES] = {0};
  byte publicKey[crypto_sign_PUBLICKEYBYTES] = {0};
  DUMP_H(secretKey,sizeof(secretKey));
  DUMP_H(publicKey,sizeof(publicKey));
  std::string address;
  while(true)
  {
    crypto_sign_keypair(publicKey,secretKey);
    DUMP_H(secretKey,sizeof(secretKey));
    DUMP_H(publicKey,sizeof(publicKey));
    byte mineSha512[crypto_hash_sha512_BYTES] = {0};
    auto hashRet = crypto_hash_sha512(mineSha512,publicKey,crypto_sign_PUBLICKEYBYTES);
    DUMP_I(hashRet);
    address = sha1Address(mineSha512,sizeof(mineSha512));
    DUMP_S(address);
/*
    break;
    if(address.at(0) == 'm') {
      break;
    }
*/
    if(address.at(0) == 'm' && address.at(1) == 'p' ) {
      break;
    }
    if(address.at(0) == 'm' && address.at(1) == 'a' && address.at(2) == 'a' && address.at(3) == 'p' ) {
      break;
    }
  }
  LOG_S(address);
  byte publicKeyB64[crypto_sign_PUBLICKEYBYTES*2] = {0};
  int b64Ret = encode_base64(publicKey,crypto_sign_PUBLICKEYBYTES,publicKeyB64);
  LOG_I(b64Ret);
  std::string pubKeyB64((char*)publicKeyB64,b64Ret);
  LOG_S(pubKeyB64);
  
  LOG_H(secretKey,sizeof(secretKey));
  byte secretKeyB64[crypto_sign_SECRETKEYBYTES*2] = {0};
  int b64Ret2 = encode_base64(secretKey,crypto_sign_SECRETKEYBYTES,secretKeyB64);
  LOG_I(b64Ret2);
  std::string secKeyB64((char*)secretKeyB64,b64Ret2);
  LOG_S(secKeyB64);
  auto goodPref = preferences.begin(preferencesZone);
  LOG_I(goodPref);
  savePref2(strConstEdtokenAddressKey,address);
  savePref2(strConstEdtokenPublicKey,pubKeyB64);
  savePref2(strConstEdtokenSecretKey,secKeyB64);
  preferences.end();
}

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


void MQTTTask( void * parameter){
  int core = xPortGetCoreID();
  LOG_I(core);
  setupMQTT();
  auto goodPref = preferences.begin(preferencesZone);

  auto jwt_host = preferences.getString(strConstMqttJWTHostKey);
  auto jwt_port = preferences.getInt(strConstMqttJWTPortKey);
  auto jwt_path = preferences.getString(strConstMqttJWTPathKey);

  auto mqtt_host = preferences.getString(strConstMqttURLHostKey);
  auto mqtt_port = preferences.getInt(strConstMqttURLPortKey);
  auto mqtt_path = preferences.getString(strConstMqttURLPathKey);
  preferences.end();

  WiFiClientSecure wiFiClient;
  WebSocketClient250 wsClient(wiFiClient,mqtt_host.c_str(),mqtt_port);
  WebSocketStreamClient wsStreamClient(wsClient,mqtt_path.c_str());
  static PubSubClient client(wsStreamClient);

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  for(;;) {//
    if (!client.connected()) {
      reconnect(client);
    }
    client.loop();
    delay(1);
  }
}
