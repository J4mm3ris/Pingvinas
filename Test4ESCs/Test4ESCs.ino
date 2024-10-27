#include <ESP32Servo.h>

#include <ESC.h>
String readString;
int pp=0, n=0;

#define MIN_SPEED 1080
#define MAX_SPEED 1390
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
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read(); 
    readString += c;
    delay(2);
  }

  if (readString.length() >0) {
    n = readString.toInt();
    readString="";

    if(n>=0 && n<=2000){
      Serial.print("Setting servo speed to ");
      Serial.print(n);
      Serial.println(" .. . . .. ");
      for(int i=0; i<4; i++){
        arr[i].speed(n);
      }
      delay(100);
    }
  } 

}
