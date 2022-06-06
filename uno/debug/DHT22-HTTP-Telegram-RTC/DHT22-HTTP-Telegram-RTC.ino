#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <CTBot.h>   // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <RTClib.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

// Personal Configuration
#include "auth.h"

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

#define DHTPIN 14  // D5
#define DHTTYPE    DHT22

// Object (Sensor) Init
DHT dht(DHTPIN, DHTTYPE);

// Variable Declaration
int dataKe = 0; // For Debug perpouse
float temp, hum, lpg, co, smoke;

void setup() {
  Serial.begin(115200);

  // We start by connecting to a WiFi network
  Serial.print(F("\n\nNode "));
  Serial.println(String(node));
  Serial.print(F("Connecting to "));
  Serial.println(ssid);

  myBot.wifiConnect(ssid, pass);   // connect the ESP8266 to the desired access point
  myBot.setTelegramToken(token);   // set the telegram bot token

  // Initialize device.
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
  dht.begin();
  Wire.begin(5, 4);  //Setting wire (5/D1 untuk SDA dan 4/D2 untuk SCL)
  rtc.begin();
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

void loop() {
  getData();
  delay(500); // wait 500 milliseconds
}

void getData() {
  temp = dht.readTemperature(); // Get Temperature Data
  hum = dht.readHumidity();; // Get Humidity Data
  smoke = 300; // Get Smoke Data

  DateTime now = rtc.now();
  int _second = now.second();

  // Check if any reads failed and exit early (to try again).
  if (isnan(hum) || isnan(temp) || temp > 120 || hum > 100) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  } else {
    if (_second % 30 == 0)
    {
      Serial.println(F("Kirim Data"));
      sendToDB(temp, hum, smoke);
      dataKe++;
    }
  };

  Serial.print(F("Humidity: "));
  Serial.print(hum);
  Serial.print(F("%  Temperature: "));
  Serial.print(temp);
  Serial.println(F("°C "));
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
