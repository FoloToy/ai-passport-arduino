// SPDX-License-Identifier: MIT
#include <FoloToyAIPassport.h>

folotoy::AIPassport passport;
bool ready = false;
uint32_t reportedAt = 0;
uint32_t peak = 0;

void setup() {
  Serial.begin(115200);
  folotoy::PassportConfig config;
  config.display = false;
  config.audio = true;
  ready = passport.begin(config);
  if (!ready) Serial.println(passport.lastError());
  if (ready && !passport.audio.setVolume(0)) { Serial.println(passport.audio.lastError()); ready = false; }
}

void loop() {
  if (!ready) { delay(100); return; }
  int16_t samples[128];
  const size_t count = passport.audio.read(samples, sizeof(samples) / sizeof(samples[0]));
  for (size_t i = 0; i < count; ++i) {
    const int32_t sample = samples[i];
    const uint32_t magnitude = sample < 0 ? -sample : sample;
    if (magnitude > peak) peak = magnitude;
  }
  if (count != sizeof(samples) / sizeof(samples[0])) {
    Serial.println(passport.audio.lastError());
    ready = false;
    passport.audio.end();
    return;
  }
  if (millis() - reportedAt >= 500) {
    reportedAt = millis();
    Serial.printf("Microphone peak: %lu\n", (unsigned long)peak);
    peak = 0;
  }
  passport.update();
}
