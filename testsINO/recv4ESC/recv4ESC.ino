/*
TODO:
  DONE 1.fix terrific splitDat();
  DONE 2.fix even more terrific getDat();
  3.fix yaw interpolation 'n shit...
  LOST CAUSE 4.find joy in life;
*/

//needed for voltage measurment
#include <driver/adc.h>

//needed for motors
#include <ESP32Servo.h>
#include <ESC.h>

//needed for lora comm
#include <SPI.h>
#include <LoRa.h>

//needed for axis sensor
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>



//BNO055 setup
uint16_t BNO055_SAMPLERATE_DELAY_MS = 50;
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x29, &Wire);

#define ss 16
#define rst 4
#define dio0 2

//motor setup
#define MIN_SPEED 1130
#define MIN_MVSPEED 1160
#define MAX_SPEED 1400 //more than enough to lift with 3s

#define FRpin 25
#define BRpin 26
#define FLpin 32
#define BLpin 33


//PID vars
double XKp=0, XKi=0, XKd=0;                 //proportional, integral and derrivative gain for yaw
double Kp=0.25, Ki=0.00, Kd=0.05;             //proportional, integral and derrivative gain for pitch and roll
double targetX=0, targetY=0, targetZ=0,         //target values for PID
       integralY=1, integralX=1, integralZ=1, //integral values for PID
       prevZ=1, prevY=1, prevX=1;             //previous values, to determine delta 

double now=1, dt=1, last_time=1;      //current time, delta time, time *last time* - utilised by integral part of PID
float PDD = 5000;                        //Modifier of total effectiveness of PID.

float Vcc, vref;

//multicore tasks
TaskHandle_t rotors;
TaskHandle_t data;

bool mayday=0;
String FullData [25];
String msgToSend="";bool send=0;
String datata="", datata2;
int pp=0, n=0, sigStr;
int packetSize

//gyro sensor
sensors_event_t orinet;
double x, y, z;

//initialising motors, corrections to keep balance.
ESC FRP (FRpin, 1000, 2000, 500); 
ESC FLP (FLpin, 1000, 2000, 500); 
ESC BRP (BRpin, 1000, 2000, 500); 
ESC BLP (BLpin, 1000, 2000, 500); 

float CFR=1, CFL=1, CBR=1, CBL=1;     //correction value for each motor
float dpitch = 0, dyaw = 0, droll=0;  //desired (target) angles

ESC arr[]= {FRP, FLP, BRP, BLP};
float corr[] = {CFR, CFL, CBR, CBL};
int mot[]={0,0,0,0};

unsigned long tMsg, tMsgLast;


void splitDat(String msg, char delim){  //returns specified part of message, that was separated by delimitors :P
  int counter=0;
  for(int i=0; i<25; i++){FullData[i]="";}
  for(int i=0; i<msg.length(); i++){
    if(msg[i]!=delim){
      FullData[counter]+=msg[i];
    }
    else{
      counter++;
    }
  }
  FullData[24]=String(counter);
}

String genSum(String s) //generates checksum from given string? 
{
  int c=0;
  for(char i:s){
    c+=i;
  }
  return String(c);
}

void setup() {
  Serial.begin(115200);
  tMsg=millis();
  tMsgLast=millis();
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_0,ADC_ATTEN_DB_11);
  LoRa.setPins(ss, rst, dio0);
  LoRa.setTxPower (20);

  //Sets motors up
  pinMode(FRpin, OUTPUT);
  pinMode(FLpin, OUTPUT);
  pinMode(BRpin, OUTPUT);
  pinMode(BLpin, OUTPUT);

  for(int i=0; i<4; i++){arr[i].arm();}
  delay(5000);
  for(int i=0; i<4; i++){arr[i].speed(MIN_SPEED);}

  //sets LoRa up
  while (!LoRa.begin(433E6)) {
    Serial.println(".LoRa");
    delay(500);
  }
  LoRa.setSyncWord(0xF3);

  //sets bno055 up
  while(!bno.begin()){
    Serial.print(".BNO055");
    delay(500);
  }
  bno.getEvent(&orinet, Adafruit_BNO055::VECTOR_EULER);
  x = orinet.orientation.x;
  targetX=x; //sets target yaw to current yaw;

  //begins tasks on multiple cores :P
  xTaskCreatePinnedToCore(rotors_code, "rotors", 15000, NULL, 1, &rotors, 0);
  xTaskCreatePinnedToCore(data_code, "data", 15000, NULL, 1, &data, 1);
  
}

