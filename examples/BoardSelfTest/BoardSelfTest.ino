// SPDX-License-Identifier: MIT
#include <FoloToyAIPassport.h>
#include "AudioStats.h"
#include <WiFi.h>
#include <math.h>

folotoy::AIPassport passport;
bool boardReady = false;
bool busReady = false;
uint32_t keyCounts[3] = {};
const folotoy::Button keys[] = {folotoy::Button::Up, folotoy::Button::Down, folotoy::Button::Ok};
const char *keyNames[] = {"UP", "DOWN", "OK"};
char command[32] = {};
size_t commandLength = 0;

void updateButtons() {
  passport.update();
  for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i) {
    if (passport.buttons.wasPressed(keys[i])) {
      ++keyCounts[i];
      Serial.printf("TEST button=%s event=press mv=%lu count=%lu\n", keyNames[i],
                    (unsigned long)passport.buttons.millivolts(), (unsigned long)keyCounts[i]);
    }
    if (passport.buttons.wasClicked(keys[i])) Serial.printf("TEST button=%s event=click\n", keyNames[i]);
    if (passport.buttons.wasLongPressed(keys[i])) Serial.printf("TEST button=%s event=long\n", keyNames[i]);
    if (passport.buttons.wasReleased(keys[i])) Serial.printf("TEST button=%s event=release\n", keyNames[i]);
  }
}

void displayTest() {
  passport.display.fillScreen(ST77XX_BLACK);
  const uint16_t width = passport.display.width();
  const uint16_t height = passport.display.height();
  passport.display.fillRect(0, 0, width, height / 3, ST77XX_RED);
  passport.display.fillRect(0, height / 3, width, height / 3, ST77XX_GREEN);
  passport.display.fillRect(0, 2 * height / 3, width, height - 2 * height / 3, ST77XX_BLUE);
  passport.display.drawRect(0, 0, width, height, ST77XX_WHITE);
  passport.display.setTextColor(ST77XX_WHITE);
  passport.display.setTextSize(2);
  passport.display.setCursor(12, 18);
  passport.display.println("AI Passport");
  passport.display.setCursor(12, 50);
  passport.display.println("Arduino test");
  passport.display.setBrightness(50);
  Serial.printf("TEST display=issued width=%u height=%u\n", width, height);
}

bool audioBegin(uint32_t rate) {
  if (!busReady || !passport.audio.begin(Wire, rate)) {
    Serial.printf("TEST audio=FAIL error=%s\n", passport.audio.lastError());
    return false;
  }
  Serial.printf("TEST audio=PASS rate=%lu heap=%lu\n", (unsigned long)rate, (unsigned long)ESP.getFreeHeap());
  return true;
}

void microphoneTest() {
  if (!audioBegin(16000) || !passport.audio.setVolume(0)) return;
  int16_t samples[128];
  uint64_t energy = 0;
  uint32_t total = 0, peak = 0;
  const uint32_t started = millis();
  while (millis() - started < 4000) {
    const size_t count = passport.audio.read(samples, sizeof(samples) / sizeof(samples[0]), 200);
    if (count != sizeof(samples) / sizeof(samples[0])) {
      Serial.printf("TEST mic=FAIL error=%s\n", passport.audio.lastError());
      return;
    }
    for (size_t i = 0; i < count; ++i) {
      const int32_t value = samples[i];
      const uint32_t magnitude = value < 0 ? -value : value;
      if (magnitude > peak) peak = magnitude;
      energy += static_cast<int64_t>(value) * value;
    }
    total += count;
    updateButtons();
  }
  Serial.printf("TEST mic=%s samples=%lu peak=%lu rms=%.2f\n", peak ? "NONZERO" : "ZERO",
                (unsigned long)total, (unsigned long)peak, sqrt(static_cast<double>(energy) / total));
}

void toneTest() {
  if (!audioBegin(16000) || !passport.audio.setVolume(20)) return;
  int16_t samples[128];
  uint32_t phase = 0;
  uint32_t total = 0;
  while (total < 32000) {
    for (size_t i = 0; i < sizeof(samples) / sizeof(samples[0]); ++i) {
      samples[i] = static_cast<int16_t>(4000.0f * sinf(2.0f * PI * (phase % 16) / 16.0f));
      ++phase;
    }
    const size_t count = passport.audio.write(samples, sizeof(samples) / sizeof(samples[0]), 200);
    if (count != sizeof(samples) / sizeof(samples[0])) {
      Serial.printf("TEST tone=FAIL error=%s\n", passport.audio.lastError());
      return;
    }
    total += count;
    updateButtons();
  }
  passport.audio.setVolume(0);
  Serial.printf("TEST tone=DELIVERED samples=%lu hz=1000\n", (unsigned long)total);
}

