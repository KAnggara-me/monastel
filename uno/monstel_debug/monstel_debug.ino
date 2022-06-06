#include <DHT.h>
#include <Wire.h>
#include <CTBot.h>
#include <RTClib.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// Personal Lib
#include "MQ2.h" // MQ2 Lib

// Personal Configuration
#include "auth.h"

// API Server
String serverName = "http://skripsi.skripsif.site/fadli/insertData/sensor";

/////// please enter your sensitive data in the Secret tab/auth.h
/////// Wifi Settings ///////
const char* ssid = WIFI_SSID;
const char* pass = WIFI_PASS;

// Variable Declaration
int dataKe = 0; // For Debug perpouse
int _second, prevSmoke;
bool debugMode = false;
float temp, hum, smoke;
bool tempWarning, humWarning;
unsigned long prevMillis = 0;        // will store last time LED was updated
unsigned long currentMillis;

// Telegram BOT Section
//const char* token = BOTtoken;
CTBot myBot;

// RTC
RTC_DS3231 rtc;

// MQ2 Init
MQ2 mq2(mq2Pin);

// Object (Sensor) Init
DHT dht(dhtPin, DHT22);

// NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "0.id.pool.ntp.org");

void setup() {
  Serial.begin(115200);

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
    rtc.adjust(DateTime(2022, 6, timeClient.getDay(), timeClient.getHours(), timeClient.getMinutes(), timeClient.getSeconds()));
    myBot.sendMessage(AdminId, "Node " + String(node) + ": Connected to " + String(ssid) + "\n\nNode " + String(node) + ": Connection OK \n\nDebug Mode : " + debugMode);
  } else {
    Serial.println(F("\nConnection Not OK"));
  }

  delay(2000);
  temp = dht.readTemperature(); // Get Temperature Data
  hum = dht.readHumidity(); // Get Humidity Data
  smoke = mq2.readSmoke(); // Get Smoke Data
}

