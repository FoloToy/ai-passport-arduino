// SPDX-License-Identifier: MIT
#pragma once
#include <Arduino.h>
#include <Wire.h>

namespace folotoy {
// Signed 16-bit mono PCM; two I2S slots are handled inside the driver.
// Blocking I/O; serialize all calls in one task.
class PassportAudio {
 public:
  PassportAudio() = default;
  ~PassportAudio();
  bool begin(TwoWire &wire = Wire, uint32_t sampleRate = 16000);
  void end();
  bool setVolume(uint8_t percent);
  bool setMicrophoneGain(float decibels);
  size_t write(const int16_t *samples, size_t count, uint32_t timeoutMs = 1000);
  size_t read(int16_t *samples, size_t count, uint32_t timeoutMs = 1000);
  const char *lastError() const { return error_; }
  bool isReady() const { return ready_; }
  uint32_t sampleRate() const { return sampleRate_; }
  PassportAudio(const PassportAudio &) = delete;
  PassportAudio &operator=(const PassportAudio &) = delete;
 private:
  struct State;
  State *state_ = nullptr;
  const char *error_ = "audio:not-initialized";
  uint32_t sampleRate_ = 0;
  bool ready_ = false;
};
}  // namespace folotoy
