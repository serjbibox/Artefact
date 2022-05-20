#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>






int getDetectLevel(void);
int getCatchLevel(void);
void saveSSID(void);
String getSSID(void);
uint8_t getType(void);
uint8_t* getMAC(void);
bool changeMac(const uint8_t mac[6]);
void initFormAction(String arg);
void formAction(String argName, String arg);


