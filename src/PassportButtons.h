// SPDX-License-Identifier: MIT
#pragma once
#include <Arduino.h>
#include "PassportPins.h"
#include "detail/ButtonState.h"

namespace folotoy {
class PassportButtons {
 public:
  void begin(uint16_t debounceMs = 25, uint16_t longPressMs = 500);
  void update();
  void end() { ready_ = false; state_.reset(0, 25, 500); }
  uint32_t millivolts() const { return millivolts_; }
  Button held() const { return state_.held(); }
  bool isPressed(Button button) const { return button != Button::None && held() == button; }
  bool wasPressed(Button button) const { return button != Button::None && state_.pressed() == button; }
  bool wasReleased(Button button) const { return button != Button::None && state_.released() == button; }
  bool wasClicked(Button button) const { return button != Button::None && state_.clicked() == button; }
  bool wasLongPressed(Button button) const { return button != Button::None && state_.longPressed() == button; }
 private:
  detail::ButtonState state_;
  uint32_t millivolts_ = 0;
  bool ready_ = false;
};
}  // namespace folotoy
