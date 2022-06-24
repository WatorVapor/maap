#include <Arduino.h>
#include <ArduinoJson.h>
#include <map>
#include <WiFi.h>
#include <Preferences.h>

#include "mbedtls/sha1.h"
#include "mybase64.hpp"
#include "base32/Base32.h"
extern "C" {
  #include <tweetnacl.h>
}
#include "debug.hpp"
#include "pref.hpp"


static unsigned char gBase64TempBinary[512];
static unsigned char gOpenedTempMsg[512];
static uint8_t gSignBinary[512];
static uint8_t gPublicKeyBinary[32];
static const int SHA512_HASH_BIN_MAX = 512/8;
static unsigned char gCalcTempHash[SHA512_HASH_BIN_MAX];

bool verifySign(const std::string &pub,const std::string &sign,const std::string &sha){
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

