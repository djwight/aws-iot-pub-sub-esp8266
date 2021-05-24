# AWS IoT Pub/Sub using a D1 mini (ESP8266)

The reason why I made this repository, was that when I wanted to connect my cheap D1 mini (ESP8266) to the AWS IoT Core to take advantage of the MQTT protocol for Pub/Sub, I found the majority of available tutorials not very helpful. If your basic idea is to publish messages through the MQTT protocol and end up with these messages stored in DynamoDB or S3, then this is the place for you. 

Hopefully, this repository will help your D1 mini/ESP8266 home project.

## Hardware Used
[D1 mini](https://www.az-delivery.de/en/products/d1-mini?variant=28983931474)

The above D1 mini was the one used for testing this code but any D1 mini should work fine

## Required Arduino Libraries

[ESP8266WiFi](https://github.com/esp8266/Arduino)

[MQTT](https://github.com/256dpi/arduino-mqtt)

[ArduinoJson](https://github.com/arduino-libraries/Arduino_JSON)