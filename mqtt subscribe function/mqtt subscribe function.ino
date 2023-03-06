#include <NTPClient.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>

#define BTN D6
volatile int detect=0;
volatile int toggle=0;
int state=0;
int pre_state=1;
const char* payload_publish;
const char* wifi_name="唐楚昊的iPhone";
const char* passport="12345678";
char present_time[20];
String payload_from_topic;
String payload_receive;
char if_student;

WiFiClient wifiClient;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP); //NTP地址
PubSubClient mqttClient(wifiClient);

//start the broker: mosquitto -c mosquitto.conf -v -p 1884

//const char* mqttBroker="test.mosquitto.org";
void changeState(){
  state=state+1;
  if(state==3){
    state=0;
  }
}

void connec_broker(){
  //connect to the ifi
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

String transfer_format(String payload){
  String payload_time=payload.substring(0,2)+":"+payload.substring(2,4);
  String payload_status=payload.substring(5);
  payload_status.toUpperCase();
  String payload3=" until ";
  String payload_final=payload_status+payload3+payload_time;
  return payload_final;
}

//call back function(for the subscribe)
void callback(char *topic, byte *payload, unsigned int length) {
  timeClient.update();
  timeClient.getFormattedTime().toCharArray(present_time,20);//get the time
  byte key[] = "nico";//the key for decryptData
  Serial.print("Message arrived in topic: ");//打印
  Serial.println(topic);//主题
  Serial.print("Message:");

  String payload_header;
  
  // to know which topic send the msg
  if(strcmp(topic,"nico/student")==0){
    payload_header="Vister published at ";
    for (int i = 0; i < length; i++) {
      //payload_from_topic+=(char) payload[i];//内容转成字符串     byte >> char  >> string
      payload[i] = payload[i] ^ key[i % 4];//decrypt data here
      payload_from_topic+=(char) payload[i];
    }
  }else if(strcmp(topic,"nico/professor")==0){
    payload_header="Host published at ";
    Wire.beginTransmission(12); 
    for (int i = 0; i < length; i++) {
      //payload_from_topic+=(char) payload[i];//内容转成字符串     byte >> char  >> string
      payload[i] = payload[i] ^ key[i % 4];//decrypt data here
      payload_from_topic+=(char) payload[i];
      Wire.write((char) payload[i]); 
    }
    Wire.endTransmission();
    payload_from_topic=transfer_format(payload_from_topic);
  }
  
  payload_header.concat(present_time);//char + string

  payload_from_topic=payload_header+": "+payload_from_topic;

  Serial.println(payload_from_topic);
  Serial.println();
  Serial.println("-----------END------------");
  //also at this time convey the msg to the public channel
  payload_publish=strdup(payload_from_topic.c_str());//string to const char*
  mqttClient.publish("nico/public", payload_publish);//the last para is const char* !!!
  mqttClient.publish("nico/public", " ");
  payload_from_topic="";

}

void setup() {
  pinMode(BTN, INPUT_PULLUP);
  //the setup is to connect to the wifi
  Serial.begin(9600);
  Serial.println();
  WiFi.begin(wifi_name, passport);
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

  //I2C setup:
  Wire.begin(D1,D2);
}


void loop() {
  mqttClient.loop();
}