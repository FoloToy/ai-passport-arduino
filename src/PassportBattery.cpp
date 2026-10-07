// SPDX-License-Identifier: MIT
#include "PassportBattery.h"
#include "detail/ButtonState.h"
namespace folotoy {
bool PassportBattery::begin(TwoWire &wire) { wire_ = &wire; return version() >= 0; }
bool PassportBattery::read(uint8_t reg, uint8_t *data, size_t length) {
  if (!wire_ || !data || !length) return false;
  wire_->beginTransmission(PassportPins::batteryAddress);
  wire_->write(reg);
  if (wire_->endTransmission(false) != 0) return false;
  const size_t received = wire_->requestFrom(PassportPins::batteryAddress, length, true);
  if (received != length) {
    while (wire_->available()) wire_->read();
    return false;
  }
  for (size_t i = 0; i < length; ++i) data[i] = wire_->read();
  return true;
}
int PassportBattery::version() { uint8_t value; return read(0, &value, 1) ? value : -1; }
int PassportBattery::percent() {
  uint8_t data[2];
  return read(4, data, sizeof(data)) && data[0] <= 100 ? data[0] : -1;
}
int PassportBattery::millivolts() {
  uint8_t data[2];
  return read(2, data, sizeof(data)) ? detail::batteryMillivolts(data[0], data[1]) : -1;
}
}  // namespace folotoy
