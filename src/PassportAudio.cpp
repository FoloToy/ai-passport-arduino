// SPDX-License-Identifier: MIT
#include "PassportAudio.h"
#include "PassportPins.h"
#include "vendor/esp_codec_dev/es8311_codec.h"
#include <driver/i2s_std.h>
#include <math.h>
#include <new>

namespace folotoy {
namespace {
struct WireControl {
  audio_codec_ctrl_if_t base = {};
  TwoWire *wire = nullptr;
  bool failed = false;
};
WireControl *control(const audio_codec_ctrl_if_t *base) {
  return reinterpret_cast<WireControl *>(const_cast<audio_codec_ctrl_if_t *>(base));
}
bool ctrlIsOpen(const audio_codec_ctrl_if_t *base) { return control(base)->wire != nullptr; }
int ctrlRead(const audio_codec_ctrl_if_t *base, int reg, int regLength, void *data, int length) {
  auto *ctrl = control(base);
  if (!data || regLength != 1 || length <= 0 || length > 32) return ESP_CODEC_DEV_INVALID_ARG;
  memset(data, 0, length);  // Upstream may inspect a register after an I2C failure.
  auto &wire = *ctrl->wire;
  wire.beginTransmission(PassportPins::codecAddress);
  wire.write(static_cast<uint8_t>(reg));
  if (wire.endTransmission(false) == 0 &&
      wire.requestFrom(PassportPins::codecAddress, static_cast<size_t>(length), true) == static_cast<size_t>(length)) {
    for (int i = 0; i < length; ++i) static_cast<uint8_t *>(data)[i] = wire.read();
    return ESP_CODEC_DEV_OK;
  }
  while (wire.available()) wire.read();
  ctrl->failed = true;
  return ESP_CODEC_DEV_READ_FAIL;
}
int ctrlWrite(const audio_codec_ctrl_if_t *base, int reg, int regLength, void *data, int length) {
  auto *ctrl = control(base);
  if (!data || regLength != 1 || length <= 0 || length > 32) return ESP_CODEC_DEV_INVALID_ARG;
  auto &wire = *ctrl->wire;
  wire.beginTransmission(PassportPins::codecAddress);
  wire.write(static_cast<uint8_t>(reg));
  wire.write(static_cast<const uint8_t *>(data), length);
  if (wire.endTransmission() == 0) return ESP_CODEC_DEV_OK;
  ctrl->failed = true;
  return ESP_CODEC_DEV_WRITE_FAIL;
}
int ctrlInfo(const audio_codec_ctrl_if_t *, audio_codec_ctrl_info_t *info) {
  if (!info) return ESP_CODEC_DEV_INVALID_ARG;
  memset(info, 0, sizeof(*info));
  info->type = AUDIO_CODEC_CTRL_I2C;
  info->i2c.addr = PassportPins::codecAddress << 1;
  info->i2c.port = 0;
  return ESP_CODEC_DEV_OK;
}
bool supportedRate(uint32_t rate) {
  return rate == 8000 || rate == 16000 || rate == 32000 || rate == 44100 || rate == 48000;
}
}  // namespace

struct PassportAudio::State {
  WireControl ctrl;
  const audio_codec_if_t *codec = nullptr;
  i2s_chan_handle_t tx = nullptr, rx = nullptr;
  bool txEnabled = false, rxEnabled = false;
};

PassportAudio::~PassportAudio() { end(); }
bool PassportAudio::begin(TwoWire &wire, uint32_t sampleRate) {
  if (ready_ && sampleRate_ == sampleRate) return true;
  if (!supportedRate(sampleRate)) { error_ = "audio:unsupported-sample-rate"; return false; }
  end();
  state_ = new (std::nothrow) State;
  if (!state_) { error_ = "audio:allocation-failed"; return false; }
  auto &s = *state_;
  s.ctrl.wire = &wire;
  s.ctrl.base.is_open = ctrlIsOpen;
  s.ctrl.base.read_reg = ctrlRead;
  s.ctrl.base.write_reg = ctrlWrite;
  s.ctrl.base.get_info = ctrlInfo;
  wire.beginTransmission(PassportPins::codecAddress);
  if (wire.endTransmission() != 0) { error_ = "audio:codec-not-found"; end(); return false; }

  i2s_chan_config_t channel = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
  channel.dma_desc_num = 6;
  channel.dma_frame_num = 240;
  channel.auto_clear_after_cb = true;
  if (i2s_new_channel(&channel, &s.tx, &s.rx) != ESP_OK) {
    error_ = "audio:i2s-channel-allocation-failed"; end(); return false;
  }
  i2s_std_config_t config = {};
  config.clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(sampleRate);
  config.clk_cfg.mclk_multiple = I2S_MCLK_MULTIPLE_256;
  config.slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO);
  config.gpio_cfg.mclk = static_cast<gpio_num_t>(PassportPins::i2sMclk);
  config.gpio_cfg.bclk = static_cast<gpio_num_t>(PassportPins::i2sBclk);
  config.gpio_cfg.ws = static_cast<gpio_num_t>(PassportPins::i2sWs);
  config.gpio_cfg.dout = static_cast<gpio_num_t>(PassportPins::i2sDout);
  config.gpio_cfg.din = static_cast<gpio_num_t>(PassportPins::i2sDin);
  if (i2s_channel_init_std_mode(s.tx, &config) != ESP_OK ||
      i2s_channel_init_std_mode(s.rx, &config) != ESP_OK) {
    error_ = "audio:i2s-config-failed"; end(); return false;
  }
  if (i2s_channel_enable(s.tx) != ESP_OK) { error_ = "audio:i2s-tx-start-failed"; end(); return false; }
  s.txEnabled = true;
  if (i2s_channel_enable(s.rx) != ESP_OK) { error_ = "audio:i2s-rx-start-failed"; end(); return false; }
  s.rxEnabled = true;

