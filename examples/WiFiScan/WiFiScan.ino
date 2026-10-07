// SPDX-License-Identifier: MIT
#include <FoloToyAIPassport.h>
#include <WiFi.h>

folotoy::AIPassport passport;
bool ready = false;

void setup() {
  Serial.begin(115200);
  ready = passport.begin();
  if (!ready) { Serial.println(passport.lastError()); return; }
  passport.display.setTextColor(ST77XX_WHITE);
  passport.display.setTextSize(1);
  passport.display.setCursor(8, 12);
  passport.display.println("Nearby Wi-Fi networks");
  WiFi.mode(WIFI_STA);
  const int count = WiFi.scanNetworks();
  if (count < 0) passport.display.println("wifi:scan-failed");
  for (int i = 0; i < count && i < 12; ++i) {
    passport.display.printf("%d: %.24s (%d dBm)\n", i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i));
  }
  WiFi.scanDelete();
  WiFi.mode(WIFI_OFF);
}

void loop() {
  if (ready) passport.update();
  delay(5);
}
