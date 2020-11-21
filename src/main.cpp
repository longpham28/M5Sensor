#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <M5Stack.h>
#include "Sensor.hpp"
#include <ArduinoJson.h>
#include <PubSubClient.h>

String ssid;
String password;

WiFiClientSecure *wifiClient;
File file;

String macAdd;

bool mqttConnected = false;
String mqttServer = "fc.happysocial.net";
int mqttPort = 8883;
String mqttUsername  = "yarukiswitch";
String mqttPassword = "yarukiyaruki";

PubSubClient *mqttClient = new PubSubClient(*wifiClient);


void write() {
  Serial.println("Writing to SD");
  file = SD.open("config.ini", FILE_WRITE);
  file.println(ssid.c_str());
  file.println(password.c_str());
  file.close();
  Serial.println("Wrote to SD");
}

String readFile(String path) {
  file = SD.open(path);
  String str = file.readString();
  file.close();
  return str;
}

String readSsid() {
  String ssid = readFile("/ssid.txt");
  ssid.trim();
  return ssid;
}

String readPassword() {
  String password =  readFile("/password.txt");
  password.trim();
  return password;
}

void connectWifi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid.c_str(), password.c_str());
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  String macAdd = WiFi.macAddress();
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Mac Address: ");
  Serial.println(macAdd);
}

void connectMQtt() {
  mqttConnected = false;
  mqttClient->setServer(mqttServer.c_str(), mqttPort);
  while (!mqttClient->connected())
  {
    Serial.println(macAdd);
    Serial.println("Connecting to MQTT...");
    if (mqttClient->connect(macAdd.c_str(), mqttUsername.c_str(), mqttPassword.c_str()))
    {
      Serial.println("connected");
      mqttConnected = true;
    } else {
      Serial.print("Failed. Error state=");
      Serial.print(mqttClient->state());
      Serial.println("");
    }
    delay(1000);
    randomSeed(micros());
  }
}

void setup() {
  M5.begin();
  M5.Power.begin();
  MPU6885.init();
  Serial.begin(115200);
  ssid = readSsid();
  password = readPassword();
  Serial.println(ssid);
  Serial.println(password);

}

void loop() {
  // put your main code here, to run repeatedly:
  collectSensordata();
  Serial.print("AccX: ");
  Serial.println(accX);
  Serial.print("AccY: ");
  Serial.println(accY);
  Serial.print("AccZ: ");
  Serial.println(accZ);
  Serial.println("-----------------");
  delay(100);
}