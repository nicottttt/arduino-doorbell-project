#include <Wire.h>
char state=0;
String payload;
void setup() {
  // put your setup code here, to run once:
  Wire.begin(12);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);
  Serial.begin(9600);
}

void requestEvent(){
  Wire.write(0);
}

void receiveEvent() {
  
  payload=""; 
  while (0 <Wire.available()) {
      char c = Wire.read();  
      payload+=c;
      //Serial.print(c);          
    }
  Serial.println(payload); 
}

void loop() {
  // put your main code here, to run repeatedly:
  
}
