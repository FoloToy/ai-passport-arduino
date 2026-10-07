// SPDX-License-Identifier: MIT
#pragma once
#include <Adafruit_ST7789.h>
#include "PassportPins.h"

namespace folotoy {
class PassportDisplay : public Adafruit_ST7789 {
 public:
  explicit PassportDisplay(SPIClass &spi = SPI);
  bool begin(uint8_t brightness = 50);
  void end();
  void setBrightness(uint8_t percent);
  bool isReady() const { return ready_; }
  PassportDisplay(const PassportDisplay &) = delete;
  PassportDisplay &operator=(const PassportDisplay &) = delete;
 private:
  SPIClass &spi_;
  bool ready_ = false;
};
}  // namespace folotoy