void sendMSG(String msg){       //sends message, automatically appends checksum to the end of it.
  String sum=(genSum(msg));
  msg=msg+";"+sum;
  LoRa.beginPacket();
  LoRa.print(msg);
  LoRa.endPacket();
}

void loop() {
  //wow! such empty? <3
}

String getDat(String d){        //verifies checksum, returns all data without checksum if correct or "-1" if incorrect
  int ct=0;String dd="";
  for(int i=0; i<d.length(); i++){
    if(d[i]==';'){ct++;}
  }
  splitDat(d, ';');
  String sm=FullData[FullData[24].toInt()];
  for(int i=0; i<d.length(); i++){
    if(d[i]==';'){ct--;}
    if(ct!=0){dd+=d[i];}
  }
  if(sm==genSum(dd)){return dd;}
  return "-1";
}

void data_code( void * pvParameters ){
  int watchDogKiller=0;         //self explainatory
  while(1){
    
    //checks for received messages
    packetSize = LoRa.parsePacket();
    if (packetSize) {
      tMsgLast=tMsg;
      tMsg=millis();
      //reads data
      while (LoRa.available()) {
        datata2 = LoRa.readString();
        Serial.println(datata2); 
        sigStr=LoRa.packetRssi();
      }
      Serial.println(sigStr);
      String returnMsg="";
      
      if(getDat(datata2)!="-1"){    //tests checksum
        vref = adc1_get_raw(ADC1_CHANNEL_0);
        Vcc=vref*(3.3651/4096)*11;

        if(FullData[0]=="MAYDAY MAYDAY MAYDAY"){ //if mayday -> well f*ck :P
          mayday=1;
          returnMsg+="RIP";
        }
        else if(FullData[0]=="%D"){
          
          returnMsg+="6969;";//message id
          returnMsg+=String(tMsg-tMsgLast);
          returnMsg+=";";
          returnMsg+=String(sigStr);
          returnMsg+="1;";
          returnMsg+=String(z);
          returnMsg+=";";
          returnMsg+=String(y);
          returnMsg+=";";
          for(int i=0; i<4; i++){
            returnMsg+=String(mot[i]);
            returnMsg+=";";}
          returnMsg+="1234;1234;";//long, lat
          returnMsg+="15";//humidity
        }
        else if(FullData[0][0]=='$'){        //if '$' -> modifies PID (YAW / PITCH&ROLL)
          double tKp=double(FullData[1].toInt())/1000.000;
          double tKi=double(FullData[2].toInt())/1000.000;
          double tKd=double(FullData[3].toInt())/1000.000;
          if(FullData[0]=="$YAW"){
            XKp=tKp; XKi=tKi;  XKd=tKd;
          }
          else if(FullData[0]=="$PR"){
            Kp=tKp; Ki=tKi;  Kd=tKd;
          }

          returnMsg+="Updated variables, turned the motors off.";
        }
        else if(FullData[0]=="&"){        //if '&' -> modifies YAW/PITCH/ROLL angles, speed
          dyaw=FullData[4].toFloat();
          droll=FullData[3].toFloat();
          dpitch=FullData[2].toFloat();
          datata=FullData[1];

          returnMsg+="Updated course, speed.";
        }
        else if(FullData[0]=="^"){        //if '^' -> sets hover speed, resets ROLL/PITCH
          droll=0;
          dpitch=0;
          datata=FullData[1];
          returnMsg+="Set hover speed";
        }
      }
      else{
        returnMsg+="Checksum badd..";
      }
      sendMSG(returnMsg);
    }
    
    delayMicroseconds(1); //time to reset watchdogg
  }
}

