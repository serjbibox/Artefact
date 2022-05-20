#include <artefact.h>
#include <common.h>

bool isDetector = false;
bool isDetectorCatch = false;

void detectorCatch(const WiFiEventSoftAPModeProbeRequestReceived& evt) {
  unsigned long currentMillis = millis();
  static unsigned long oldTick = 0;
    //Serial.print("Probe request from: ");
    //Serial.println(macToString(evt.mac));
  if(evt.mac[0]==0 && evt.mac[1]==0 && evt.mac[2] == 0xAB && evt.mac[3] == 0xCD && evt.mac[4] == 0xEF && 
    (currentMillis - oldTick)>100){
    oldTick = currentMillis;
    if(evt.rssi > (0-getDetectLevel())){
      isDetector = true;
    }
     if(evt.rssi > (0-getCatchLevel())){
      isDetectorCatch = true;
    }   
    Serial.print("Probe request from: ");
    Serial.print(macToString(evt.mac));
    Serial.print(" RSSI: ");
    Serial.print(evt.rssi);
    Serial.print(", isDetector: ");
    Serial.print(isDetector);
    Serial.print(", isDetectorCatch: ");
    Serial.println(isDetectorCatch);
    
  }

}

void artAction(void){
  static uint8_t cnt = 0;
    if(isDetector){
      cnt = 0;
      isDetector = false;
    }
    if(cnt<20){
      digitalWrite(D3, HIGH);
      cnt++;
    }else{
      digitalWrite(D3, LOW);
    }

    if(isDetectorCatch){
      digitalWrite(D1, HIGH);
      initFormAction(String("isCatchedArt"));
      char cart[6] = {"abcde"};
      String c = getSSID();
      cart[0] = 'c';
      for(int i = 1; i<5;i++){
        cart[i] = c[i];
      }
      Serial.print("Change art SSID: ");
      Serial.println(cart);
      formAction(String("APSSID"), String(cart));
      saveSSID();
      delay(1000); 
      system_restart();
      delay(2000);  
    }
}