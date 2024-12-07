/*
WARNING

gali random sumastyt max greiciu(ne variable o tiesiog... dideliu greiciu...)
skrist i siena
tai jo...
...

*/
#include <ESP32Servo.h>
#include <ESC.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

String readString;



uint16_t BNO055_SAMPLERATE_DELAY_MS = 50;
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x29, &Wire);


#define MIN_SPEED 1080

#define MIN_MOVE_SPEED 1160

#define MAX_SPEED 1250

int n=MIN_SPEED;

#define FRpin 25
#define BRpin 26
#define FLpin 32
#define BLpin 33

float CFR=1, CFL=1, CBR=1, CBL=1; //kiek individualu motora reik pakoreguot;

float CL=1, CR=1, CB=1, CF=1, corrCW=1, corrCCW=1;  //kiek individualia asi reik koreguot, labai neefektyviai, bet px...

float dpitch = 0, dyaw = 0, droll=0; //desired kampai, jei judet jis tures, kiek turetu pasiversti.... Ateiciai...

ESC FRP (FRpin, 1000, 2000, 500); 
ESC FLP (FLpin, 1000, 2000, 500); 
ESC BRP (BRpin, 1000, 2000, 500); 
ESC BLP (BLpin, 1000, 2000, 500); 

ESC arr[]= {FRP, FLP, BRP, BLP};

float corr[] = {CFR, CFL, CBR, CBL};

int cntRot=0; 
float rot=0,rotAVG=0;


void setup() {
  Serial.begin(115200);
  pinMode(FRpin, OUTPUT);
  pinMode(FLpin, OUTPUT);
  pinMode(BRpin, OUTPUT);
  pinMode(BLpin, OUTPUT);
  for(int i=0; i<4; i++){
    arr[i].arm();
  }
  if (!bno.begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.print("Nusisauk dalabjobs");
    while (1);
  }
  delay(5000);
  for(int i=0; i<4; i++){
    arr[i].speed(MIN_SPEED);
  }
}

void loop() {

  //nuskaito giroskopo duomenis (cj cia gyro, nzn istikro...)
  sensors_event_t orinet;
  bno.getEvent(&orinet, Adafruit_BNO055::VECTOR_EULER);
  double x, y, z;
  x = orinet.orientation.x; //jei mazeja, sukas i kaire (nuo 0 persoka prie 360), jei dideja - i desine (360 - soka i 0) bet kazkaip chujovai veikia ngl
  y = orinet.orientation.y; // jei 0> lenkias i kaire, 0< lenkias i desine
  z = orinet.orientation.z; // jei 0> lenkias atgal, 0< lenkias i prieki

/*

  //paskaiciuoja vidurki pasisukimo, nezinau kaip jis cia veikia ir tikrai reiks keist, labai temp ir nesaugus ir prastas sprendimas kolkas .... .. .... Kuris net neveiks turbut...
  if(cntRot!=10){
    rotAVG+=x;cntRot++;
  }
  else{
    rot=rotAVG/10;cntRot=0;
  }

  

  //pakeicia korekcija propeleriu pagal *giroskopa?*

  if(rot>x+10){
    corrCW-=0.01;corrCCW+=0.01;
  } //CW+, CCW-
  else if(rot<x-10){
    corrCW+=0.01;corrCCW-=0.01;
  }//CW-, CCW+
  else{
    corrCW=1;corrCCW-=1;
  }
*/

  if(y<-1){CL+=0.005;CR-=0.005;}
  else if(y>1){CR+=0.005;CL-=0.005;}
  else{CR=1; CL=1;}

  if(z<-1){CB+=0.005;CF-=0.005;}
  else if(z>1){CF+=0.005;CB-=0.005;}
  else{CF=1; CB=1;}

  //updatina each propeleri pagal koregacijas visas..

  CFR=(CR+CF+corrCCW)/3; 
  CFL=(CL+CF+corrCW)/3;
  CBR=(CR+CB+corrCW)/3;
  CBL=(CL+CB+corrCCW)/3;

  corr[0]=CFR;
  corr[1]=CFL;
  corr[2]=CBR;
  corr[3]=CBL;

  //sureguliuoja kad nepersistengtu...

  for(int i=0; i<4; i++){
    if(corr[i]*n<MIN_MOVE_SPEED){corr[i]=float(MIN_MOVE_SPEED/float(n));} //cia kad visada bent judetu

    //if(corr[i]*n<MIN_SPEED){corr[i]=float(MIN_SPEED/float(n));}//cia kad nejudetu kai mazas greitis...
    if(corr[i]*n>MAX_SPEED){corr[i]=float(MAX_SPEED/float(n));} //cia kad nenuskristu...
  }

  //cia kad zinot jog jis i siena neiskris turbut... bet nepades vistiek jauciu......

  // Serial.print("CFR= ");
  // Serial.print(corr[0]);
  // Serial.print(" CFL= ");
  // Serial.print(corr[1]);
  // Serial.print(" CBR= ");
  // Serial.print(corr[2]);
  // Serial.print(" CBL= ");
  // Serial.println(corr[3]);


  for(int i=0; i<4; i++){
    if(n>MIN_SPEED){
      if(int(n*corr[i])<MAX_SPEED){
          arr[i].speed(int(n*corr[i]));
      }
      else{
        arr[i].speed(int(n));
      }
    }
  }



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
        if(1){
          if(int(n*corr[i])<MAX_SPEED){
              arr[i].speed(int(n*corr[i]));
          }
          else{
            arr[i].speed(int(n));
          }
        }
      }
      delay(100);
    }
  } 

}
