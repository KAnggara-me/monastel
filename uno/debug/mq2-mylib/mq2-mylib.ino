// Personal Lib
#include "MQ2.h" // MQ2 Lib

// Personal Configuration
#include "config.h"

// Variable Declaration
float smoke;

// MQ2 Init
MQ2 mq2(mq2Pin);

void setup() {
  Serial.begin(115200);
  mq2.begin();
  delay(2000);
}

void loop() {
  smoke = mq2.readSmoke(); // Get Smoke Data
  Serial.println(F("Smoke \t\t:") + String(smoke));
  delay(1000); // wait 500 milliseconds
}
