#include <ArduinoJson.h>
#include <ArduinoMqttClient.h>
#include <ESP8266WiFi.h>
#include <DFRobot_DHT11.h>
DFRobot_DHT11 DHT;
#define DHT11_PIN 13

char ssid[] = "iPhone";    // your network SSID (name)
char pass[] = "12455434";    // your network password (use for WPA, or use as key for WEP)

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

const char broker[]    = "iot-06z00dzycoxz6ra.mqtt.iothub.aliyuncs.com";
int        port        = 1883;
const char inTopic[]   = "/sys/k0x0urPKldE/ESP8266/thing/service/property/set";
const char outTopic[]  = "/sys/k0x0urPKldE/ESP8266/thing/event/property/post";

const long interval = 10000;
unsigned long previousMillis = 0;

int count = 0;
String inputString = "";



void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; 
  }
  pinMode(4,OUTPUT);
  digitalWrite(4,HIGH);
  
  // attempt to connect to WiFi network:
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    
    Serial.print(".");
    delay(5000);
  }

  Serial.println("You're connected to the network");
  Serial.println();

  mqttClient.setId("k0x0urPKldE.ESP8266|securemode=2,signmethod=hmacsha256,timestamp=1710377100434|");
  mqttClient.setUsernamePassword("ESP8266&k0x0urPKldE","892b37e32abeab193f4129500f468a7fac2543d7fffd9dff93671808b2015f48");





  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1);
  }

  Serial.println("You're connected to the MQTT broker!");
  Serial.println();


  mqttClient.onMessage(onMqttMessage);//回调函数

  Serial.print("Subscribing to topic: ");
  Serial.println(inTopic);
  Serial.println();

  int subscribeQos = 1;
  mqttClient.subscribe(inTopic, subscribeQos);

 

  Serial.print("Waiting for messages on topic: ");
  Serial.println(inTopic);
  Serial.println();
}

void loop() {

  mqttClient.poll();

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {

    previousMillis = currentMillis;

    String payload;

    DHT.read(DHT11_PIN);
    DynamicJsonDocument json_msg (512);
     DynamicJsonDocument json_data(512);
   

   
    json_data["temp"] = DHT.temperature;
    json_data["humi"] = DHT.humidity   ;
    json_msg["params"] = json_data     ;
    json_msg["version"] = "1.0.0"      ;
    // 序列化
    serializeJson(json_msg,payload);
    
   

    Serial.print("Sending message to topic: ");
    Serial.println(outTopic);
    Serial.println(payload);


    bool retained = false;
    int qos = 1;
    bool dup = false;

    mqttClient.beginMessage(outTopic, payload.length(), retained, qos, dup);
    mqttClient.print(payload);
    mqttClient.endMessage();

    Serial.println();

    count++;
  }
}

void onMqttMessage(int messageSize) {
  // we received a message, print out the topic and contents
  Serial.print("Received a message with topic '");
  Serial.print(mqttClient.messageTopic());
  Serial.print("', duplicate = ");
  Serial.print(mqttClient.messageDup() ? "true" : "false");
  Serial.print(", QoS = ");
  Serial.print(mqttClient.messageQoS());
  Serial.print(", retained = ");
  Serial.print(mqttClient.messageRetain() ? "true" : "false");
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");

  // use the Stream interface to print the contents
  while (mqttClient.available()) {
    Serial.print((char)mqttClient.read());
    char inchar = (char)mqttClient.read();
    inputString += inchar;
    if(inputString.length()==messageSize)
    {
      DynamicJsonDocument json_msg (1024);
      DynamicJsonDocument json_item (1024);
      DynamicJsonDocument json_value (1024);
      //反序列化，变为json对象
      deserializeJson(json_msg,inputString);
      String items = json_msg["items"];
      deserializeJson(json_item,items);
      String led = json_item["led"];
      deserializeJson(json_value,led);
      bool value = json_value["value"];


      if(value == 0)
      {
        digitalWrite(4,HIGH);
        Serial.println("off");
      }
      else
      {
        digitalWrite(4,LOW);
        Serial.println("on");
      }
      inputString = "";



      
    }
  }
  Serial.println();

  Serial.println();
}
