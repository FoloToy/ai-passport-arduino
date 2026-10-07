// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>

namespace folotoy {
enum class Button : uint8_t { None, Up, Down, Ok };

namespace detail {
inline Button decodeButton(uint32_t mv) {
  if (mv < 150) return Button::Up;
  if (mv < 447) return Button::Down;
  if (mv < 1900) return Button::Ok;
  return Button::None;
}
inline int batteryMillivolts(uint8_t high, uint8_t low) {
  const uint32_t raw = ((static_cast<uint32_t>(high) << 8) | low) & 0x3FFF;
  return static_cast<int>(raw * 3125UL / 10000UL);
}

// Clock differences use unsigned arithmetic so millis() wrap is harmless.
class ButtonState {
 public:
  void reset(uint32_t now, uint16_t debounceMs, uint16_t longPressMs) {
    stable_ = candidate_ = Button::None;
    candidateSince_ = pressedSince_ = now;
    debounceMs_ = debounceMs;
    longPressMs_ = longPressMs;
    longSent_ = false;
    clearEvents();
  }
  void update(Button raw, uint32_t now) {
    clearEvents();
    if (raw != candidate_) { candidate_ = raw; candidateSince_ = now; }
    if (candidate_ != stable_ && uint32_t(now - candidateSince_) >= debounceMs_) {
      const Button previous = stable_;
      if (previous != Button::None) {
        released_ = previous;
        // A long hold is never also a click, even with sparse polling.
        if (!longSent_ && uint32_t(now - pressedSince_) < longPressMs_)
          clicked_ = previous;
      }
      stable_ = candidate_;
      if (stable_ != Button::None) {
        pressed_ = stable_;
        pressedSince_ = now;
        longSent_ = false;
      }
    }
    if (stable_ != Button::None && !longSent_ &&
        uint32_t(now - pressedSince_) >= longPressMs_) {
      longPressed_ = stable_;
      longSent_ = true;
    }
  }
  Button held() const { return stable_; }
  Button pressed() const { return pressed_; }
  Button released() const { return released_; }
  Button clicked() const { return clicked_; }
  Button longPressed() const { return longPressed_; }
 private:
  void clearEvents() { pressed_ = released_ = clicked_ = longPressed_ = Button::None; }
  Button stable_ = Button::None, candidate_ = Button::None;
  Button pressed_ = Button::None, released_ = Button::None;
  Button clicked_ = Button::None, longPressed_ = Button::None;
  uint32_t candidateSince_ = 0, pressedSince_ = 0;
  uint16_t debounceMs_ = 25, longPressMs_ = 500;
  bool longSent_ = false;
};
}  // namespace detail
}  // namespace folotoy
