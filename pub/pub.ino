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
const char MQTT_PUB_TOPIC[] = "iot_topic"; // Add you specifc topic for publishing to here
MQTTClient client;

//////////////////////////////////////////////////////////// 
///                ACCESSORY FUNCTIONS
////////////////////////////////////////////////////////////

/**
 * Initialises connection to the wifi
 * 
 * @param ssid SSID of the wifi
 * @param password wifi passord
 */
void setupWifi(const char* ssid, const char* password) {
  u8g2.drawStr(0, 10, "Connecting to:");
  u8g2.drawStr(0, 20, WIFI_SSID);
  u8g2.sendBuffer();
  delay(1000);
  WiFi.hostname(THINGNAME);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  // Print connecting until connected
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    u8g2.setCursor(0,40);
    u8g2.print("WiFi Connecting...");
    sendClear(500, "display");
  }
  timeClient.begin();
  while(!timeClient.update()){
    timeClient.forceUpdate();
  }
  net.setX509Time(timeClient.getEpochTime());
}

/**
 * 
 */
void connectMqtt() {
  u8g2.drawStr(0, 10, "MQTT Connecting...");
  u8g2.sendBuffer();
  while (!client.connected()) {
    if (client.connect(THINGNAME)) {
      u8g2.drawStr(0, 20, "MQTT Connected!");
      u8g2.sendBuffer();
    } else {
      u8g2.drawStr(0, 20, " MQTT Connection failed -> repeating in 5 seconds");
      u8g2.sendBuffer();
      delay(5000);
    }
  }
  delay(1000);
  u8g2.clearDisplay();
}

/**
 * 
 */
void publishMessage(char* msg) {
  StaticJsonDocument<200> doc;
  doc["id"] = "bitter_orange";
  doc["message"] = msg;
  doc["timestamp"] = timeClient.getEpochTime();
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client

  client.publish(MQTT_PUB_TOPIC, jsonBuffer);
}


//////////////////////////////////////////////////////////// 
///           END OF ACCESSORY FUNCTIONS
////////////////////////////////////////////////////////////

void setup() {
  // OLED setup
  Serial.begin();

  // Wifi set up
  setupWifi(WIFI_SSID, WIFI_PASS);

  // Set up secure MQTT connection
  net.setTrustAnchors(&cert);
  net.setClientRSACert(&client_crt, &key);
  client.begin(AWS_IOT_ENDPOINT, MQTT_PORT, net);
  connectMqtt();
}

void loop() {
  // Re-connect wifi and/or mqtt connection if lost
  if (WiFi.status() != WL_CONNECTED) {
    setupWifi();
  }
  if (!client.connected()) {
    connectMqtt();
  }
  
  // Publish message to AWS IoT Core
  publishMessage(reading);

  delay(10000);
}