double PIDX(double err){
  double prop=err;
  integralX +=err*dt;
  double derivative = (err-prevX)/dt;
  prevX=err;
  double out=(XKp*prop)+(XKi*integralX)+(XKd*derivative);
  if(out>150){out=150;}
  if(out<-150){out=-150;}
  return out;
}

double PIDY(double err){
  double prop=err;
  integralY +=err*dt;
  double derivative = (err-prevY)/dt;
  prevY=err;
  double out=(Kp*prop)+(Ki*integralY)+(Kd*derivative);
  return out;
}

double PIDZ(double err){
  double prop=err;
  integralZ +=err*dt;
  double derivative = (err-prevZ)/dt;
  prevZ=err;
  double out=(Kp*prop)+(Ki*integralZ)+(Kd*derivative);
  return out;
}

void interpolX(double &actual, double &target){ //interpolates yaw value cuz it jumps 360<->0 (degrees)
  int diff=actual-target;
  target=180;
  actual=180+diff;
  if(actual>360){actual-=360;}
}

void rotors_code( void * pvParameters ){
  int watchDogKiller=0;   //self explainatory
  while(1){
    if(mayday){   //oopsy daisy
      for(int i=0; i<4; i++){arr[i].speed(0);}
      while(1);
    }

    //retrieving gyro data
    bno.getEvent(&orinet, Adafruit_BNO055::VECTOR_EULER);
    x = orinet.orientation.x; //getting yaw value
    y = orinet.orientation.y; //getting roll value
    z = orinet.orientation.z; //getting pitch value

    //pid time track
    now=millis();
    dt=(now-last_time)/1000;
    last_time=now;

    //PID calculations for yaw
    double actualX = x;
    if(n<MIN_SPEED){
      targetX=x; //if not in flight, updates yaw target
    }
    interpolX(actualX, targetX);
    double errorX = targetX-actualX;
    
    //PID calculation for roll and pitch
    double actualY = y;
    double errorY = targetY-actualY;

    double actualZ = z;
    double errorZ = targetZ-actualZ;
    

    //does the PID things
    targetY=droll;
    targetZ=dpitch;
    if(n>MIN_SPEED){
      double piidX=PIDX(errorX);
      double piidY=PIDY(errorY);
      double piidZ=PIDZ(errorZ);

      for(int i=0; i<4; i++){
        corr[i]=1;
      }

      if(piidX>0){
        corr[0]=1+abs(piidX)/PDD; corr[3]=1+abs(piidX)/PDD; corr[1]=1-abs(piidX)/PDD; corr[2]=1-abs(piidX)/PDD; 
      }
      else if(piidX<0){
        corr[0]=1-abs(piidX)/PDD; corr[3]=1-abs(piidX)/PDD; corr[1]=1+abs(piidX)/PDD; corr[2]=1+abs(piidX)/PDD; 
      }

      corr[0]-=piidY/PDD; corr[2]-=piidY/PDD; corr[1]+=piidY/PDD; corr[3]+=piidY/PDD;

      corr[0]+=piidZ/PDD; corr[1]+=piidZ/PDD; corr[2]-=piidZ/PDD; corr[3]-=piidZ/PDD;



    }
    for(int i=0; i<4; i++){
      corr[i]/=1;
      if(corr[i]<0){corr[i]=0;}   //makes sure that corr coefficients are positive
      mot[i]=n*corr[i];
    }

    //!updates individual motor speeds ONLY if they are supposed to be moving!!!!!!!!
    if(n>MIN_SPEED){
    for(int i=0; i<4; i++){
      if(n*corr[i]<MAX_SPEED){
        if(0<n*corr[i]){
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

    //updates motor speed, if new data was received
    if (datata.length() > 0) {
      n = datata.toInt();
      datata="";
      for(int i=0; i<4; i++){
          if(n<=MIN_SPEED){
            arr[i].speed(n);
          }
          else if(n*corr[i]<MAX_SPEED){
            if(0<n*corr[i]){
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
    delayMicroseconds(1); //time to reset watchdogg
  }
}
