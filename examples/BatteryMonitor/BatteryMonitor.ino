// SPDX-License-Identifier: MIT
#include <FoloToyAIPassport.h>

folotoy::AIPassport passport;
uint32_t checkedAt = 0;
bool ready = false;

void setup() {
  Serial.begin(115200);
  folotoy::PassportConfig config;
  config.display = false;
  config.battery = true;
  ready = passport.begin(config);
  if (!ready) Serial.println(passport.lastError());
}

void loop() {
  if (!ready) { delay(100); return; }
  passport.update();
  if (millis() - checkedAt >= 1000) {
    checkedAt = millis();
    const int percent = passport.battery.percent();
    const int mv = passport.battery.millivolts();
    if (percent < 0 || mv < 0) Serial.println("battery:read-failed-or-not-ready");
    else Serial.printf("Battery: %d%%, %d mV\n", percent, mv);
  }
  delay(5);
}
