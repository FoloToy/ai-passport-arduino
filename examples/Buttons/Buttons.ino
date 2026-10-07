// SPDX-License-Identifier: MIT
#include <FoloToyAIPassport.h>

folotoy::AIPassport passport;
const folotoy::Button keys[] = {folotoy::Button::Up, folotoy::Button::Down, folotoy::Button::Ok};
const char *names[] = {"UP", "DOWN", "OK"};

void setup() {
  Serial.begin(115200);
  folotoy::PassportConfig config;
  config.display = false;
  if (!passport.begin(config)) Serial.println(passport.lastError());
}

void loop() {
  passport.update();
  for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i) {
    if (passport.buttons.wasPressed(keys[i])) Serial.printf("%s pressed (%lu mV)\n", names[i], (unsigned long)passport.buttons.millivolts());
    if (passport.buttons.wasClicked(keys[i])) Serial.printf("%s clicked\n", names[i]);
    if (passport.buttons.wasLongPressed(keys[i])) Serial.printf("%s long press\n", names[i]);
    if (passport.buttons.wasReleased(keys[i])) Serial.printf("%s released\n", names[i]);
  }
  delay(5);
}
