#include <common.h>
#include <detector.h>

typedef enum {
    VERYSLOW = 1000,
    SLOW = 750,
    MIDDLE = 500,
    FAST = 250,       
    VERYFAST = 100
} led_pwm_t;

const int LAST_SCAN = 4000;
const int SCAN_PERIOD = 1000;

uint8_t detectorMAC[6] = {0,0,0,0,0,0};

void setDetectorMAC(uint8_t mac[6]){
    for (int i = 0; i<6; i++){
        detectorMAC[i] = mac[i];
    }
    
}

void detectorAction(void){
unsigned long currentMillis;
    static unsigned long ledTick = 0;
    static unsigned long scanFaultTick = 0;
    static unsigned long scanTick = 0;
    static unsigned long scanTime = 0;
    static int n = 10000;
    static bool scanFlag = false;
    static uint32_t strength = 10000;
    static uint16_t ledClock = VERYSLOW;
    char buf[10];
    currentMillis = millis();
    if((currentMillis - scanFaultTick) > LAST_SCAN){
        if(scanFlag){
            scanFlag = false;
            strength = 10000;
            Serial.println("scan restart...");
        }

    }
    if((currentMillis - scanTick) > SCAN_PERIOD){   
        scanTick = currentMillis;
        if(!scanFlag){
            Serial.println("scan start...");
            changeMac(detectorMAC);
            WiFi.scanNetworks(true, false, MY_CHANNEL, NULL);
            scanTime = currentMillis;
            scanFlag = true;
            n = 0;
            scanFaultTick = currentMillis;
        }
    }    
    n = WiFi.scanComplete();

    if (n > 0) {
        scanTime = currentMillis-scanTime;
        int32_t maxRssi = -1000;
        int maxRssiIdx = 1000;
        Serial.print("scan done ");
        Serial.print(n);
        Serial.println(" networks found");
        for (int i = 0; i < n; ++i) {
            strength = 10000;
            WiFi.SSID(i).toCharArray(buf, 4);
            if(String(buf) == "art"){
                if(WiFi.RSSI(i)>maxRssi) {
                    maxRssi = WiFi.RSSI(i);
                    maxRssiIdx = i;
                }
                Serial.print(i + 1);
                Serial.print(": ");
                Serial.print(WiFi.SSID(i));
                Serial.print(" (");
                Serial.print(0 - WiFi.RSSI(i));
                Serial.print(")");
                Serial.print(" (");
                Serial.print(macToString(WiFi.BSSID(i)));
                Serial.print(") ");     
                Serial.print("channel: ");    
                Serial.print(WiFi.channel(i)); 
                Serial.print(", scan time: ");    
                Serial.print(scanTime); 
                Serial.println("");  
                yield(); 
            }
        }
        if(maxRssiIdx != 1000){
            if(WiFi.RSSI(maxRssiIdx) > -35) ledClock = VERYFAST;
            else if(WiFi.RSSI(maxRssiIdx) > -50) ledClock = FAST;  
            else if(WiFi.RSSI(maxRssiIdx) > -60) ledClock = MIDDLE; 
            else if(WiFi.RSSI(maxRssiIdx) > -65) ledClock = SLOW;
            else if(WiFi.RSSI(maxRssiIdx) > -75) ledClock = VERYSLOW;
            else ledClock = VERYSLOW;
            strength = (WiFi.RSSI(maxRssiIdx)*WiFi.RSSI(maxRssiIdx))*2/10;
            if(strength<100) strength = 100;
            Serial.printf("Select SSID: %s, PWM: %i, %i\n", WiFi.SSID(maxRssiIdx), ledClock, strength);       
            yield();          
        }
        scanFlag = false;
        WiFi.scanDelete();
    }

    yield();
    if(strength < 10000){   
        if((currentMillis - ledTick) > strength){
            ledTick = currentMillis;
            if((strength>1280) & (strength<1805)){
                digitalWrite(D2, LOW);
                digitalWrite(D5, LOW);
                delay(50);  
                digitalWrite(D2, HIGH); 
                digitalWrite(D5, HIGH);

            } else if(strength>980){
                digitalWrite(D1, LOW);
                digitalWrite(D2, LOW);
                digitalWrite(D3, LOW);
                digitalWrite(D5, LOW);
                delay(50);  
                digitalWrite(D1, HIGH);  
                digitalWrite(D2, HIGH);
                digitalWrite(D3, HIGH);   
                digitalWrite(D5, HIGH);               
            }else{
                digitalWrite(D0, LOW);
                digitalWrite(D1, LOW);
                digitalWrite(D2, LOW);
                digitalWrite(D3, LOW);
                digitalWrite(D4, LOW);
                digitalWrite(D5, LOW);
                delay(50);  
                digitalWrite(D0, HIGH);  
                digitalWrite(D1, HIGH);
                digitalWrite(D2, HIGH); 
                digitalWrite(D3, HIGH);
                digitalWrite(D4, HIGH); 
                digitalWrite(D5, HIGH);              
            }

        }
    }
}