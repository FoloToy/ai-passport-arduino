// SPDX-License-Identifier: MIT
#include "PassportButtons.h"
namespace folotoy {
void PassportButtons::begin(uint16_t debounceMs, uint16_t longPressMs) {
  pinMode(PassportPins::buttons, INPUT);  // External resistor ladder, no pull-up.
  analogReadResolution(12);
  analogSetPinAttenuation(PassportPins::buttons, ADC_11db);
  state_.reset(millis(), debounceMs, longPressMs);
  ready_ = true;
}
void PassportButtons::update() {
  if (!ready_) return;
  millivolts_ = analogReadMilliVolts(PassportPins::buttons);
  state_.update(detail::decodeButton(millivolts_), millis());
}
}  // namespace folotoy
