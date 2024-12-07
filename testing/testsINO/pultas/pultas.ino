#include <SPI.h>
#include <LoRa.h>

//RA01
#define ss 16
#define rst 4
#define dio0 2
int n;
String readString="";
unsigned long timeReceived, timeSent;
String LoRaData;

String splitDat(String msg, char delim, int theOne){  //returns specified part of message, that was separated by delimitors :P
  String ret=""; 
  int counter=0;
  String list[25];
  for(int i=0; i<25; i++){list[i]="";}
  for(int i=0; i<msg.length(); i++){
    if(msg[i]!=delim){
      list[counter]+=msg[i];
    }
    else{
      counter++;
    }
  }
  return list[theOne];
}

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
  LoRa.setTxPower (20);
  while (!LoRa.begin(433E6)) {
    Serial.println("MoDuLiS nEpAsIjUnGe");
    delay(500);
  }

  LoRa.setSyncWord(0xF3);
  Serial.println("legogo");
}

void loop() {
  LoRaData="";
  while (Serial.available()) {
    char c = Serial.read(); 
    readString += c;
  }

  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    while (LoRa.available()) {
      LoRaData = LoRa.readString();
       
    }
    timeReceived=millis();
    Serial.println(LoRaData+"  back/forth:"+String(timeReceived-timeSent)+"ms. ");
  }

  if (readString.length() > 0) {
    readString.trim();
    String sum = genSum(readString);
    readString+=";"+sum;
    LoRa.beginPacket();
    LoRa.print(readString);
    LoRa.endPacket();
    readString="";
    timeSent=millis();
  }
}