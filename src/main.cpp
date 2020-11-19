#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <M5Stack.h>
#include "Sensor.hpp"
String ssid;
String password;

WiFiClientSecure *wifiClient;
File file;


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