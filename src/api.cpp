#include <api.h>
#include <artefact.h>
#include <main.h>
#include <common.h>

ESP8266WebServer server(80);
WiFiEventHandler detectorCatchHandler;
String typeArtString = "";
String typeDetectorString = "";
String typeAnoString = "";

enum {
  TYPE_ART,
  TYPE_DET,
  TYPE_ANO,
  TYPE_CATCHED_ART
};

void serverSet(void){

    server.on("/", handleRoot);
    server.onNotFound(handle_not_found);
    server.on("/initform/", handleInitForm);
    server.on("/artform/", handleArtForm);
    server.begin();
    Serial.println("HTTP server started");
}

void handleInitForm(void) {
if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
  } else {
    for (uint8_t i = 0; i < server.args(); i++) {
        if(server.argName(i) == "inflType") {
            initFormAction(server.arg(i));
        }     
      }
      initResponse();
    }
}

void handleArtForm(void) {
if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
  } else {
    for (uint8_t i = 0; i < server.args(); i++) {
        formAction(server.argName(i), server.arg(i));
      }
     handleRoot();
    }
}

void initResponse(void) {
    int n = 0;
    String catchString = "";
    String detString = "";
    String macString = ""; 
    char catchStringBuf[500];
    char detStringBuf[500];
    char macStringBuf[500];
    switch(getType()){
        case TYPE_ANO:
            sprintf(detStringBuf, "");
            sprintf(catchStringBuf, "");
            n = sprintf(macStringBuf, "");
        break;
        case TYPE_ART:
            n = sprintf(detStringBuf, "SSID артефакта начинается с \"artN\", где N - номер вида: 01, 02...(art01A20R5)\
            <p>Уровень видимости (10...95): \
            <input type=\"number\" id=\"detectLevel\" name=\"ARTDET\" min=10 max=95 value=\"%i\"><br>", getDetectLevel());
            Serial.print("N detStringBuf: ");
            Serial.println(getDetectLevel());
            n = sprintf(catchStringBuf, "<p>Уровень активации (10...95): \
            <input type=\"number\" id=\"catch\" name=\"ARTCATCH\" min=10 max=95 value=\"%i\"><br>", getCatchLevel());
            Serial.print("N catchStringBuf: ");
            Serial.println(getCatchLevel());
            n = sprintf(macStringBuf, "");
            yield();
            //sprintf(macStringBuf, "");
        break;
        case TYPE_DET:
            sprintf(detStringBuf, "");
            sprintf(catchStringBuf, "");
            sprintf(macStringBuf, "<p>AP MAC: XX:XX:XX:XX:XX:XX:<br>\
            <p>Последняя цифра MAC - номер детектора<br>\
            <input type=\"text\" name=\"APMAC\" value=\"%02x:%02x:%02x:%02x:%02x:%02x\"><br>\
            ",
             getMAC()[0], getMAC()[1], getMAC()[2], getMAC()[3], getMAC()[4], getMAC()[5]);      
            Serial.print("api MAC: "); 
            Serial.println(macToString(getMAC()));
        break;
    }
  String artPostForm = "<html>\
  <head>\
  <meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\" />\
   <!DOCTYPE html PUBLIC \"-//WAPFORUM//DTD XHTML Mobile 1.0//EN\" \"http://www.wapforum.org/DTD/xhtml-mobile10.dtd\">\
   <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\
    <title>Настройка ESP модуля: " + getSSID() + "</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Настройка ESP модуля: " + getSSID() + "</h1><br>\
    <form method=\"post\" enctype=\"application/x-www-form-urlencoded\" action=\"/artform/\">\
      <p>AP SSID:<br>\
      <input type=\"text\" name=\"APSSID\" value=\"" + getSSID() + "\"><br>" +
      "<br>" + 
      macStringBuf + 
      "<br>" + 
      detStringBuf +
      "<br>" + 
      catchStringBuf +
      "<br>" + 
      "<input type=\"submit\" value=\"Применить\">\
    </form>\
  </body>\
</html>";
  yield();
  Serial.println("yield()");
  server.send(200, "text/html", artPostForm);
}


void handleRoot(void) {
  String rootPostForm = "<html>\
  <head>\
  <meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\" />\
   <!DOCTYPE html PUBLIC \"-//WAPFORUM//DTD XHTML Mobile 1.0//EN\" \"http://www.wapforum.org/DTD/xhtml-mobile10.dtd\">\
   <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\
    <title>Настройка ESP модуля: </title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Настройка ESP модуля: " + getSSID() + "</h1><br>\
    <form method=\"post\" enctype=\"application/x-www-form-urlencoded\" action=\"/initform/\">\
      <input type=\"radio\" id=\"isArt\" name=\"inflType\" value=\"isArt\" " + typeArtString + ">\
      <label for=\"isArt\">Артефакт</label><br>\
      <input type=\"radio\" id=\"isAno\" name=\"inflType\" value=\"isAno\"" + typeAnoString + ">\
      <label for=\"isAno\">Аномалия</label><br>\
      <input type=\"radio\" id=\"isDetector\" name=\"inflType\" value=\"isDetector\"" + typeDetectorString + ">\
      <label for=\"isDetector\">Детектор</label><br>\
      <input type=\"radio\" id=\"reset\" name=\"inflType\" value=\"reset\">\
      <label for=\"reset\">Перезапустить</label><br>\
      <br>\
      <input type=\"submit\" value=\"Применить\">\
    </form>\
  </body>\
</html>";
  server.send(200, "text/html", rootPostForm);
}


void setCheckedType(uint8_t type){
  switch(type){
    case TYPE_ANO:
      typeArtString = "";
      typeAnoString = "checked";
      typeDetectorString = "";
      detectorCatchHandler = NULL;
    break;
    case TYPE_ART:
      typeArtString = "checked";
      typeAnoString = "";
      typeDetectorString = "";    
      detectorCatchHandler = WiFi.onSoftAPModeProbeRequestReceived(&detectorCatch);
    break;
    case TYPE_CATCHED_ART:
      typeArtString = "checked";
      typeAnoString = "";
      typeDetectorString = "";    
      detectorCatchHandler = NULL;
      digitalWrite(D1, HIGH);
    break;    
    case TYPE_DET:
      typeArtString = "";
      typeAnoString = "";
      typeDetectorString = "checked"; 
      detectorCatchHandler = NULL;   
    break;
  }    

}

void handleServer(void){
    server.handleClient();
}

void handle_not_found(void) {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += server.method() == HTTP_GET ? "GET" : "POST";
  if (server.args() > 0) {
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i = 0; i < server.args(); i++){
      message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
  }
  server.send(404, "text/plain", message);
}