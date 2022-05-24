#include <NTPClient.h>
// change next line to use with another board/shield
#include <ESP8266WiFi.h>
//#include <WiFi.h> // for WiFi shield
//#include <WiFi101.h> // for WiFi 101 shield or MKR1000
#include <WiFiUdp.h>

const char *ssid     = "POCO M3";
const char *password = "33333333";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "0.id.pool.ntp.org");

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);

  while ( WiFi.status() != WL_CONNECTED ) {
    delay (500);
    Serial.print ( "." );
  }

  timeClient.begin();
  timeClient.setTimeOffset(25200);
}

void loop() {
  timeClient.update();
  time_t epochTime = timeClient.getEpochTime();

  Serial.println("Human Time: " + timeClient.getFormattedTime());
  Serial.print("Epoch Time: ");
  Serial.println(epochTime);
  if (epochTime % 30 == 0) {
    Serial.println("30 S");
  }
  delay(1000);
}
