#include <SPI.h>
#include <LoRa.h>

//RA01
#define ss 16
#define rst 4
#define dio0 2
int n;
String readString="";

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
  }

  if (readString.length() > 0) {
    n = readString.toInt();
    LoRa.beginPacket();
    LoRa.print(n);
    LoRa.endPacket();
    readString="";
  }
}