#include <ESP32Servo.h>

#include <ESC.h>
String readString;
int pp=0, n=0;
ESC myESC (18, 1000, 2000, 500); 

#define MIN_SPEED 1080 // speed just slow enough to turn motor off
#define MAX_SPEED 1390 // speed where my motor drew 3.6 amps at 12v.

void setup() {
  Serial.begin(115200);
  pinMode(18, OUTPUT);
  myESC.arm();
  delay(5000);
}

void loop() {


  // for (int i=150; i<450; i++){ // run speed from 840 to 1190
  //   myESC.speed(MIN_SPEED-200+i); // motor starts up about half way through loop
  //   delay(10);
  // }
  // delay(3000);
  // myESC.speed(0);
  // myESC.arm();
  // delay(3000);
  // myESC.speed(MIN_SPEED);
  // delay(3000);
  // myESC.speed(MAX_SPEED);
  // delay(5000);
  // myESC.speed(0);
  // myESC.arm();
  // delay(3000);

  // myESC.speed(1170);
  // delay(3000);
  // myESC.speed(1130);
  // delay(3000);

  // for(int i=0; i<200; i++){
  //   myESC.speed(MIN_SPEED+i);
  //   Serial.print("Speed: ");
  //   Serial.println(MIN_SPEED+i);
  //   delay(1000);
    
  // }


  while (Serial.available()) {
    char c = Serial.read();  //gets one byte from serial buffer
    readString += c; //makes the String readString
    delay(2);  //slow looping to allow buffer to fill with next character
  }

  if (readString.length() >0) {
    n = readString.toInt();  //convert readString into a number
    readString="";

    if(n>=0 && n<=2000){
      Serial.print("Setting servo speed to ");
      Serial.print(n);
      Serial.println(" .. . . .. ");
      myESC.speed(n); 
    }
  } 

  

}