void cycleTest() {
  passport.audio.end();
  const uint32_t before = ESP.getFreeHeap();
  const uint32_t rates[] = {8000, 16000, 32000, 44100, 48000};
  for (uint32_t rate : rates) {
    for (int repeat = 0; repeat < 3; ++repeat) {
      if (!audioBegin(rate)) return;
      int16_t zero[128] = {};
      const size_t count = passport.audio.write(zero, sizeof(zero) / sizeof(zero[0]), 200);
      if (count != sizeof(zero) / sizeof(zero[0])) {
        Serial.printf("TEST cycles=FAIL error=%s\n", passport.audio.lastError());
        passport.audio.end(); return;
      }
      passport.audio.end();
      updateButtons();
    }
  }
  Serial.printf("TEST cycles=PASS iterations=15 heapBefore=%lu heapAfter=%lu\n",
                (unsigned long)before, (unsigned long)ESP.getFreeHeap());
}

void graphicsTest() {
  for (uint8_t rotation = 0; rotation < 4; ++rotation) {
    passport.display.setRotation(rotation);
    const int expectedWidth = rotation & 1 ? 320 : 240;
    const int expectedHeight = rotation & 1 ? 240 : 320;
    if (passport.display.width() != expectedWidth || passport.display.height() != expectedHeight ||
        passport.display.getRotation() != rotation) {
      Serial.println("TEST graphics=FAIL reason=rotation-dimensions"); return;
    }
    displayTest();
    passport.display.setCursor(12, 90);
    passport.display.printf("Rotation %u", rotation);
    delay(600);
  }
  passport.display.setRotation(0);
  displayTest();
  const uint8_t levels[] = {0, 10, 50, 100};
  for (uint8_t level : levels) { passport.display.setBrightness(level); delay(600); }
  passport.display.setBrightness(50);
  Serial.println("TEST graphics=ISSUED rotations=4 brightness=0,10,50,100 origin=0");
}

void boardCycleTest() {
  passport.audio.end();
  const uint32_t before = ESP.getFreeHeap();
  for (int i = 0; i < 10; ++i) {
    passport.end();
    if (!passport.begin()) { Serial.printf("TEST boardcycles=FAIL error=%s\n", passport.lastError()); return; }
    if (!passport.display.isReady()) { Serial.println("TEST boardcycles=FAIL reason=display-state"); return; }
  }
  displayTest();
  Serial.printf("TEST boardcycles=PASS iterations=10 heapBefore=%lu heapAfter=%lu\n",
                (unsigned long)before, (unsigned long)ESP.getFreeHeap());
}

void negativeTest() {
  folotoy::PassportAudio uninitialized;
  int16_t samples[8] = {};
  const bool readRejected = uninitialized.read(samples, 8, 10) == 0;
  const bool writeRejected = uninitialized.write(samples, 8, 10) == 0;
  const bool volumeRejected = !uninitialized.setVolume(50);
  const bool gainRejected = !uninitialized.setMicrophoneGain(12);
  const bool rateRejected = !uninitialized.begin(Wire, 12345);
  if (!audioBegin(16000)) return;
  const bool nullReadRejected = passport.audio.read(nullptr, 8, 10) == 0;
  const bool nullWriteRejected = passport.audio.write(nullptr, 8, 10) == 0;
  const bool negativeGainRejected = !passport.audio.setMicrophoneGain(-1);
  const bool highGainRejected = !passport.audio.setMicrophoneGain(50);
  const bool nanGainRejected = !passport.audio.setMicrophoneGain(NAN);
  const bool zeroRead = passport.audio.read(nullptr, 0, 10) == 0;
  const bool zeroWrite = passport.audio.write(nullptr, 0, 10) == 0;
  const bool all = readRejected && writeRejected && volumeRejected && gainRejected && rateRejected &&
      nullReadRejected && nullWriteRejected && negativeGainRejected && highGainRejected && nanGainRejected &&
      zeroRead && zeroWrite;
  passport.audio.setVolume(0);
  Serial.printf("TEST negative=%s cases=12\n", all ? "PASS" : "FAIL");
}

