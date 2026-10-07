// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
enum class RecorderState : uint8_t { Ready, Recording, Recorded, Playing, Error };
