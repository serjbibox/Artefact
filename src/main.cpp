
/* Create a WiFi access point and provide a web server on it. */
#include <main.h>
#include <artefact.h>
#include <common.h>
#include <api.h>
#include <detector.h>
extern "C" {
#include <cstdint>
#include "user_interface.h"
}


#define EEPROM_LENGTH 64

#define SSID_LENGTH_LOC 0
#define TYPE_LOC 1

#define SSID_MAX_LENGTH 32
#define SSID_LOC 2

#define PASS_LENGTH 8
#define PASS_LOC 34

#define MAC_LENGTH 6
#define MAC_LOC 42

#define DET_LENGTH 1
#define DET_LOC 48

#define CATCH_LENGTH 1
#define CATCH_LOC 49
enum {
  TYPE_ART,
  TYPE_DET,
  TYPE_ANO,
  TYPE_CATCHED_ART
};

typedef struct{
  uint8_t Type;
  String SSID;
  uint8_t SsidLength;
  String Password;
  uint8_t MAC[6];
  int DetectLevel;
  int CatchLevel;
}WifiModule_t;
WifiModule_t Module;





uint8_t bufLength = 1;
String newAp;
uint8_t flag = 0;
char buf[EEPROM_LENGTH];
char detBuf[10];
char castchBuf[10];




bool isArtefact = false;


bool changeMac(const uint8_t mac[6])
{
  return wifi_set_macaddr(STATION_IF, const_cast<uint8*>(mac)) & wifi_set_macaddr(SOFTAP_IF, const_cast<uint8*>(mac));
}

int getDetectLevel(void){
  return Module.DetectLevel;
}
int getCatchLevel(void){
  return Module.CatchLevel;
}
uint8_t getType(void){
  return Module.Type;
}
String getSSID(void){
  return Module.SSID;
}
uint8_t* getMAC(void){
  return Module.MAC;
}

void formAction(String argName, String arg){
      if(argName == "APSSID") {
        flag = 1;
        Module.SSID = "";
        Module.SSID = arg;
        Serial.printf("Received SSID: %s, %i\n",String(Module.SSID), Module.SSID.length());
      }
      if(argName == "ARTDET") {
        if(arg.length()>0){
          Module.DetectLevel = arg.toInt();
          Serial.printf("Received ARTDET: %i, %i\n",Module.DetectLevel, arg.length());
        }
      }
      if(argName == "ARTCATCH") {
        if(arg.length()>0){
          Module.CatchLevel = arg.toInt();
          Serial.printf("Received ARTCATCH: %i, %i\n",Module.CatchLevel, arg.length());
        }
      }  
      if(argName == "APMAC") {
        if(arg.length()>0){
          flag = 1;
          stringToMac(arg, Module.MAC);
          Serial.print("Received MAC:");
          Serial.printf("stringToMac: %i:%i:%i:%i:%i:%i\n",
            Module.MAC[0], Module.MAC[1], Module.MAC[2], Module.MAC[3], Module.MAC[4], Module.MAC[5]);
          //changeMac(Module.MAC);
        }
      }       
}

void initFormAction(String arg){
  Serial.print("inflType: ");
  Serial.println(arg);
  while(1){
   if(arg == "isArt"){
      Module.Type = TYPE_ART;
      break;
    }
    if(arg == "isAno"){
      Module.Type = TYPE_ANO;
      break;
    }
    if(arg == "isCatchedArt"){
      Module.Type = TYPE_CATCHED_ART;
      Serial.println("catched!");
      break;
    }
    if(arg == "isDetector"){
      Module.Type = TYPE_DET;
      break;
    }
    if(arg == "reset"){
      system_restart();
      delay(2000);  
      break;
    }
    break;
  }
  setCheckedType(Module.Type);
  EEPROM.begin(EEPROM_LENGTH);

  EEPROM.write(TYPE_LOC, Module.Type);
  if(Module.Type == TYPE_DET){
    Module.MAC[0] = 0x00;
    Module.MAC[1] = 0x00;
    Module.MAC[2] = 0xAB;
    Module.MAC[3] = 0xCD;
    Module.MAC[4] = 0xEF;
    Module.MAC[5] = EEPROM.read(MAC_LOC);
  }
  EEPROM.end();
  yield();

}



