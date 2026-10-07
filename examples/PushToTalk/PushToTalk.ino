// SPDX-License-Identifier: MIT
// Hold OK to record, release to stop. UP replays; DOWN clears/cancels.
// 16 kHz mono PCM, max 3 seconds (96 KB RAM). No flash or cloud storage.
#include <FoloToyAIPassport.h>
#include <math.h>
#include <stdlib.h>
#include "RecorderState.h"

folotoy::AIPassport passport;
constexpr uint32_t sampleRate = 16000;
constexpr size_t capacity = sampleRate * 3;
constexpr size_t chunkSize = 160;  // 10 ms; keep polling the buttons.
constexpr uint16_t background = 0x0864;
constexpr uint16_t card = 0x1106;
constexpr uint16_t ink = 0xEF7D;
constexpr uint16_t muted = 0x8D15;
constexpr uint16_t accent = 0x67D9;
constexpr uint16_t red = 0xF2CE;
int16_t *recording = nullptr;
size_t captured = 0, played = 0;
RecorderState state = RecorderState::Ready;
bool ready = false;
int batteryPercent = -1;
uint32_t startedAt = 0, drawnAt = 0, batteryAt = 0;
uint32_t peak = 0, clipped = 0;
uint64_t energy = 0;
const char *notice = "Hold OK to record";

void label(int16_t x, int16_t y, const char *text, uint8_t size,
           uint16_t color) {
  passport.display.setTextSize(size);
  passport.display.setTextColor(color);
  passport.display.setCursor(x, y);
  passport.display.print(text);
}

void drawBattery() {
  auto &d = passport.display;
  d.fillRect(157, 26, 57, 18, background);
  d.drawRoundRect(158, 28, 18, 10, 2, muted);
  d.fillRect(176, 31, 2, 4, muted);
  if (batteryPercent >= 0) {
    d.fillRect(160, 30, batteryPercent * 14 / 100, 6, accent);
    char value[8];
    snprintf(value, sizeof(value), "%d%%", batteryPercent);
    label(183, 30, value, 1, ink);
  } else {
    label(184, 30, "--", 1, muted);
  }
}

void drawScreen() {
  auto &d = passport.display;
  d.fillRoundedScreen(background);
  d.setTextWrap(false);
  label(24, 28, "FOLOTOY", 1, accent);
  drawBattery();
  label(24, 57, "Voice Note", 2, ink);
  label(24, 82, "AI PASSPORT / ARDUINO", 1, muted);
  d.fillRoundRect(18, 108, 204, 146, 18, card);
  d.drawRoundRect(18, 108, 204, 146, 18, 0x21A9);
  const bool active = state == RecorderState::Recording;
  const uint16_t iconColor = active ? red : accent;
  if (state == RecorderState::Playing) {
    d.fillTriangle(109, 124, 109, 144, 128, 134, accent);
  } else {
    d.fillRoundRect(113, 121, 14, 23, 7, iconColor);
    d.drawRoundRect(108, 132, 24, 18, 10, iconColor);
    d.drawFastVLine(120, 149, 6, iconColor);
    d.drawFastHLine(113, 155, 15, iconColor);
  }
  const char *heading = "READY";
  if (active) heading = "RECORDING";
  else if (state == RecorderState::Recorded) heading = "NOTE SAVED";
  else if (state == RecorderState::Playing) heading = "PLAYING";
  else if (state == RecorderState::Error) heading = "AUDIO ERROR";
  label((240 - static_cast<int16_t>(strlen(heading) * 6)) / 2, 169,
        heading, 1, active ? red : accent);
  label(44, 236, "16 kHz  /  MONO PCM", 1, muted);
  label(24, 266, notice, 1, ink);
  label(24, 284, "UP play   DOWN clear", 1, muted);
  d.applyCornerMask();
  drawnAt = 0;
}

void drawProgress(uint32_t level = 0) {
  if (drawnAt && millis() - drawnAt < 100) return;
  drawnAt = millis();
  auto &d = passport.display;
  d.fillRect(35, 186, 170, 40, card);
  const size_t count = state == RecorderState::Playing ? played : captured;
  char duration[20];
  snprintf(duration, sizeof(duration), "%.1f / 3.0 s", count / float(sampleRate));
  label(51, 188, duration, 2, ink);
  d.fillRoundRect(36, 215, 168, 5, 2, 0x21A9);
  const int16_t bar = state == RecorderState::Recording
      ? min(static_cast<uint32_t>(168), level * 168 / 12000)
      : static_cast<int16_t>(count * 168 / capacity);
  if (bar > 0) d.fillRoundRect(36, 215, bar, 5, 2,
                              state == RecorderState::Recording ? red : accent);
}

void audioError(const char *reason) {
  Serial.printf("RECORDER error=%s\n", reason);
  // A failed mute is reported; never silently treat a partial recording as OK.
  if (passport.audio.isReady() && !passport.audio.setVolume(0))
    Serial.println(passport.audio.lastError());
  state = RecorderState::Error;
  notice = "Audio failed; restart";
  drawScreen();
  drawProgress();
}

