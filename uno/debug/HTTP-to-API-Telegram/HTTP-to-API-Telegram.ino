#include <CTBot.h>   // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <RTClib.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
// Personal Lib
#include "MQ2.h" // MQ2 Lib

// Personal Configuration
#include "auth.h"
//#include "formula.h"

// API Server
String serverName = "http://skripsi.skripsif.site/fadli/insertData/sensor";

/////// please enter your sensitive data in the Secret tab/auth.h
/////// Wifi Settings ///////
const char* ssid = WIFI_SSID;
const char* pass = WIFI_PASS;

// HW Config
int node = NodeId;

// Telegram BOT Section
const char* token = BOTtoken;
const int AdminId = CHAT_ID;
CTBot myBot;

// RTC
RTC_DS3231 rtc;
char t[32];

// GPIO Config
int mq2Pin = A0;

// Variable Declaration
int dataKe = 0; // For Debug perpouse
int temp, hum, lpg, co, smoke;

// MQ2 Init
MQ2 mq2(mq2Pin);

void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT);

  // We start by connecting to a WiFi network
  Serial.print(F("\n\nNode "));
  Serial.println(String(node));
  Serial.print(F("Connecting to "));
  Serial.println(ssid);

  myBot.wifiConnect(ssid, pass);   // connect the ESP8266 to the desired access point
  myBot.setTelegramToken(token);   // set the telegram bot token

  initIO();

  // check if all things are ok
  if (myBot.testConnection()) {
    Serial.println(F("\nConnection OK"));
    myBot.sendMessage(AdminId, "Node " + String(node) + ": Connected to " + String(ssid) + "\n\nNode " + String(node) + ": Connection OK");
  } else {
    Serial.println(F("\nConnection Not OK"));
  }
}

// OK
void initIO() {
  mq2.begin();
  Wire.begin(5, 4);  //Setting wire (5/D1 untuk SDA dan 4/D2 untuk SCL)
  rtc.begin();
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

void loop() {
  getData();
  delay(500); // wait 500 milliseconds
}

void getData() {
  temp = random(18, 30); // Get Temperature Data
  hum = random(50, 85); // Get Humidity Data
  smoke = mq2.readSmoke(); // Get Smoke Data

  DateTime now = rtc.now();
  int _second = now.second();

  if (_second % 30 == 0)
  {
    Serial.println(F("Kirim Data"));
    sendToDB(temp, hum, smoke);
    dataKe++;
  }
  Serial.println("S :" + String(_second) + " | Data Ke-" + String(dataKe) + " | Smoke :" + String(smoke)); //set it false if you don't want to print the values to the Serial
}

void sendToDB(int temp, int humidity, int smoke) {
  if (myBot.testConnection()) {
    WiFiClient client;
    HTTPClient http;

    String serverPath = serverName + node + "/" + String(temp) + "/" + String(humidity) + "/" + String(smoke) +  "/0";
    Serial.println(serverPath);

    // Your Domain name with URL path or IP address with path
    http.begin(client, serverPath.c_str());

    // Send HTTP GET request
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      Serial.print(F("HTTP Response code: "));
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);

      // Debug porpouse
      DateTime now = rtc.now();
      int _s = now.second();
      int _m = now.minute();
      int _h = now.hour();

      // Send error if respons code not 200
      if (httpResponseCode == 200) {
        myBot.sendMessage(AdminId, "Node " + String(node) + ": Sendata to Database Success\n\nData Ke : " + String(dataKe) + "\nSmoke : " + String(smoke) + " ppm\nResponse Code: " + String(httpResponseCode) + "\nPayload : " + String(payload) + "\n\nSend Time : " + String(_h) + ":" + String(_m) + ":" + String(_s));
      } else {
        myBot.sendMessage(AdminId, "Node " + String(node) + ": Sendata to Database Error \n\nError Code: " + String(httpResponseCode) + "\nPayload : " + String(payload));
      }
    }
    else {
      Serial.print(F("Error code: "));
      Serial.println(httpResponseCode);
      myBot.sendMessage(AdminId, "Node " + String(node) + ": Sendata to Database Error, \n\nError Code: " + String(httpResponseCode));
    }
    http.end(); // Free resources
  }
  else {
    Serial.println(F("Not Connected to Internet"));
  }
}

void getMsg() {
  if (1 == 2) {}
}