void checkSettings(void){
  uint8_t i=0;
  EEPROM.begin(EEPROM_LENGTH);
  Module.SsidLength = EEPROM.read(SSID_LENGTH_LOC);
  if((Module.SsidLength == 0) | (Module.SsidLength>SSID_MAX_LENGTH)){
    Serial.println("First load... Writing default SSID");
    Module.SSID = "ESP2866_NEWAP";
    Module.SsidLength = Module.SSID.length();
    EEPROM.write(SSID_LENGTH_LOC, Module.SsidLength);
    for(i=0;i<Module.SsidLength;i++){
      EEPROM.write(i + SSID_LOC, Module.SSID[i]);
      yield();
    }
    digitalWrite(D2, HIGH);
    delay(500);
    digitalWrite(D2, LOW);
    delay(500);
    digitalWrite(D2, HIGH);
    delay(500);
    digitalWrite(D2, LOW);
    delay(500);
    delay(100);
    digitalWrite(D2, HIGH);    
  }else{
      char rune;
      Module.SSID = "";
      for(i=SSID_LOC;i<(Module.SsidLength + SSID_LOC);i++){
        rune = EEPROM.read(i);
        Module.SSID += rune;
        yield();
      }
    digitalWrite(D2, HIGH);
    delay(100);
    digitalWrite(D2, LOW);
    delay(100);
    digitalWrite(D2, HIGH);
    delay(100);
    digitalWrite(D2, LOW);
    delay(100);
    digitalWrite(D2, HIGH);
    delay(100);   
}
  Serial.println("->");
  Serial.print("Module SSID: ");
  Serial.println(Module.SSID);
  Serial.print("Module SSID Length: ");
  Serial.println(Module.SsidLength);

  Module.Type = EEPROM.read(TYPE_LOC);
  if(Module.Type>3){
    Serial.println("First load... Writing default module type");
    Module.Type = TYPE_ANO;
    EEPROM.write(TYPE_LOC, Module.Type);
    yield();
  }
  Module.DetectLevel = EEPROM.read(DET_LOC);
  Module.CatchLevel = EEPROM.read(CATCH_LOC);
  Serial.print("Module type: ");
  Serial.println(Module.Type);
  Serial.print("Module Detection Level: ");
  yield();
  Serial.println(Module.DetectLevel);
  Serial.print("Module Catch Level: ");
  Serial.println(Module.CatchLevel);
  Module.Password = "12345678";
  yield();
  WiFi.mode(WIFI_AP_STA);
  WiFi.macAddress(Module.MAC);
  Serial.print("Module MAC: ");
  Serial.println(macToString(getMAC()));

  if(Module.Type==TYPE_DET){  
    Module.MAC[0] = 0x00;
    Module.MAC[1] = 0x00;
    Module.MAC[2] = 0xAB;
    Module.MAC[3] = 0xCD;
    Module.MAC[4] = 0xEF;
    Module.MAC[5] = EEPROM.read(MAC_LOC);
    changeMac(Module.MAC);
    Serial.print("New module MAC: ");
    Serial.println(WiFi.macAddress());
    setDetectorMAC(Module.MAC);
  }
  if(Module.Type == TYPE_CATCHED_ART){
    digitalWrite(D1, LOW);
  }
  EEPROM.end();
}
uint8_t newMACAddress[6] = {0x30, 0xAE, 0xA4, 0x07, 0x0D, 0x66};

void setup() {
  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);
  pinMode(D5, OUTPUT);
  digitalWrite(D0, HIGH);
  digitalWrite(D1, HIGH);
  digitalWrite(D2, HIGH);
  digitalWrite(D3, HIGH);
  digitalWrite(D4, HIGH);
  digitalWrite(D5, HIGH);  
  //digitalWrite(LED_BUILTIN, HIGH);
  Serial.begin(115200);
  Serial.println();
  Serial.println("Configuring access point...");
  checkSettings();
  setCheckedType(Module.Type);
  WiFi.softAP(Module.SSID, Module.Password, MY_CHANNEL, 0, 1);
  if(Module.Type==TYPE_DET){ 
    changeMac(Module.MAC);
  }
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP); 
  serverSet();
  //changeMac(mac);
}


void saveSSID(void){
  uint8_t i;
  EEPROM.begin(EEPROM_LENGTH);
  Module.SsidLength = Module.SSID.length();
  EEPROM.write(SSID_LENGTH_LOC, Module.SsidLength);
  for(i=0;i<Module.SsidLength;i++){
    EEPROM.write(i + SSID_LOC, Module.SSID[i]);
    yield();
  }
  EEPROM.write(DET_LOC, Module.DetectLevel);
  EEPROM.write(CATCH_LOC, Module.CatchLevel);
  EEPROM.write(MAC_LOC, Module.MAC[5]);
  EEPROM.end();
}

void loop() {
  unsigned long currentMillis;
  static unsigned long oldTick = 0;
  handleServer();
  currentMillis = millis();
  if(WiFi.softAPgetStationNum()==0){
    switch(Module.Type){
      case TYPE_ANO:

      break;
      case TYPE_ART:
        if((currentMillis - oldTick) > 500){
          oldTick = currentMillis;
          artAction();
        }
      break;
      case TYPE_DET:
          detectorAction();
      break;
      case TYPE_CATCHED_ART:
          digitalWrite(D1, LOW);  
      break;
    }
}
yield();
  if(flag==1) {
    saveSSID();
    Serial.print("new ssid: ");
    Serial.println(Module.SSID);
    //WiFi.softAP(Module.SSID, Module.Password);
    flag = 0;
  }

}
