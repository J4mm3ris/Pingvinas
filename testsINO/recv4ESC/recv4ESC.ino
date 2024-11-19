#include <ESP32Servo.h>
#include <ESC.h>
#include <SPI.h>
#include <LoRa.h>

//RA01
#define ss 16
#define rst 4
#define dio0 2


String datata;
int pp=0, n=0, sigStr;


#define MIN_SPEED 1080
#define MAX_SPEED 1200

#define FRpin 25
#define BRpin 26
#define FLpin 32
#define BLpin 33


ESC FRP (FRpin, 1000, 2000, 500); 
ESC FLP (FLpin, 1000, 2000, 500); 
ESC BRP (BRpin, 1000, 2000, 500); 
ESC BLP (BLpin, 1000, 2000, 500); 

ESC arr[]= {FRP, FLP, BRP, BLP};

void setup() {
  Serial.begin(115200);
  LoRa.setPins(ss, rst, dio0);

  pinMode(FRpin, OUTPUT);
  pinMode(FLpin, OUTPUT);
  pinMode(BRpin, OUTPUT);
  pinMode(BLpin, OUTPUT);

  for(int i=0; i<4; i++){
    arr[i].arm();
  }
  delay(5000);
  for(int i=0; i<4; i++){
    arr[i].speed(MIN_SPEED);
  }
  while (!LoRa.begin(433E6)) {
    Serial.println(".");
    delay(500);
  }

  LoRa.setSyncWord(0xF3);
}

void loop() {

  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    while (LoRa.available()) {
      datata = LoRa.readString();
      Serial.println(datata); 
      sigStr=LoRa.packetRssi();
    }
  }

  if (datata.length() >0) {

    LoRa.beginPacket();
    LoRa.print("confirmed: '");
    LoRa.print(datata);
    LoRa.print("', tX strength = ");
    LoRa.print(sigStr);
    LoRa.print("dBm. ");
    LoRa.endPacket();
    Serial.println("Patvirtinau gavima...");

    n = datata.toInt();
    datata="";

    if(n>=0 && n<=2000){
      Serial.print("Setting servo speed to ");
      Serial.print(n);
      Serial.println(" .. . . .. ");
      for(int i=0; i<4; i++){
        arr[i].speed(n);
      }
    }
  } 

}
