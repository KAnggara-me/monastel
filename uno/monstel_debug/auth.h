/**********************************************************************************/
//WiFi Section
//#define WIFI_SSID "KAnggara75v7"  // WiFi Name
//#define WIFI_PASS "kenamkali3"     // WiFi Pass
#define WIFI_SSID "POCO M3"  // WiFi Name
#define WIFI_PASS "33333333"     // WiFi Pass
/**********************************************************************************/
//Telegram Section
//your Bot Token (Get from Botfather)
#define token "5368104163:AAHURKnqe6z7ajbIp1-uonwRPrAHtWTaeFI"
#define AdminId 1000839880 // Telegram Uuer ID (Must Integer) as know as AdminId
/**********************************************************************************/
// Server Section
#define node 1 // API Server Endpoint (Must Integer)
/**********************************************************************************/
// HW Config
// GPIO Config
#define mq2Pin A0 // ESP8266 Pin A0
#define dhtPin 14 // ESP8266 Pin D5
/**********************************************************************************/
// Sensor Variable
// Temperature
const int lowTemp = 18;
const int highTemp = 33;

// Humidity
const int lowHum = 40;
const int highHum = 80;

// Smoke
const int highSmoke = 205;
const int normalSmoke = 200;
/**********************************************************************************/
