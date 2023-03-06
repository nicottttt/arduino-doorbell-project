#include <Wire.h>

void setup() {
  // put your setup code here, to run once:
  Wire.begin(D1,D2);
  Serial.begin(9600);
}


void loop() {
  Wire.requestFrom(12, 1);
  // put your main code here, to run repeatedly:
  Serial.println("ok");
  Wire.beginTransmission(12); 
  Wire.write("Hello Arduino"); 
  Wire.endTransmission(); 
  delay(1000);
}