// OK
void initIO() {
  dht.begin();
  mq2.begin();
  timeClient.begin();
  timeClient.setTimeOffset(25200);
  timeClient.update();
  Wire.begin(5, 4);  //Setting wire (5/D1 untuk SDA dan 4/D2 untuk SCL)
  rtc.begin();
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

void loop() {
  currentMillis = millis();
  DateTime now = rtc.now();
  _second = now.second();

  if ((_second % 30 == 0) || (currentMillis - prevMillis >= 30000)) {
    // save the last time you blinked the LED
    prevMillis = currentMillis;
    sendToDB();
  }

  if (_second % 2 == 0) {
    temp = dht.readTemperature(); // Get Temperature Data
    hum = dht.readHumidity(); // Get Humidity Data
    smoke = mq2.readSmoke(); // Get Smoke Data
    Serial.println(F("\n\nRTC \t\t:") + String(_second));
    Serial.println(F("Debug Mode \t:") + String(debugMode));
    Serial.println(F("\nSuhu \t\t:") + String(temp));
    Serial.println(F("Smoke \t\t:") + String(smoke));
    Serial.println(F("Kelembapan \t:") + String(hum));
    Serial.println(F("======================================="));
  }

  checkData();
  getMsg();
  delay(1000); // wait 500 milliseconds
}

void getMsg() {
  // a variable to store telegram message data
  TBMessage msg;

  // if there is an incoming message...
  if (myBot.getNewMessage(msg))
  {
    // check if the message is a text message
    if (msg.messageType == CTBotMessageText) {
      int id;
      // check if the message comes from a chat group (the group.id is negative)
      if (msg.group.id < 0) {
        if ((msg.text.indexOf("/calibrate") == 0)) {
          id = msg.sender.id;
        } else {
          id = msg.group.id;
        }
        sendFeedback(id, msg.text);
      } else {
        id = msg.sender.id;
        sendFeedback(id, msg.text);
      }
      Serial.print(F("Pesan : "));
      Serial.println(msg.text);
    }
  }
}

void sendFeedback(int id, String cmd) {
  if (cmd.indexOf("/status") == 0) {
    myBot.sendMessage(id, "Node " + String(node) + " Online \n\nSuhu : " + String(temp) + " ºC \nAsap\t\t: " + String(smoke) + " ppm\nKelembapan : " + String(hum) + " %RH\n");
  }

  if ((cmd.indexOf("/calibrate") == 0) && (id == AdminId)) {
    myBot.sendMessage(id, "Node " + String(node) + " \n\nCalibrate MQ-2: Start...");
    mq2.begin();
    myBot.sendMessage(id, "Node " + String(node) + " \n\nCalibrate MQ-2: Finish..");
  }

  if ((cmd.indexOf("/debug") == 0) && (id == AdminId)) {
    debugMode = !debugMode;
    myBot.sendMessage(id, debugMode ? "Node " + String(node) + "\n\nDebug Mode: ON" : "Node " + String(node) + "\n\nDebug Mode: OFF");
  }
}

void checkData() {
  //  Temperature Section
  if (temp < lowTemp) {
    tempWarning  = true;
    myBot.sendMessage(AdminId, "Suhu Sekarang kurang dari " + String(lowTemp) + "ºC pada Ruang Server Node " + String(node) + "\n\nSuhu: " + String(temp) + "ºC\nMohon Cek Ruangan Server!!!");
  }

  if (temp > highTemp) {
    tempWarning = true;
    myBot.sendMessage(AdminId, "Suhu Sekarang melebihi dari " + String(highTemp) + "ºC pada Ruang Server Node " + String(node) + "\n\nSuhu: " + String(temp) + "ºC\nMohon Cek Ruangan Server!!!");
  }

  if ((temp < highTemp && temp > lowTemp) && tempWarning) {
    myBot.sendMessage(AdminId, "Suhu Telah Normal pada Ruang Server Node " + String(node) + "\n\nSuhu: " + String(temp) + "ºC");
    tempWarning = false;
  }

  //  Humidity Section
  if (hum < lowHum) {
    myBot.sendMessage(AdminId, "Kelembapan Sekarang Kurang dari " + String(lowHum) + "%RH pada Ruang Server Node " + String(node) + "\n\nKelembapan: " + String(hum) + "%RH\nMohon Cek Ruangan Server!!!");
    humWarning = true;
  }

  if (hum > highHum) {
    myBot.sendMessage(AdminId, "Kelembapan Sekarang Lebih dari " + String(highHum) + "%RH pada Ruang Server Node " + String(node) + "\n\nKelembapan: " + String(hum) + "%RH\nMohon Cek Ruangan Server!!!");
    humWarning = true;
  }

  if ((hum < highHum && hum > lowHum) && humWarning) {
    myBot.sendMessage(AdminId, "Kelembapan Telah Normal pada Ruang Server Node " + String(node) + "\n\nKelembapan: " + String(hum) + "%RH");
    humWarning = false;
  }

  //  Smoke Section
  if (smoke > 205) {
    prevSmoke = smoke;
    myBot.sendMessage(AdminId, "Terdeteksi Asap pada Ruang Server Node " + String(node) + "\n\nAsap: " + String(smoke) + "ppm");
  }

  if (prevSmoke > highSmoke && smoke < highSmoke) {
    myBot.sendMessage(AdminId, "Asap sudah Tidak Terdeteksi pada Ruang Server Node " + String(node) + "\n\nAsap: " + String(smoke) + "ppm");
  }

  if (humWarning || tempWarning || (smoke > 205)) {
    sendToDB();
    prevSmoke = normalSmoke;
    humWarning = false;
    tempWarning = false;
  }
}

void sendToDB() {
  // Check if any reads failed and exit early (to try again).
  if (isnan(hum) || isnan(temp) || temp > 81 || hum > 100) {
    Serial.println(F("Failed to read from DHT sensor!"));
    myBot.sendMessage(AdminId, "Node " + String(node) + "\n\nFailed to read from DHT sensor!");
    return;
  } else {
    dataKe++;
    Serial.println(F("Kirim Data"));

    // Check Connection status
    if (myBot.testConnection()) {
      WiFiClient client;
      HTTPClient http;
      String serverPath = serverName + node + "/" + String(temp) + "/" + String(hum) + "/" + String(smoke) +  "/0";

      // Your Domain name with URL path or IP address with path
      http.begin(client, serverPath.c_str());

      // Send HTTP GET request
      int httpResponseCode = http.GET();

      if (debugMode) {
        if (httpResponseCode > 0) {
          Serial.println(serverPath);
          Serial.print(F("HTTP Response code: "));
          Serial.println(httpResponseCode);
          String payload = http.getString();
          Serial.println(payload);
          myBot.sendMessage(AdminId, "Node " + String(node) + ": Send data to Database Success\n\nData Ke : " + String(dataKe) + "\nSmoke : " + String(smoke) + " ppm\nTemp: " + String(temp) + "ºC\nHumidity: " + String(hum) + "%RH\nResponse Code: " + String(httpResponseCode) + "\nPayload : " + String(payload) + "\n\nSend On : " + String(timeClient.getFormattedTime()));
        } else {
          Serial.print(F("Error code: "));
          Serial.println(httpResponseCode);
          myBot.sendMessage(AdminId, "Node " + String(node) + ": Send data to Database Error\n\nError Code: " + String(httpResponseCode));
        }
      }

      http.end(); // Free resources

    } else {
      Serial.println(F("Not Connected to Internet"));
    }
  }
}
