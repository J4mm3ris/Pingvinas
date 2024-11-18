
int xrot, yrot, FLP, FRP, BRP, BLP;

float lat, lon;
String t="";
void setup() {
  Serial.begin(115200);

}

void loop() {
  t="$$";
  xrot=random()%360-180;
  yrot=random()%360-180;
  FLP=random()%100;
  FRP=random()%100;
  BLP=random()%100;
  BRP=random()%100;
  lat=float(random()%1000000)/float(100000);
  lon=float(random()%1000000)/float(100000);
  t+=String(xrot)+";"+String(yrot)+";"+String(FLP)+";"+String(FRP)+";"+String(FLP)+";"+String(BRP)+";"+String(BLP)+";"+String(lat)+";"+String(lon)+";";
  t+="$$";

  Serial.print(t);
  delay(50);
}