AudioStats measureAudio(bool playTone) {
  AudioStats stats;
  int16_t output[128], input[128];
  uint32_t phase = 0;
  for (int block = 0; block < 220; ++block) {
    for (size_t i = 0; i < sizeof(output) / sizeof(output[0]); ++i) {
      output[i] = playTone ? static_cast<int16_t>(6000.0f * sinf(2.0f * PI * (phase % 16) / 16.0f)) : 0;
      ++phase;
    }
    if (passport.audio.write(output, 128, 200) != 128 || passport.audio.read(input, 128, 200) != 128) {
      Serial.printf("TEST loopback=FAIL error=%s\n", passport.audio.lastError()); stats.count = 0; return stats;
    }
    // Discard startup and DMA queue latency before comparing the acoustic signal.
    if (block >= 60) {
      for (size_t i = 0; i < sizeof(input) / sizeof(input[0]); ++i) {
        const int32_t value = input[i];
        stats.energy += static_cast<int64_t>(value) * value;
        const double angle = 2.0 * PI * (stats.count % 16) / 16.0;
        stats.real += value * cos(angle);
        stats.imaginary += value * sin(angle);
        if (value >= 32760 || value <= -32760) ++stats.clipped;
        ++stats.count;
      }
    }
    if ((block % 16) == 0) {
      const int voltage = passport.battery.millivolts();
      if (voltage < 0) { Serial.println("TEST loopback=FAIL reason=shared-i2c"); stats.count = 0; return stats; }
      passport.display.drawPixel(block % 240, 300, ST77XX_WHITE);
    }
    updateButtons();
  }
  return stats;
}
void loopbackTest() {
  if (!audioBegin(16000) || !passport.audio.setMicrophoneGain(12) || !passport.audio.setVolume(0)) return;
  AudioStats quiet = measureAudio(false);
  if (!quiet.count || !passport.audio.setVolume(40)) return;
  AudioStats tone = measureAudio(true);
  passport.audio.setVolume(0);
  if (!tone.count) return;
  const double quietBand = 2.0 * sqrt(quiet.real * quiet.real + quiet.imaginary * quiet.imaginary) / quiet.count;
  const double toneBand = 2.0 * sqrt(tone.real * tone.real + tone.imaginary * tone.imaginary) / tone.count;
  Serial.printf("TEST loopback samples=%lu quietRms=%.2f toneRms=%.2f quiet1k=%.2f tone1k=%.2f clipped=%lu\n",
      (unsigned long)tone.count, sqrt(static_cast<double>(quiet.energy) / quiet.count),
      sqrt(static_cast<double>(tone.energy) / tone.count), quietBand, toneBand, (unsigned long)tone.clipped);
  passport.audio.setMicrophoneGain(30);
}

void batteryTest() {
  const bool found = busReady && passport.battery.begin(Wire);
  const int percent = found ? passport.battery.percent() : -1;
  const int voltage = found ? passport.battery.millivolts() : -1;
  Serial.printf("TEST battery found=%d percent=%d mv=%d\n", found, percent, voltage);
}

void runCommand(const char *text) {
  if (!strcmp(text, "status")) {
    Serial.printf("TEST status ready=%d heap=%lu minHeap=%lu buttonMv=%lu up=%lu down=%lu ok=%lu\n",
                  boardReady, (unsigned long)ESP.getFreeHeap(), (unsigned long)ESP.getMinFreeHeap(),
                  (unsigned long)passport.buttons.millivolts(), (unsigned long)keyCounts[0],
                  (unsigned long)keyCounts[1], (unsigned long)keyCounts[2]);
  } else if (!strcmp(text, "display")) displayTest();
  else if (!strcmp(text, "battery")) batteryTest();
  else if (!strcmp(text, "tone")) toneTest();
  else if (!strcmp(text, "mic")) microphoneTest();
  else if (!strcmp(text, "cycles")) cycleTest();
  else if (!strcmp(text, "graphics")) graphicsTest();
  else if (!strcmp(text, "boardcycles")) boardCycleTest();
  else if (!strcmp(text, "negative")) negativeTest();
  else if (!strcmp(text, "loopback")) loopbackTest();
  else if (!strcmp(text, "stop")) { passport.audio.end(); Serial.println("TEST audio=STOPPED"); }
  else if (!strcmp(text, "wifi")) {
    passport.audio.end();
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    const int count = WiFi.scanNetworks();
    WiFi.scanDelete();
    WiFi.mode(WIFI_OFF);
    Serial.printf("TEST wifi count=%d\n", count);
  } else Serial.println("TEST commands=status,display,battery,tone,mic,cycles,graphics,boardcycles,negative,loopback,wifi,stop");
}

void setup() {
  Serial.begin(115200);
  boardReady = passport.begin();
  busReady = Wire.begin(folotoy::PassportPins::sda, folotoy::PassportPins::scl,
                        folotoy::PassportPins::i2cFrequency);
  Wire.setTimeOut(100);
  if (boardReady) displayTest();
  Serial.printf("TEST boot board=%d i2c=%d error=%s\n", boardReady, busReady, passport.lastError());
}

void loop() {
  if (boardReady) updateButtons();
  while (Serial.available()) {
    const int c = Serial.read();
    if (c == '\r') continue;
    if (c == '\n') {
      command[commandLength] = '\0';
      if (boardReady) runCommand(command);
      else Serial.println("TEST board=FAIL");
      commandLength = 0;
    } else if (commandLength < sizeof(command) - 1) command[commandLength++] = static_cast<char>(c);
    else { commandLength = 0; Serial.println("TEST command=TOO_LONG"); }
  }
  delay(5);
}
