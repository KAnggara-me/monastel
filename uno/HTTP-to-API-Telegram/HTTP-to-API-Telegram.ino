#include <CTBot.h>   // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <RTClib.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

// Personal Configuration
#include "auth.h"
//#include "formula.h"

///////please enter your sensitive data in the Secret tab/auth.h
/////// Wifi Settings ///////
const char* ssid = WIFI_SSID;
const char* pass = WIFI_PASS;

// API Server
String serverName = "http://skripsi.skripsif.site/fadli/insertData/";

// Telegram BOT Section
const char* token = BOTtoken;
const int AdminId = CHAT_ID;
CTBot myBot;

// HW Config
String node = NodeId;
String nodeNo = node.substring(6);

// RTC
RTC_DS3231 rtc;
char t[32];

void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT);

  // We start by connecting to a WiFi network
  Serial.print(F("\n\nNode "));
  Serial.println(String(nodeNo));
  Serial.print(F("Connecting to "));
  Serial.println(ssid);

  myBot.wifiConnect(ssid, pass);   // connect the ESP8266 to the desired access point
  myBot.setTelegramToken(token);   // set the telegram bot token

  initIO();

  // check if all things are ok
  if (myBot.testConnection()) {
    Serial.println(F("\nConnection OK"));
    myBot.sendMessage(AdminId, "Node " + String(nodeNo) + ": Connected to " + String(ssid) + "\n\nNode " + String(nodeNo) + ": Connection OK");
  } else {
    Serial.println(F("\nConnection Not OK"));
  }
}

void initIO()
{
  Wire.begin(5, 4);  //Setting wire (5/D1 untuk SDA dan 4/D2 untuk SCL)
  rtc.begin();
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

void loop() {
  getData();
  delay(1000); // wait 500 milliseconds
}

void getData() {
  int temp = random(18, 30);
  int hum = random(50, 85);
  int smoke = random(400, 950);
  DateTime now = rtc.now();
  int _second = now.second();
  if (_second % 30 == 0)
  {
    sendToDB(temp, hum, smoke);
    myBot.sendMessage(AdminId, "Counter: " + String(_second));
    Serial.println(F("Kirim Data"));
  }
  Serial.println(_second);
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
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
      // Send error if respons code not 200
      if (httpResponseCode != 200) {
        myBot.sendMessage(AdminId, "Node " + String(nodeNo) + ": Sendata to Database Error, \n\nError Code: " + String(httpResponseCode) + "\n\nPayload : " + String(payload));
      }
    }
    else {
      Serial.print(F("Error code: "));
      Serial.println(httpResponseCode);
      myBot.sendMessage(AdminId, "Node " + String(nodeNo) + ": Sendata to Database Error, \n\nError Code: " + String(httpResponseCode));
    }
    // Free resources
    http.end();
  }
  else {
    Serial.println("Not Connected to Internet");
  }
}
