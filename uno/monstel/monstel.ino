#include <NTPClient.h>
#include <CTBot.h>   // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include "Utilities.h" // for int64ToAscii() helper function
#include <WiFiUdp.h>

// Personal Configuratio
#include "auth.h"
#include "formula.h"

///////please enter your sensitive data in the Secret tab/auth.h
/////// Wifi Settings ///////
const char* ssid = WIFI_SSID;
const char* pass = WIFI_PASS;
const char* server = "skripsi.skripsif.site/fadli/insertData/";
// https://skripsi.skripsif.site/fadli/insertData/sensor1/30.0/85/62.5/0

// Telegram BOT Section
const char* token = BOTtoken;
const int AdminId = CHAT_ID;

// HW Config
String node = NodeId;
String nodeNo = node.substring(6);

// Sensor Data
int suhu, mq2, hum;

CTBot myBot;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "0.id.pool.ntp.org");

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
  timeClient.begin();
  timeClient.setTimeOffset(25200);

  // check if all things are ok
  if (myBot.testConnection()) {
    Serial.println(F("\nConnection OK"));
    myBot.sendMessage(AdminId, "Bot Node " + String(nodeNo) + ": Connected to " + String(ssid) + "\n\nBot Node " + String(nodeNo) + ": Connection OK");
  } else {
    Serial.println(F("\nConnection Not OK"));
  }
}

void loop() {
  timeClient.update();
  time_t epochTime = timeClient.getEpochTime();
  Serial.println("Human Time: " + timeClient.getFormattedTime());
  Serial.println("Epoch Time: " + String(epochTime));
  if (epochTime % 30 == 0) {
    Serial.println("30 S");
  } else {
    getData();
    getMsg();
  }
  delay(1000); // wait 500 milliseconds
}

void getData() {
  suhu = 27;
  hum = 80;
  mq2 = 450;
}

void getMsg() {
  // a variable to store telegram message data
  TBMessage msg;
  // if there is an incoming message...
  if (CTBotMessageText == myBot.getNewMessage(msg))
  {
    // check if the message is a text message
    if (msg.messageType == CTBotMessageText) {
      // check if the message comes from a chat group (the group.id is negative)
      if (msg.group.id < 0) {
        int id = msg.group.id;
        sendFeedback(id, msg.text);
      } else {
        int id = msg.sender.id;
        sendFeedback(id, msg.text);
      }
      Serial.print(F("Pesan : "));
      Serial.println(msg.text);
    }
  }
}

void sendFeedback(int id, String cmd) {
  if (cmd.indexOf("/status") == 0) {
    myBot.sendMessage(id, "Node " + String(nodeNo) + " Online \n\nSuhu: " + String(suhu) + " ºC \nAsap\t\t: " + String(mq2) + " ppm\nKelembapan: " + String(hum) + " %RH\n");
  }
  if (cmd.indexOf("/temp") == 0) {
    myBot.sendMessage(id, "Node " + String(nodeNo) + " Temperature Status\n\nSuhu: " + String(suhu) + " ºC \nKelembapan: " + String(hum) + " %RH\n");
  }
  if (cmd.indexOf("/asap") == 0) {
    myBot.sendMessage(id, "Node " + String(nodeNo) + " Smoke Status\n\nAsap: " + String(mq2) + " ppm\n");
  }
}

void sendToAPI() {

}