void beginRecording() {
  if (!passport.audio.setVolume(0)) { audioError(passport.audio.lastError()); return; }
  // The continuously enabled RX DMA contains old idle/playback samples.
  // Drop at most its backlog without waiting for new audio.
  int16_t discard[chunkSize];
  for (unsigned i = 0; i < 12; ++i)
    if (passport.audio.read(discard, chunkSize, 0) < chunkSize) break;
  captured = played = 0;
  peak = clipped = 0;
  energy = 0;
  state = RecorderState::Recording;
  notice = "Release OK to finish";
  drawScreen();
  // Discard samples captured during the initial full-screen redraw too.
  for (unsigned i = 0; i < 12; ++i)
    if (passport.audio.read(discard, chunkSize, 0) < chunkSize) break;
  startedAt = millis();
  Serial.println("RECORDER start rate=16000 maxMs=3000 gainDb=24");
}

void finishRecording(const char *reason) {
  state = captured ? RecorderState::Recorded : RecorderState::Ready;
  notice = captured ? "UP to hear your note" : "Hold OK to record";
  const double rms = captured ? sqrt(static_cast<double>(energy) / captured) : 0;
  Serial.printf("RECORDER stop reason=%s samples=%u durationMs=%u wallMs=%lu peak=%lu rms=%.2f clipped=%lu\n",
                reason, static_cast<unsigned>(captured),
                static_cast<unsigned>(captured * 1000 / sampleRate),
                static_cast<unsigned long>(millis() - startedAt),
                static_cast<unsigned long>(peak), rms,
                static_cast<unsigned long>(clipped));
  drawScreen();
  drawProgress();
}

void beginPlayback() {
  state = RecorderState::Playing;
  played = 0;
  notice = "DOWN to stop playback";
  // Draw before unmuting/filling the small audio DMA queue.
  drawScreen();
  if (!passport.audio.setVolume(65)) { audioError(passport.audio.lastError()); return; }
  Serial.printf("RECORDER playback samples=%u\n", static_cast<unsigned>(captured));
}

void finishPlayback(bool cancelled) {
  // Allow the final submitted samples to leave DMA before muting (max 90 ms).
  if (!cancelled) delay(100);
  if (!passport.audio.setVolume(0)) { audioError(passport.audio.lastError()); return; }
  state = RecorderState::Recorded;
  notice = "UP to hear your note";
  Serial.printf("RECORDER playback=%s samples=%u\n", cancelled ? "cancelled" : "complete",
                static_cast<unsigned>(played));
  drawScreen();
  drawProgress();
}

void setup() {
  Serial.begin(115200);
  folotoy::PassportConfig config;
  config.audio = true;
  config.sampleRate = sampleRate;
  ready = passport.begin(config);
  if (!ready) { Serial.println(passport.lastError()); return; }
  // Optional gauge: a missing/unconfigured battery must not disable recording.
  if (passport.battery.begin(Wire)) batteryPercent = passport.battery.percent();
  recording = static_cast<int16_t *>(malloc(capacity * sizeof(int16_t)));
  if (!recording) { audioError("record-buffer-allocation-failed"); ready = false; return; }
  if (!passport.audio.setVolume(0) || !passport.audio.setMicrophoneGain(24.0f)) {
    audioError(passport.audio.lastError()); ready = false; return;
  }
  drawScreen();
  drawProgress();
  Serial.printf("RECORDER ready bufferBytes=%u heap=%lu\n",
                static_cast<unsigned>(capacity * sizeof(int16_t)),
                static_cast<unsigned long>(ESP.getFreeHeap()));
}

void loop() {
  if (!ready || state == RecorderState::Error) { delay(20); return; }
  passport.update();
  if (state == RecorderState::Recording) {
    if (!passport.buttons.isPressed(folotoy::Button::Ok)) {
      finishRecording("released");
    } else if (captured == capacity || millis() - startedAt >= 3000) {
      finishRecording("limit");
    } else {
      const size_t count = min(chunkSize, capacity - captured);
      const size_t got = passport.audio.read(recording + captured, count, 25);
      uint32_t blockPeak = 0;
      for (size_t i = 0; i < got; ++i) {
        const int32_t sample = recording[captured + i];
        const uint32_t magnitude = sample < 0 ? -sample : sample;
        blockPeak = max(blockPeak, magnitude);
        energy += static_cast<int64_t>(sample) * sample;
        if (magnitude >= 32767) ++clipped;
      }
      captured += got;
      peak = max(peak, blockPeak);
      if (got != count) { audioError(passport.audio.lastError()); return; }
      drawProgress(blockPeak);
    }
  } else if (state == RecorderState::Playing) {
    if (passport.buttons.wasPressed(folotoy::Button::Down)) finishPlayback(true);
    else {
      const size_t count = min(chunkSize, captured - played);
      const size_t sent = passport.audio.write(recording + played, count, 25);
      played += sent;
      if (sent != count) { audioError(passport.audio.lastError()); return; }
      if (played == captured) finishPlayback(false);
      else drawProgress();
    }
  } else if (passport.buttons.wasPressed(folotoy::Button::Ok)) {
    beginRecording();
  } else if (passport.buttons.wasPressed(folotoy::Button::Up) && captured) {
    beginPlayback();
  } else if (passport.buttons.wasPressed(folotoy::Button::Down)) {
    captured = 0;
    state = RecorderState::Ready;
    notice = "Hold OK to record";
    drawScreen();
    drawProgress();
    Serial.println("RECORDER cleared");
  }
  // Keep slow/shared-I2C battery reads out of recording and playback.
  if (state != RecorderState::Recording && state != RecorderState::Playing &&
      millis() - batteryAt >= 5000) {
    batteryAt = millis();
    batteryPercent = passport.battery.percent();
    drawBattery();
  }
  delay(1);
}
