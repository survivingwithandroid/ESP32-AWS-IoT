#include <Arduino.h>
#include <WiFi.h>
#include <certs.h>
#include <aws.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// DHT
#include <Adafruit_Sensor.h>
#include <DHT.h>

#define DHT_TYPE DHT11
#define DHT_PIN 4

const char *SSID = "you_ssid";
const char *PWD = "your_pwd";

WiFiClientSecure secureClient = WiFiClientSecure();
PubSubClient mqttClient(secureClient);
DHT dht(DHT_PIN, DHT_TYPE);


void connectToWiFi() {
  Serial.print("Connecting to ");
  Serial.println(SSID);
  
  WiFi.begin(SSID, PWD);
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
    // we can even make the ESP32 to sleep
  }

  Serial.print("Connected - ");
  //Serial.println(WiFi.localIP);
}

void connectToAWS() {
  mqttClient.setServer(AWS_END_POINT, 8883);
  secureClient.setCACert(AWS_PUBLIC_CERT);
  secureClient.setCertificate(AWS_DEVICE_CERT);
  secureClient.setPrivateKey(AWS_PRIVATE_KEY);

  Serial.println("Connecting to MQTT....");

  mqttClient.connect(DEVICE_NAME);
  
  while (!mqttClient.connected()) {
    Serial.println("Connecting to MQTT....Retry");
    mqttClient.connect(DEVICE_NAME);
    delay(5000);
  }

  Serial.println("MQTT Connected");
}

void setup() {
  Serial.begin(9600);
  dht.begin();
  connectToWiFi();
  connectToAWS();

}

void loop() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  // Compute the hit Index using celsius
  float heatIndex = dht.computeHeatIndex(temperature, humidity, false);

  Serial.print("Temperature:");
  Serial.println(temperature);
  Serial.print("Humidity:");
  Serial.println(humidity);
  Serial.print("Heat Index:");
  Serial.println(heatIndex);
 
  StaticJsonDocument<128> jsonDoc;
  JsonObject eventDoc = jsonDoc.createNestedObject("event");
  eventDoc["temp"] = temperature;
  eventDoc["hum"] = humidity;
  eventDoc["hi"] = heatIndex;

  char jsonBuffer[128];

  serializeJson(eventDoc, jsonBuffer);
  mqttClient.publish("mychannel/", jsonBuffer);
  delay(10000);
}