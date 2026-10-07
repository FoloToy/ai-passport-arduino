// SPDX-License-Identifier: MIT
#pragma once
#include <sdkconfig.h>
#if !defined(ARDUINO_ARCH_ESP32) || !defined(CONFIG_IDF_TARGET_ESP32C3)
#error "FoloToy AI Passport requires an ESP32-C3 board and Arduino-ESP32 3.x."
#endif
#include <esp_arduino_version.h>
#if ESP_ARDUINO_VERSION_MAJOR < 3
#error "FoloToy AI Passport requires Arduino-ESP32 3.x."
#endif
#include "PassportDisplay.h"
#include "PassportButtons.h"
#include "PassportBattery.h"
#include "PassportAudio.h"

namespace folotoy {
struct PassportConfig {
  bool display = true;
  bool buttons = true;
  bool battery = false;
  bool audio = false;
  uint8_t brightness = 50;
  uint32_t sampleRate = 16000;
};
class AIPassport {
 public:
  bool begin(const PassportConfig &config = PassportConfig());
  void update() { buttons.update(); }
  void end();
  const char *lastError() const { return error_; }
  PassportDisplay display;
  PassportButtons buttons;
  PassportBattery battery;
  PassportAudio audio;
  AIPassport() = default;
  AIPassport(const AIPassport &) = delete;
  AIPassport &operator=(const AIPassport &) = delete;
 private:
  const char *error_ = "board:not-initialized";
  bool ready_ = false;
};
}  // namespace folotoy
