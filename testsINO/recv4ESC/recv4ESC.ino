//needed for 
#include <ESP32Servo.h>
#include <ESC.h>

//needed for lora comm
#include <SPI.h>
#include <LoRa.h>

//needed for axis sensor
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

//might be? needed to disable watchdog cuz idc 'bout it
#include <rtc_wdt.h>;

//BNO055 setup
uint16_t BNO055_SAMPLERATE_DELAY_MS = 50;
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x29, &Wire);

#define ss 16
#define rst 4
#define dio0 2

#define MIN_SPEED 1130
#define MIN_MVSPEED 1160
#define MAX_SPEED 1200

#define FRpin 25
#define BRpin 26
#define FLpin 32
#define BLpin 33


//PID vars

double Kp=1, Ki=0.2, Kd=0.8, now=1, dt=1, last_time=1, integral=1, prev=1; //*,*,*, dabartinis laikas, delta-laikas, laikas praeitam loope, ., . - abu taskus reiks part of array padaryt...
double target;

//multicore tasks
TaskHandle_t rotors;
TaskHandle_t data;


String datata="", datata2;
int pp=0, n=0, sigStr;


sensors_event_t orinet;
double x, y, z;


ESC FRP (FRpin, 1000, 2000, 500); 
ESC FLP (FLpin, 1000, 2000, 500); 
ESC BRP (BRpin, 1000, 2000, 500); 
ESC BLP (BLpin, 1000, 2000, 500); 

float CFR=1, CFL=1, CBR=1, CBL=1; //kiek individualu motora reik pakoreguot;
float CL=1, CR=1, CB=1, CF=1, corrCW=1, corrCCW=1;  //kiek individualia asi reik koreguot, labai neefektyviai, bet px...
float dpitch = 0, dyaw = 0, droll=0; //desired kampai, jei judet jis tures, kiek turetu pasiversti.... Ateiciai...

ESC arr[]= {FRP, FLP, BRP, BLP};
float corr[] = {CFR, CFL, CBR, CBL};

unsigned long t1, t2;


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

String msgToSend="";bool send=0;

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

  pinMode(FRpin, OUTPUT);
  pinMode(FLpin, OUTPUT);
  pinMode(BRpin, OUTPUT);
  pinMode(BLpin, OUTPUT);

  for(int i=0; i<4; i++){arr[i].arm();}
  delay(5000);
  for(int i=0; i<4; i++){arr[i].speed(MIN_SPEED);}

  while (!LoRa.begin(433E6)) {
    Serial.println(".LoRa");
    delay(500);
  }
  LoRa.setSyncWord(0xF3);

  while(!bno.begin()){
    Serial.print(".BNO055");
    delay(500);
  }
  bno.getEvent(&orinet, Adafruit_BNO055::VECTOR_EULER);
  x = orinet.orientation.x;
  target=x;
  t1=millis();
  //begin tasks on multiple cores :P
  xTaskCreatePinnedToCore(rotors_code, "rotors", 10000, NULL, 1, &rotors, 0);        
  xTaskCreatePinnedToCore(data_code, "data", 3000, NULL, 1, &data, 1);        
}

void sendMSG(String msg){
  String sum=(genSum(msg));
  msg+=";"+sum;
  LoRa.beginPacket();
  LoRa.print(msg);
  LoRa.endPacket();
}

void loop() {
}

void data_code( void * pvParameters ){
  int watchDogKiller=0;
  while(1){

    if(send){
      sendMSG(msgToSend);
      send=0;
    }

    int packetSize = LoRa.parsePacket();
    if (packetSize) {
      //reads data
      while (LoRa.available()) {
        datata2 = LoRa.readString();
        Serial.println(datata2); 
        sigStr=LoRa.packetRssi();
      }

      //tests checksum
      String sum=splitDat(datata2, ';', 1), returnMsg="", dat=splitDat(datata2, ';', 0);
      if(genSum(dat)==sum){
        returnMsg+="Checksum OK, '";
        datata=dat;
      }else{
        returnMsg+="Checksum badd, '";
      }
      returnMsg+=String(dat)+"', signal strength = "+String(sigStr)+"dBm. ";

      sendMSG(returnMsg);
      
    }
    delay(1); //time to reset watchdogg
  }
}


double PID(double err){
  double prop=err;
  integral +=err*dt;
  double derivative = (err-prev)/dt;
  prev=err;
  double out=(Kp*prop)+(Ki*integral)+(Kd*derivative);
  //if(out>0){out-=180;}else{out+=180;}

  //if()
  if(out>150){out=150;}
  if(out<-150){out=-150;}
  return out;
}

void interpolX(double &actual, double &target){ //kadangi x value sokineja 0<->360 tai koreguoja, kad target visada butu = 180 ir current value butu artimiausias kelias ligi 180 :P
  if(360-target+actual<target-actual){
    actual=180-(360-target+actual);
    target =180;
  }
  else{
    actual=180-(target-actual);
    target =180;
  }
}

void rotors_code( void * pvParameters ){
  int watchDogKiller=0;
  while(1){
    int watchDogKiller2=0;

    bno.getEvent(&orinet, Adafruit_BNO055::VECTOR_EULER);
    x = orinet.orientation.x; //jei mazeja, sukas i kaire (nuo 0 persoka prie 360), jei dideja - i desine (360 - soka i 0) bet kazkaip chujovai veikia ngl
    y = orinet.orientation.y; // jei 0> lenkias i kaire, 0< lenkias i desine
    z = orinet.orientation.z; // jei 0> lenkias atgal, 0< lenkias i prieki
    t2=millis();

    //PID cia bus
    now=millis();
    dt=(now-last_time)/1000;
    last_time=now;
    double actual = x;
    interpolX(actual, target);
    double error = target-actual;
    if(abs(target-actual)<1){error=0;}
    if(n>MIN_SPEED){
      double piid=PID(error);
    
      if(piid>0){
        corr[0]=1+abs(piid)/300; corr[3]=1+abs(piid)/300; corr[1]=1-abs(piid)/300; corr[2]=1-abs(piid)/300; 
      }
      else if(piid<0){
        corr[0]=1-abs(piid)/300; corr[3]=1-abs(piid)/300; corr[1]=1+abs(piid)/300; corr[2]=1+abs(piid)/300; 
      }
    }
    //Serial.println(String(PID(error)) + " : "+String(x-180)+" target = "+String(target));
    if(n>MIN_SPEED){
    for(int i=0; i<4; i++){
      if(n*corr[i]<MAX_SPEED){
        if(MIN_MVSPEED<n*corr[i]){
          arr[i].speed(n*corr[i]);
        }
        else{
          arr[i].speed(MIN_MVSPEED);
        }
      }
      else{
        arr[i].speed(MAX_SPEED);
      }
    }
    }

    if (datata.length() > 0) {
      n = datata.toInt();
      datata="";

      if(n>=0 && n<=MAX_SPEED){
        Serial.print("Setting servo speed to ");
        Serial.print(n);
        Serial.println(" .. . . .. ");
        for(int i=0; i<4; i++){
          if(n<=MIN_SPEED){
            arr[i].speed(n);
          }
          else if(n*corr[i]<MAX_SPEED){
            if(MIN_MVSPEED<n*corr[i]){
              arr[i].speed(n*corr[i]);
            }
            else{
              arr[i].speed(MIN_MVSPEED);
            }
          }
          else{
            arr[i].speed(MAX_SPEED);
          }
        }
      }
    }
    delay(1); //time to reset watchdogg
  }
}
