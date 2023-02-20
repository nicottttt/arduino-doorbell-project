#include <NTPClient.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#define BTN D6
volatile int detect=0;
volatile int toggle=0;
int state=0;
int pre_state=1;
const char* payload;
char present_time[20];
String payload_from_topic;

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

void connec_broker(){
  IPAddress mqttBroker(172,20,10,6);
  mqttClient.setServer(mqttBroker, 1884);  // or the port in which the broker is listening
  while (!mqttClient.connected()) {
    if (mqttClient.connect("esp8266-mqtt-client")) {  // esp8266-mqtt-client is your mqtt client id, feel free to change it
      Serial.println("Connected to MQTT broker.\n");     
    } else {
      Serial.println("Unable to connect to MQTT broker, retrying..."); 	// you can display the error code with mqttClient.state()
      delay(3000);		// retry after 3 sec
    }
  }
  
}

//回调函数
void callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message arrived in topic: ");//打印
    Serial.println(topic);//主题
    Serial.print("Message:");
    for (int i = 0; i < length; i++) {
      payload_from_topic+=(char) payload[i];//内容转成字符串     byte >> char  >> string
    }
    Serial.println(payload_from_topic);
    Serial.println();
    Serial.println("-----------END------------");
    payload_from_topic="";
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

  //initial the time
  timeClient.begin();
  timeClient.setTimeOffset(3600); //British time, in France must +1(+3600)
  timeClient.update();
  timeClient.getFormattedTime().toCharArray(present_time,20);//get the time

  //connect to the broker and subscribe to the topic
  connec_broker();
  mqttClient.setCallback(callback);
  mqttClient.subscribe("nico/student");//the student can post the msg through this topic
  mqttClient.subscribe("nico/professor");//the professor can post the msg through this topic
}

void loop() {
  mqttClient.loop();

  if(!digitalRead(BTN)){//read the btn
    while(1){
      if(digitalRead(BTN)){break;}
    }
    timeClient.update();
    timeClient.getFormattedTime().toCharArray(present_time,20);//get the time
    changeState();
    Serial.print("change state\n");
  }

  if(pre_state!=state){//if some of the status changes, the nodeMCU can publish the msg to the topic 
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
    mqttClient.publish("nico/public", payload);//the last para is const char* !!!
    mqttClient.publish("nico/public", present_time);
  }

  pre_state=state;
}