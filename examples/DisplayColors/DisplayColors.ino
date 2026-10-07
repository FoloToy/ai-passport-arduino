// SPDX-License-Identifier: MIT
#include <FoloToyAIPassport.h>

folotoy::AIPassport passport;
uint32_t changedAt = 0;
uint8_t colorIndex = 0;
const uint16_t colors[] = {ST77XX_RED, ST77XX_GREEN, ST77XX_BLUE, ST77XX_WHITE};
bool ready = false;

void setup() {
  Serial.begin(115200);
  ready = passport.begin();
  if (!ready) Serial.println(passport.lastError());
}

void loop() {
  if (!ready) { delay(100); return; }
  passport.update();
  if (millis() - changedAt >= 1000) {
    changedAt = millis();
    passport.display.fillScreen(colors[colorIndex]);
    colorIndex = (colorIndex + 1) % (sizeof(colors) / sizeof(colors[0]));
    passport.display.drawRect(0, 0, passport.display.width(), passport.display.height(), ST77XX_BLACK);
  }
  delay(5);
}
