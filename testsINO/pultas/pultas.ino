#include <SPI.h>
#include <LoRa.h>

//RA01
#define ss 16
#define rst 4
#define dio0 2
int n;
String readString="";
unsigned long timeReceived, timeSent;


String genSum(String s) //generates checksum? kinda... bad... nvm..
{
  int c=0;
  for(char i:s){
    c+=i;
  }
  return String(c);
}

void setup() {
  Serial.begin(115200);

  LoRa.setPins(ss, rst, dio0);

  while (!LoRa.begin(433E6)) {
    Serial.println("MoDuLiS nEpAsIjUnGe");
    delay(500);
  }

  LoRa.setSyncWord(0xF3);
  Serial.println("legogo");
}

void loop() {

  while (Serial.available()) {
    char c = Serial.read(); 
    readString += c;
    delay(2);
  }

  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    while (LoRa.available()) {
      String LoRaData = LoRa.readString();
      Serial.print("Received: ");
      Serial.println(LoRaData); 
    }
    timeReceived=millis();
    Serial.print("This message bounced back and forth in ");
    Serial.print(timeReceived-timeSent);
    Serial.println(" ms.");
    Serial.println("");
  }

  if (readString.length() > 0) {
    readString.trim();
    String sum = genSum(readString);
    Serial.println("ShwasdSUM = "+sum);
    readString+=";"+sum;
    LoRa.beginPacket();
    LoRa.print(readString);
    LoRa.endPacket();
    Serial.println("Message sent");
    readString="";
    timeSent=millis();
  }
}