// SPDX-License-Identifier: MIT
#include <FoloToyAIPassport.h>

folotoy::AIPassport passport;

void setup() {
  Serial.begin(115200);
  if (!passport.begin()) {
    Serial.println(passport.lastError());
    return;
  }
  passport.display.setTextColor(ST77XX_WHITE);
  passport.display.setTextSize(2);
  passport.display.setCursor(16, 24);
  passport.display.println("Hello, Passport!");
  passport.display.setTextSize(1);
  passport.display.println("Press UP, DOWN or OK.");
}

void loop() {
  passport.update();
  if (passport.buttons.wasClicked(folotoy::Button::Ok)) {
    passport.display.fillRect(16, 80, 210, 24, ST77XX_BLACK);
    passport.display.setCursor(16, 80);
    passport.display.println("OK clicked");
  }
  delay(5);
}
