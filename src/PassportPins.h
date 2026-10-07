// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>

namespace folotoy {
// AI Passport ESP32-C3 board. Pin values are GPIO numbers.
struct PassportPins {
  static constexpr int lcdMosi = 9;
  static constexpr int lcdSclk = 8;
  static constexpr int lcdCs = 1;
  static constexpr int lcdDc = 20;
  static constexpr int lcdReset = -1;  // Software reset; no GPIO reset line.
  static constexpr int backlight = 21;
  static constexpr uint16_t width = 240;
  static constexpr uint16_t height = 320;
  static constexpr uint32_t displayFrequency = 40000000;
  static constexpr int buttons = 0;
  static constexpr int sda = 10;
  static constexpr int scl = 7;
  static constexpr uint32_t i2cFrequency = 100000;
  static constexpr uint8_t codecAddress = 0x18;
  static constexpr uint8_t batteryAddress = 0x63;
  static constexpr int i2sMclk = 6;
  static constexpr int i2sBclk = 5;
  static constexpr int i2sWs = 3;
  static constexpr int i2sDout = 2;
  static constexpr int i2sDin = 4;
};
}  // namespace folotoy
