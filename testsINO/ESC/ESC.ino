#include <ESP32Servo.h>

#include <ESC.h>
String readString;
int pp=0, n=0;
#define pin 26

ESC myESC (pin, 1000, 2000, 500); 

#define MIN_SPEED 1030
#define MAX_SPEED 1600

void setup() {
  Serial.begin(115200);
  pinMode(pin, OUTPUT);
  myESC.arm();
  delay(5000);
}

void loop(){

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
      myESC.speed(n); 
    }
  } 

  

}
