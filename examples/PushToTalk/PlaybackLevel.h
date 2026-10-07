// SPDX-License-Identifier: MIT
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <math.h>

struct PlaybackLevel {
  uint32_t inputPeak = 0, outputPeak = 0;
  float gain = 1.0f;
};

// Level a completed RAM note, never the live capture stream. Remove DC, target
// 20000/32768 peak with at most 8x boost, and avoid boosting near-silence.
inline PlaybackLevel levelPlayback(int16_t *samples, size_t count) {
  PlaybackLevel result;
  if (!samples || !count) return result;
  int64_t sum = 0;
  for (size_t i = 0; i < count; ++i) sum += samples[i];
  const int32_t mean = static_cast<int32_t>(sum / static_cast<int64_t>(count));
  uint64_t energy = 0;
  for (size_t i = 0; i < count; ++i) {
    const int32_t sample = static_cast<int32_t>(samples[i]) - mean;
    const uint32_t magnitude = sample < 0 ? -sample : sample;
    if (magnitude > result.inputPeak) result.inputPeak = magnitude;
    energy += static_cast<int64_t>(sample) * sample;
  }
  uint32_t gainQ8 = 256;
  if (result.inputPeak) {
    gainQ8 = 20000UL * 256UL / result.inputPeak;
    if (gainQ8 > 2048) gainQ8 = 2048;
    // Background hiss should not become a loud note.
    if (sqrt(static_cast<double>(energy) / count) < 80 && gainQ8 > 256)
      gainQ8 = 256;
  }
  result.gain = gainQ8 / 256.0f;
  for (size_t i = 0; i < count; ++i) {
    int32_t sample = (static_cast<int32_t>(samples[i]) - mean) *
                     static_cast<int32_t>(gainQ8) / 256;
    if (sample > 32767) sample = 32767;
    if (sample < -32768) sample = -32768;
    samples[i] = static_cast<int16_t>(sample);
    const uint32_t magnitude = sample < 0 ? -sample : sample;
    if (magnitude > result.outputPeak) result.outputPeak = magnitude;
  }
  return result;
}
