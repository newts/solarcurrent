/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com  
*********/
// https://randomnerdtutorials.com/esp32-mqtt-publish-subscribe-arduino-ide/

#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Arduino_JSON.h>

// define SECRET_SSID and SECRET_PASSWORD in this file
#include "secret.h"

// Replace the next variables with your SSID/Password combination
const char* ssid = SECRET_SSID;
const char* password = SECRET_PASSWORD;

// Add your MQTT Broker IP address, example:
//
const char* mqtt_server = "192.168.1.17";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
char topic[50];


// LED Pin
const int ledPin = 2;

char mac_buf[50];

void setup() {
  Serial.begin(115200);
  // default settings
  setup_wifi();
  sprintf(topic,"solar-%s/currents", mac_buf);
  Serial.print("hot topic: ");
  Serial.println(topic);
  
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(ledPin, OUTPUT);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

   WiFi.macAddress().toCharArray(mac_buf,20);
   Serial.println(mac_buf);
   // remove colons
   //
   char *mp = mac_buf;
   while(*mp) {
     if (*mp == ':') {
       strcpy(mp,mp+1);
     }
     mp++;
   }
   Serial.println(mac_buf);
}

void callback(char* topic, byte* message, unsigned int length) {
#ifdef DEBUG_MQTT
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.println(". Message: ");
#endif

  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }


  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (String(topic) == "esp32/output") {
 #ifdef DEBUG_MQTT
    Serial.print("Changing output to ");
 #endif
    if(messageTemp == "on"){
#ifdef DEBUG_MQTT
      Serial.println("on");
#endif
      digitalWrite(ledPin, HIGH);
    }
    else if(messageTemp == "off"){
#ifdef DEBUG_MQTT
      Serial.println("off");
#endif
      digitalWrite(ledPin, LOW);
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// https://microcontrollerslab.com/adc-esp32-measuring-voltage-example/

void loop() {
  int c1, c2, c3, c4;
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 6000) {
    lastMsg = now;

    JSONVar payload;
    payload["machine_id"] = mac_buf;

    // 0 3 6 7   36 39 34 35 
    c1 = analogRead(34);
    c2 = analogRead(35);
    c3 = analogRead(32);
    c4 = analogRead(33);
    Serial.printf("%d,%d,%d,%d\n", c1,c2,c3,c4);
    
    payload["c1"] = c1;
    payload["c2"] = c2;
    payload["c3"] = c3;
    payload["c4"] = c4;

    Serial.println(payload);
    String payload_string = JSON.stringify(payload);
    char payload_buf[200];
    payload_string.toCharArray(payload_buf, 200);
    
    client.publish(topic,payload_buf);
  }
}
