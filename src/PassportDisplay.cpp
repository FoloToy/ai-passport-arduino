// SPDX-License-Identifier: MIT
#include "PassportDisplay.h"
#include <driver/gpio.h>

namespace folotoy {
PassportDisplay::PassportDisplay(SPIClass &spi)
    : Adafruit_ST7789(&spi, PassportPins::lcdCs, PassportPins::lcdDc,
                      PassportPins::lcdReset), spi_(spi) {}

bool PassportDisplay::beginDisplay(uint8_t brightness) {
  if (ready_) return true;
  const int pins[] = {PassportPins::lcdCs, PassportPins::lcdSclk,
                      PassportPins::lcdMosi, PassportPins::lcdDc, PassportPins::backlight};
  gpio_deep_sleep_hold_dis();
  for (int pin : pins) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, pin == PassportPins::lcdCs ? HIGH : LOW);
    gpio_hold_dis(static_cast<gpio_num_t>(pin));
  }
  spi_.begin(PassportPins::lcdSclk, -1, PassportPins::lcdMosi, PassportPins::lcdCs);
  // Wake the panel before software reset, including after deep sleep.
  initSPI(PassportPins::displayFrequency, SPI_MODE0);
  sendCommand(0x11);
  delay(120);
  init(PassportPins::width, PassportPins::height, SPI_MODE0);
  setSPISpeed(PassportPins::displayFrequency);
  // Panel-specific porch, power and gamma settings from the AI Passport BSP.
  struct Command { uint8_t code; uint8_t data[16]; uint8_t count; };
  Command commands[] = {
    {0xB2, {0x05,0x05,0x00,0x33,0x33}, 5}, {0xB7, {0x35}, 1},
    {0xBB, {0x21}, 1}, {0xC0, {0x2C}, 1}, {0xC2, {0x01}, 1},
    {0xC3, {0x0B}, 1}, {0xC4, {0x20}, 1}, {0xC6, {0x0F}, 1},
    {0xD0, {0xA7,0xA1}, 2}, {0xD0, {0xA4,0xA1}, 2}, {0xD6, {0xA1}, 1},
    {0xE0, {0xD0,0x04,0x08,0x0A,0x09,0x05,0x2D,0x43,0x49,0x09,0x16,0x15,0x26,0x2B}, 14},
    {0xE1, {0xD0,0x03,0x09,0x0A,0x0A,0x06,0x2E,0x44,0x40,0x3A,0x15,0x15,0x26,0x2A}, 14}
  };
  for (auto &command : commands) sendCommand(command.code, command.data, command.count);
  delay(10);
  // The portrait panel uses RGB order and no X/Y mirroring.
  setRotation(0);
  invertDisplay(true);
  enableDisplay(true);
  fillScreen(ST77XX_BLACK);
  if (!ledcAttach(PassportPins::backlight, 5000, 10)) return false;
  ready_ = true;
  setBrightness(brightness);
  return true;
}
void PassportDisplay::setRotation(uint8_t requested) {
  // The board's native portrait has no mirror bits; Adafruit calls it rotation 2.
  // Keep the public rotation and dimensions consistent with native portrait = 0.
  const uint8_t logical = requested & 3;
  Adafruit_ST7789::setRotation((logical + 2) & 3);
  rotation = logical;
}
void PassportDisplay::setBrightness(uint8_t percent) {
  if (!ready_) return;
  if (percent > 100) percent = 100;
  ledcWrite(PassportPins::backlight, 1023UL * percent / 100UL);
}
void PassportDisplay::fillRoundedScreen(uint16_t color) {
  fillScreen(color);
  applyCornerMask();
}
void PassportDisplay::applyCornerMask() {
  // Only 60 short scan lines; no full-screen buffer on the no-PSRAM C3.
  const int16_t radius = 30;
  startWrite();
  for (int16_t y = 0; y < radius; ++y) {
    const int16_t dy = radius - y;
    int16_t inset = 0;
    while ((radius - inset) * (radius - inset) + dy * dy > radius * radius)
      ++inset;
    if (!inset) continue;
    writeFastHLine(0, y, inset, ST77XX_BLACK);
    writeFastHLine(width() - inset, y, inset, ST77XX_BLACK);
    writeFastHLine(0, height() - 1 - y, inset, ST77XX_BLACK);
    writeFastHLine(width() - inset, height() - 1 - y, inset, ST77XX_BLACK);
  }
  endWrite();
}
void PassportDisplay::end() {
  if (!ready_) return;
  setBrightness(0);
  enableDisplay(false);
  ledcDetach(PassportPins::backlight);
  digitalWrite(PassportPins::backlight, LOW);
  ready_ = false;
}
}  // namespace folotoy
