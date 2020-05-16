/*zz
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp8266-nodemcu-date-time-ntp-client-server-arduino/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Replace with your network credentials
const char *ssid     = "Monem";
const char *password = "5531-6111-2017-0801-3560";

// Constants
const int INTERVAL = 2*60*60; //seconds

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

void setup() {
  // Initialize Serial Monitor
  Serial.begin(9600);
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);

  //Initialize a NTPClient to get time
  timeClient.begin();
  timeClient.setTimeOffset(7200);

  delay(5000);
}

unsigned long epochTime;

String checksum(String s) {
    byte c = 0;
    for (int i = 0; i < s.length(); i++) {
        c ^= s[i];
    }
    return String(c);
}

void loop() {
  
    timeClient.update();
    epochTime = timeClient.getEpochTime();
    String timeStr = String(epochTime);
    Serial.print(epochTime);
    Serial.println(checksum(timeStr));
    delay(INTERVAL*1000);
}
