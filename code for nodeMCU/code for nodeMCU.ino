#include <NTPClient.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <dht11.h>

#define BTN D6
volatile int detect=0;
volatile int toggle=0;
int state=0;
int pre_state=1;
const char* payload;
char present_time[20];
boolean connectedFlag=0;
dht11 DHT11;

WiFiClient wifiClient;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP); //NTP地址
PubSubClient mqttClient(wifiClient);
//const char* mqttBroker="test.mosquitto.org";



void changeState(){
  state=state+1;
  if(state==3){
    state=0;
  }
}

void setup() {
  pinMode(BTN, INPUT_PULLUP);
  //the setup is to connect to the wifi
  Serial.begin(9600);
  Serial.println();
  WiFi.begin("唐楚昊的iPhone", "12345678");
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

  //ntp:
  timeClient.begin();
  timeClient.setTimeOffset(3600); //British time, in France must +1(+3600)

}

void connec_broker(){
  IPAddress mqttBroker(172,20,10,6);
  mqttClient.setServer(mqttBroker, 1884);  // or the port in which the broker is listening
  while (!mqttClient.connected()) {
    if (mqttClient.connect("esp8266-mqtt-client")) {  // esp8266-mqtt-client is your mqtt client id, feel free to change it
      Serial.println("Connected to MQTT broker.\n");
      connectedFlag=1;
    } else {
      Serial.println("Unable to connect to MQTT broker, retrying..."); 	// you can display the error code with mqttClient.state()
      delay(3000);		// retry after 3 sec
    }
  }
  //initial the time
  timeClient.update();
  timeClient.getFormattedTime().toCharArray(present_time,20);//get the time

  mqttClient.subscribe("nico/topic");
}

void loop() {

  mqttClient.loop();
  //ntp test:
  
  // timeClient.update();
  // timeClient.getFormattedTime().toCharArray(present_time,20);//get the time
  // Serial.print("aa:");
  // Serial.print(present_time);
  // Serial.print("\n");
  // delay(5000);
  
  if(connectedFlag==0){
    connec_broker();
  }

  if(!digitalRead(BTN)){//read the btn
    while(1){
      if(digitalRead(BTN)){break;}
    }
    timeClient.update();
    timeClient.getFormattedTime().toCharArray(present_time,20);//get the time
    changeState();
    Serial.print("change state\n");
  }

  
  if(pre_state!=state){
    switch(state){
      case 0: 
        payload="Present";
        break;
      case 1: 
        payload="absent";
        break;
      case 2: 
        payload="busy";
        break;
    }
    mqttClient.publish("nico/topic", payload);//the last para is const char* !!!
    mqttClient.publish("nico/topic", present_time);
    
  }

  pre_state=state;
}