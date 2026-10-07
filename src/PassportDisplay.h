// SPDX-License-Identifier: MIT
#pragma once
#include <Adafruit_ST7789.h>
#include "PassportPins.h"

namespace folotoy {
class PassportDisplay : public Adafruit_ST7789 {
 public:
  explicit PassportDisplay(SPIClass &spi = SPI);
  bool beginDisplay(uint8_t brightness = 50);
  void end();
  void setRotation(uint8_t rotation) override;
  void setBrightness(uint8_t percent);
  // Match the main firmware's 30 px rounded viewport, without a framebuffer.
  void fillRoundedScreen(uint16_t color);
  // Call after a frame that draws into the corners. Raw SPI writes bypass this.
  void applyCornerMask();
  bool isReady() const { return ready_; }
  PassportDisplay(const PassportDisplay &) = delete;
  PassportDisplay &operator=(const PassportDisplay &) = delete;
 private:
  SPIClass &spi_;
  bool ready_ = false;
};
}  // namespace folotoy
