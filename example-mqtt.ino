#include <PubSubClient.h>
#include <WiFi.h>

const char* ssid = "yourWiFiSSID";
const char* password = "yourWifiPassword";

// mqtt broker server

WiFiClient espClient;

PubSubClient mqttClient(espClient);


long lastMsg = 0;
int value = 0;
const int relayOne = 25;
const int lightSensor = 32;
float resultLightSensor;

void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.println("");
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("-");
  }

  Serial.println("");
  Serial.println("Wifi connected");
  Serial.println("IP Address: ");
  Serial.println(WiFi.localIP());

  mqttClient.setServer("mqttServer", 3478);
  mqttClient.setCallback(mqttSub);
  pinMode(relayOne, OUTPUT);
  pinMode(lightSensor, INPUT);

  // put your setup code here, to run once:
}

void mqttSub(char* topic, byte* message, unsigned int length){
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print("ini looping ke : ");
    Serial.println(i);
    Serial.println((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println("ini message temp :" + messageTemp);
  // Serial.println();

  if (String(topic) == "esp32/output") {
    Serial.print("Changing output to ");
    if(messageTemp == "on"){
      Serial.println("on");
      digitalWrite(relayOne, LOW);
      mqttClient.publish("esp32/lightSensor/state", "on");
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      digitalWrite(relayOne, HIGH);
      mqttClient.publish("esp32/lightSensor/state", "off");
    }
  }
}

// reconnect to mqtt
void reconnect() {
  while (!mqttClient.connected()) {
    Serial.println("Attempting MQTT connection ...");
    
    if (mqttClient.connect("ESP32-sudobash", "admin", "SecretPassword")) {
    //  if mqtt server use anonymous security
    // if (mqttClient.connect("ESP32-sudobash")) {
      Serial.println("connected");
      mqttClient.subscribe("esp32/output");
    } else {
      Serial.print("failed, reconnect= ");
      Serial.print(mqttClient.state());
      Serial.print(" try again in 3 seconds");
      delay(3000);

    }
  }
}

void loop() {
  if (!mqttClient.connected()){
    reconnect();
  }
  mqttClient.loop();
  long now = millis();
  if(now - lastMsg > 1000){
    lastMsg = now;
    resultLightSensor = analogRead(lightSensor);
    char tempResultLightSensor[9];
    dtostrf(resultLightSensor, 6, 3, tempResultLightSensor);
    //Serial.println(tempResultLightSensor);
    mqttClient.publish("esp32/lightSensor", tempResultLightSensor);
    mqttClient.publish("garden/waterpump/available", "1");
  }
}