  es8311_codec_cfg_t codecConfig = {};
  codecConfig.ctrl_if = &s.ctrl.base;
  codecConfig.codec_mode = ESP_CODEC_DEV_WORK_MODE_BOTH;
  codecConfig.pa_pin = -1;
  codecConfig.master_mode = false;
  codecConfig.use_mclk = true;
  codecConfig.no_dac_ref = true;
  codecConfig.hw_gain.pa_voltage = 5.0f;
  codecConfig.hw_gain.codec_dac_voltage = 3.3f;
  s.codec = es8311_codec_new(&codecConfig);
  if (!s.codec || s.ctrl.failed) { error_ = "audio:codec-init-failed"; end(); return false; }
  esp_codec_dev_sample_info_t format = {};
  format.bits_per_sample = 16;
  format.channel = 2;
  format.sample_rate = sampleRate;
  format.mclk_multiple = 256;
  // The control adapter also records failures the upstream codec may omit.
  if (s.codec->set_fs(s.codec, &format) != ESP_CODEC_DEV_OK || s.ctrl.failed ||
      s.codec->enable(s.codec, true) != ESP_CODEC_DEV_OK || s.ctrl.failed) {
    error_ = "audio:codec-format-or-start-failed"; end(); return false;
  }
  ready_ = true;
  sampleRate_ = sampleRate;
  if (!setMicrophoneGain(30.0f) || !setVolume(30)) { end(); return false; }
  error_ = "ok";
  return true;
}
void PassportAudio::end() {
  ready_ = false;
  sampleRate_ = 0;
  if (!state_) return;
  auto &s = *state_;
  if (s.codec) {
    s.codec->enable(s.codec, false);
    s.codec->close(s.codec);
    free(const_cast<audio_codec_if_t *>(s.codec));
  }
  if (s.txEnabled) i2s_channel_disable(s.tx);
  if (s.rxEnabled) i2s_channel_disable(s.rx);
  if (s.tx) i2s_del_channel(s.tx);
  if (s.rx) i2s_del_channel(s.rx);
  delete state_;
  state_ = nullptr;
}
bool PassportAudio::setVolume(uint8_t percent) {
  if (!ready_) { error_ = "audio:not-initialized"; return false; }
  if (percent > 100) percent = 100;
  auto &s = *state_;
  s.ctrl.failed = false;
  // Digital attenuation, 100% = 0 dB. Hardware gain remains upstream-controlled.
  const float db = percent ? 20.0f * log10f(percent / 100.0f) : -96.0f;
  if (s.codec->set_vol(s.codec, db) != 0 || s.codec->mute(s.codec, percent == 0) != 0 || s.ctrl.failed) {
    error_ = "audio:volume-write-failed"; return false;
  }
  error_ = "ok"; return true;
}
bool PassportAudio::setMicrophoneGain(float decibels) {
  if (!ready_) { error_ = "audio:not-initialized"; return false; }
  if (!isfinite(decibels) || decibels < 0.0f || decibels > 42.0f) {
    error_ = "audio:invalid-microphone-gain"; return false;
  }
  auto &s = *state_;
  s.ctrl.failed = false;
  if (s.codec->set_mic_gain(s.codec, decibels) != 0 || s.ctrl.failed) {
    error_ = "audio:gain-write-failed"; return false;
  }
  error_ = "ok"; return true;
}
size_t PassportAudio::write(const int16_t *samples, size_t count, uint32_t timeoutMs) {
  if (!ready_ || (!samples && count)) { error_ = "audio:invalid-write-state-or-buffer"; return 0; }
  constexpr size_t chunkSize = 128;
  int16_t stereo[chunkSize * 2];
  size_t total = 0;
  const uint32_t started = millis();
  while (total < count) {
    const size_t frames = min(count - total, chunkSize);
    for (size_t i = 0; i < frames; ++i) stereo[2*i] = stereo[2*i+1] = samples[total+i];
    const uint32_t elapsed = millis() - started;
    size_t transferred = 0;
    const esp_err_t result = i2s_channel_write(state_->tx, stereo, frames * sizeof(int16_t) * 2,
                                             &transferred, elapsed < timeoutMs ? timeoutMs - elapsed : 0);
    total += transferred / (sizeof(int16_t) * 2);
    if (result != ESP_OK || transferred != frames * sizeof(int16_t) * 2) {
      error_ = "audio:write-timeout-or-failure"; return total;
    }
  }
  error_ = "ok"; return total;
}
size_t PassportAudio::read(int16_t *samples, size_t count, uint32_t timeoutMs) {
  if (!ready_ || (!samples && count)) { error_ = "audio:invalid-read-state-or-buffer"; return 0; }
  constexpr size_t chunkSize = 128;
  int16_t stereo[chunkSize * 2];
  size_t total = 0;
  const uint32_t started = millis();
  while (total < count) {
    const size_t frames = min(count - total, chunkSize);
    const uint32_t elapsed = millis() - started;
    size_t transferred = 0;
    const esp_err_t result = i2s_channel_read(state_->rx, stereo, frames * sizeof(int16_t) * 2,
                                            &transferred, elapsed < timeoutMs ? timeoutMs - elapsed : 0);
    const size_t received = transferred / (sizeof(int16_t) * 2);
    for (size_t i = 0; i < received; ++i) samples[total+i] = stereo[2*i];
    total += received;
    if (result != ESP_OK || received != frames) { error_ = "audio:read-timeout-or-failure"; return total; }
  }
  error_ = "ok"; return total;
}
}  // namespace folotoy
