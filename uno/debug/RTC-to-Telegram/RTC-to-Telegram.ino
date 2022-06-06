#include <CTBot.h>   // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <RTClib.h>
#include <Wire.h>

// Personal Configuration
#include "auth.h"
//#include "formula.h"

///////please enter your sensitive data in the Secret tab/auth.h
/////// Wifi Settings ///////
const char* ssid = WIFI_SSID;
const char* pass = WIFI_PASS;

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
    if (!Serial) {
      myBot.sendMessage(AdminId, "Node " + String(nodeNo) + ": Powered via Debig Mode");
    }
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
  sendToDB();
  delay(1000); // wait 500 milliseconds
}

void sendToDB() {
  DateTime now = rtc.now();
  int s = now.second();
  if (s % 30 == 0)
  {
    myBot.sendMessage(AdminId, "Counter: " + String(s));
    Serial.println(F("Kirim Data"));
  }
  Serial.println(s);
}
