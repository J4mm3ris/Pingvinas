//needed for 
#include <ESP32Servo.h>
#include <ESC.h>

//needed for lora comm
#include <SPI.h>
#include <LoRa.h>

//needed to disable watchdog cuz idc 'bout it
#include <rtc_wdt.h>;


//RA01
#define ss 16
#define rst 4
#define dio0 2

//multicore tasks
TaskHandle_t rotors;
TaskHandle_t data;


String datata="", datata2;
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

unsigned long timeReceived, timeUpdated;




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

  //begin tasks on multiple cores :P
  xTaskCreatePinnedToCore(rotors_code, "rotors", 1000, NULL, 1, &rotors, 0);        
  xTaskCreatePinnedToCore(data_code, "data", 1000, NULL, 1, &data, 1);        
}

void loop() {
}

void data_code( void * pvParameters ){
  int watchDogKiller=0;
  while(1){
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
      while (LoRa.available()) {
        datata = LoRa.readString();
        timeReceived=millis();
        delay(1); //kad kitas branduolys spetu updatint datata2
        Serial.println(datata2); 
        sigStr=LoRa.packetRssi();
      }
      LoRa.beginPacket();
      LoRa.print("confirmed: '");
      LoRa.print(datata2);
      LoRa.print("', tX strength = ");
      LoRa.print(sigStr);
      LoRa.print("dBm. ");
      LoRa.endPacket();
      Serial.println("Confirmed received...");
    }
    delay(1); //time to reset watchdog
  }
}


void rotors_code( void * pvParameters ){
  int watchDogKiller=0;
  while(1){
    int watchDogKiller2=0;
    if (datata.length() > 0) {
      datata2=datata;
      n = datata.toInt();
      datata="";

      if(n>=0 && n<=2000){
        Serial.print("Setting servo speed to ");
        Serial.print(n);
        Serial.print(" .. . . .. ");
        for(int i=0; i<4; i++){
          arr[i].speed(n);
        }
      }
      timeUpdated=millis();
      Serial.print("From the moment it was received, it took ");
      Serial.print(timeUpdated-timeReceived);
      Serial.println("ms. ");
    }
    delay(1); //time to reset watchdog
  }
}
