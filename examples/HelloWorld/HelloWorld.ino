// SPDX-License-Identifier: MIT
#include <FoloToyAIPassport.h>

folotoy::AIPassport passport;
bool ready = false;
constexpr uint16_t background = 0x0864;
constexpr uint16_t ink = 0xEF7D;
constexpr uint16_t accent = 0x67D9;

void setup() {
  Serial.begin(115200);
  ready = passport.begin();
  if (!ready) { Serial.println(passport.lastError()); return; }
  auto &d = passport.display;
  d.fillRoundedScreen(background);
  d.setTextWrap(false);
  d.setTextColor(accent);
  d.setTextSize(1);
  d.setCursor(24, 28);
  d.print("FOLOTOY / ARDUINO");
  d.setTextColor(ink);
  d.setTextSize(3);
  d.setCursor(24, 65);
  d.print("Hello,");
  d.setCursor(24, 94);
  d.print("Passport!");
  d.fillRoundRect(18, 145, 204, 96, 18, 0x1106);
  d.drawRoundRect(18, 145, 204, 96, 18, 0x21A9);
  d.setTextSize(2);
  d.setCursor(36, 167);
  d.print("Let's build.");
  d.setTextSize(1);
  d.setTextColor(accent);
  d.setCursor(36, 205);
  d.print("Display  Audio  Buttons");
  d.setTextColor(ink);
  d.setCursor(24, 270);
  d.print("Press UP, DOWN or OK");
  d.applyCornerMask();
}

void loop() {
  if (!ready) { delay(100); return; }
  passport.update();
  const char *key = nullptr;
  if (passport.buttons.wasPressed(folotoy::Button::Up)) key = "UP pressed";
  if (passport.buttons.wasPressed(folotoy::Button::Down)) key = "DOWN pressed";
  if (passport.buttons.wasPressed(folotoy::Button::Ok)) key = "OK pressed";
  if (key) {
    passport.display.fillRect(36, 167, 170, 20, 0x1106);
    passport.display.setTextSize(2);
    passport.display.setTextColor(ink);
    passport.display.setCursor(36, 167);
    passport.display.print(key);
    Serial.println(key);
  }
  delay(5);
}
