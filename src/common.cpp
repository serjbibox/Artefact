#include <common.h>

String macToString(const unsigned char* mac) {
  char buf[20];
  snprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(buf);
}



void stringToMac(String mac, uint8_t* buf) {
  uint8_t k, m;
  uint8_t i;
  char d[6][3];
  for(i=0;i<6;i++){
    mac.toCharArray(d[i], 3, i*3);
    if((d[i][0]>='0') & (d[i][0] <= '9')) k = d[i][0] - 0x30;
    else k = toLowerCase(d[i][0]) - 0x57;
    if((d[i][1]>='0') & (d[i][1] <= '9')) m = d[i][1] - 0x30;
    else m = toLowerCase(d[i][1]) - 0x57;
    buf[i] = k*16 + m;
    //Serial.printf("hex to int: d: 0x%c%c, %i, %i, ; k = %i; m = %i\n", d[i][0], d[i][1], d[i][0], d[i][1] ,k, m);
  }
}