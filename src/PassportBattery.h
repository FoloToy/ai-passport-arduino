// SPDX-License-Identifier: MIT
#pragma once
#include <Wire.h>
#include "PassportPins.h"

namespace folotoy {
// Read-only gauge access. Never installs or overwrites a battery profile.
class PassportBattery {
 public:
  bool begin(TwoWire &wire = Wire);
  int percent();       // 0..100; -1 on I2C failure or invalid/not-ready SOC.
  int millivolts();    // -1 on I2C failure.
  int version();      // -1 on I2C failure.
 private:
  bool read(uint8_t reg, uint8_t *data, size_t length);
  TwoWire *wire_ = nullptr;
};
}  // namespace folotoy
