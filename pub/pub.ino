#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFiClientSecure.h>
#include <MQTT.h>

#include "secrets.h"

// Time client config for certificate validation
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

// Secure Wifi client config
WiFiClientSecure net;
BearSSL::X509List cert(AWS_CERT_CA);
BearSSL::X509List client_crt(AWS_CERT_CRT);
BearSSL::PrivateKey key(AWS_CERT_PRIVATE);

// ESP8266 MQTT client config
const int MQTT_PORT = 8883;
const char MQTT_PUB_TOPIC[] = "iot/test_topic"; // Add you specifc topic for publishing to here
MQTTClient client;

//////////////////////////////////////////////////////////// 
///                ACCESSORY FUNCTIONS
////////////////////////////////////////////////////////////

/**
 * Initialises connection to the WiFi
 */
void setupWifi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);
  delay(1000);
  WiFi.hostname(THINGNAME);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  // Print until connected
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");

  }
  Serial.println("");
  Serial.println("WiFi Connected!");
  timeClient.begin();
  while(!timeClient.update()){
    timeClient.forceUpdate();
  }
  net.setX509Time(timeClient.getEpochTime());
}

/**
 * Initialises the MQTT broker connection to AWS.
 */
void connectMqtt() {
  Serial.println("MQTT Connecting...");
  while (!client.connected()) {
    if (client.connect(THINGNAME)) {
      Serial.println("MQTT Connected!");
    } else {
      Serial.println("MQTT Connection failed -> repeating in 5 seconds");
      delay(5000);
    }
  }
  delay(1000);
}

/**
 * Published as message to the AWS IoT core.
 */
void publishMessage(char* msg) {
  StaticJsonDocument<200> doc;
  doc["id"] = "bitter_orange";
  doc["message"] = msg;
  doc["timestamp"] = timeClient.getEpochTime();
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client

  // Publish the json message to AWS
  client.publish(MQTT_PUB_TOPIC, jsonBuffer);
  Serial.print("Message published to: ");
  Serial.println(MQTT_PUB_TOPIC);
}

//////////////////////////////////////////////////////////// 
///           END OF ACCESSORY FUNCTIONS
////////////////////////////////////////////////////////////


void setup() {
  Serial.begin(115200);

  // Wifi set up
  setupWifi();

  // Set up secure MQTT connection
  net.setTrustAnchors(&cert);
  net.setClientRSACert(&client_crt, &key);
  client.begin(AWS_IOT_ENDPOINT, MQTT_PORT, net);
  connectMqtt();
}

void loop() {
  // Re-connect wifi and/or mqtt connection if lost
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi Connection lost, reconnecting...");
    setupWifi();
  }
  if (!client.connected()) {
    Serial.println("MQTT Connection lost, reconnecting...");
    connectMqtt();
  }
  
  // Publish message to AWS IoT Core
  publishMessage("data to be sent");
  delay(10000);
}
