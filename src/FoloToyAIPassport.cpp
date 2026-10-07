// SPDX-License-Identifier: MIT
#include "FoloToyAIPassport.h"
namespace folotoy {
bool AIPassport::begin(const PassportConfig &config) {
  if (ready_) return true;
  if (config.display && !display.begin(config.brightness)) {
    error_ = "display:backlight-init-failed"; return false;
  }
  if (config.buttons) buttons.begin();
  if (config.battery || config.audio) {
    if (!Wire.begin(PassportPins::sda, PassportPins::scl, PassportPins::i2cFrequency)) {
      error_ = "i2c:bus-init-failed"; end(); return false;
    }
    Wire.setTimeOut(100);
  }
  if (config.battery && !battery.begin(Wire)) {
    error_ = "battery:gauge-not-found"; end(); return false;
  }
  if (config.audio && !audio.begin(Wire, config.sampleRate)) {
    error_ = audio.lastError(); end(); return false;
  }
  ready_ = true;
  error_ = "ok";
  return true;
}
void AIPassport::end() {
  audio.end();
  buttons.end();
  display.end();
  ready_ = false;
  // Shared Arduino buses remain owned by the sketch; do not call Wire.end().
}
}  // namespace folotoy
