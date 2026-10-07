// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
struct AudioStats {
  uint64_t energy = 0;
  double real = 0, imaginary = 0;
  uint32_t count = 0, clipped = 0;
};
