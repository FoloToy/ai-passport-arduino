// SPDX-License-Identifier: MIT
#include <FoloToyAIPassport.h>
#include <math.h>

folotoy::AIPassport passport;
bool ready = false;
uint32_t phase = 0;

void setup() {
  Serial.begin(115200);
  folotoy::PassportConfig config;
  config.display = false;
  config.audio = true;
  ready = passport.begin(config);
  if (!ready) Serial.println(passport.lastError());
  // 20% output volume. A low-amplitude 1 kHz sine wave plays continuously.
  if (ready && !passport.audio.setVolume(20)) { Serial.println(passport.audio.lastError()); ready = false; }
}

void loop() {
  if (!ready) { delay(100); return; }
  int16_t samples[128];
  for (size_t i = 0; i < sizeof(samples) / sizeof(samples[0]); ++i) {
    samples[i] = static_cast<int16_t>(2000 * sinf(2.0f * PI * (phase % 16) / 16.0f));
    ++phase;
  }
  if (passport.audio.write(samples, sizeof(samples) / sizeof(samples[0])) != sizeof(samples) / sizeof(samples[0])) {
    Serial.println(passport.audio.lastError());
    ready = false;
    passport.audio.end();
  }
  passport.update();
}
