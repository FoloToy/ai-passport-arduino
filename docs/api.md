# API reference

[简体中文](api.zh_CN.md)

Include `<FoloToyAIPassport.h>`. Public types are in namespace `folotoy`.
Create one `AIPassport` per board. Classes that own display/audio resources cannot
be copied. Calls are intended for one sketch task; they are not reentrant.

## Board lifecycle

- `bool begin(const PassportConfig &config = PassportConfig())`: returns false
  if a requested peripheral fails. The default requests display and buttons.
  Battery and audio are optional and default to false. A failure stops any
  started audio/display resources; it does not erase settings or stop shared Wire.
- `void update()`: sample and debounce buttons. Call once per loop, ideally every
  5–10 ms. Read event flags after this call, before calling it again.
- `void end()`: stop audio/button polling and turn off display/backlight. Does not end shared
  SPI/Wire buses. Calling begin again while ready reuses the current configuration;
  call end first to apply a different `PassportConfig`.
- `const char *lastError()`: a static stage-specific error string, or `"ok"`.

`PassportConfig` fields: `display=true`, `buttons=true`, `battery=false`,
`audio=false`, `brightness=50` (0–100%), `sampleRate=16000` (Hz).

## Display

`passport.display` is a `PassportDisplay`, derived from `Adafruit_ST7789`.
It exposes Adafruit GFX methods such as `fillScreen`, `drawPixel`, `drawRect`,
`setCursor`, `print`, and `setRotation`. Coordinates are pixels; colors are RGB565.
No framebuffer is allocated by this library.

- `bool begin(uint8_t brightness = 50)`: initialize the panel and backlight.
  The SPI display has no MISO, so success indicates initialization was issued,
  not confirmation that a physical panel responded.
- `void setBrightness(uint8_t percent)`: 0–100%, clamped to 100.
- `void end()`: display off and backlight off.
- `bool isReady() const`: whether software initialization completed.

Initial orientation is portrait (240 × 320), RGB order, no mirroring, with the
board's required inversion. Check rotations/color order on hardware when changing
panel revisions. SPI and the backlight PWM pin are reserved while the display runs.

## Buttons

`Button` values: `None`, `Up`, `Down`, `Ok`.

- `begin(debounceMs=25, longPressMs=500)` initializes GPIO0/ADC1.
- `update()` samples calibrated millivolts and advances the state machine.
- `millivolts()` returns the latest sampled voltage.
- `held()` returns the current debounced button.
- `isPressed(button)` checks whether the specified button is held.
- `wasPressed`, `wasReleased`, `wasClicked`, `wasLongPressed` return event flags
  valid until the next `update()`. Queries do not consume the event.

A click is emitted on release after a short hold. Long holds produce one long
press and no click. No double-click API is provided. The resistor ladder exposes
one button at a time; simultaneous presses are not independently detectable.
ADC voltage windows are 0–149 mV (UP), 150–446 mV (DOWN), 447–1899 mV (OK),
and ≥1900 mV (released). Timing uses unsigned arithmetic across millis rollover.

## Battery

`passport.battery` reads a CW2017 using the already-initialized Wire bus.

- `bool begin(TwoWire &wire = Wire)`: check the gauge version register.
- `int percent()`: integer percentage, or -1 on I2C error/invalid SOC.
- `int millivolts()`: voltage in mV, or -1 on I2C error.
- `int version()`: raw version register, or -1 on I2C error.

The driver does not provision a cell profile, wake/restart the gauge, or change its
registers. Gauge readiness and SOC accuracy depend on its existing configuration.
A sleeping/unconfigured gauge may need the manufacturer's provisioning firmware.
Transactions have the shared Wire timeout (100 ms when initialized by AIPassport).

## Audio

`passport.audio` owns I2S0 TX/RX and the ES8311 codec. It uses Wire for control,
256 × sample-rate MCLK, stereo I2S slots, and exposes a mono sample API. TX duplicates
each mono sample to both slots; RX selects the left microphone slot. No DAC-reference
channel is enabled. There is no file/WAV/MP3 decoder, network streaming, speech
recognition, or echo cancellation in this API.

- `bool begin(TwoWire &wire = Wire, uint32_t sampleRate = 16000)`:
  configure the codec and I2S. Accepted rates: 8000, 16000, 32000, 44100, 48000 Hz.
  Start with 16 kHz for board acceptance. Default output volume is 30%; microphone
  gain is 30 dB. Calling begin with a new rate stops and recreates audio resources.
- `void end()`: mute/suspend codec and release I2S channels. Wire stays open.
- `bool setVolume(uint8_t percent)`: 0 mutes; 1–100 sets digital attenuation.
- `bool setMicrophoneGain(float decibels)`: accepts finite values in 0–42 dB;
  the codec quantizes gain to supported hardware steps.
- `size_t write(const int16_t *samples, size_t count, uint32_t timeoutMs=1000)`:
  output mono PCM, returning the number of samples accepted by I2S.
- `size_t read(int16_t *samples, size_t count, uint32_t timeoutMs=1000)`:
  capture mono PCM, returning the number of samples placed in the caller's buffer.
- `lastError()`, `isReady()`, `sampleRate()`: state and diagnostics.

`count` is samples, not bytes. I/O is blocking, uses a bounded 512-byte temporary
stereo buffer, and treats timeoutMs as an overall call budget. Partial transfers
return the completed sample count with an error string. Serialize begin/end,
volume/gain changes, and I/O; do not stop audio concurrently with a blocked call.
Do not run audio I/O in an interrupt or a button callback.

## Shared buses and standalone use

The board class initializes Wire at GPIO10/7, 100 kHz only if audio or battery is
requested. Standalone `PassportBattery::begin` and `PassportAudio::begin` expect
the caller to initialize the bus first. Do not create a second I2C owner on these
pins or change speed during transactions. Shared Wire operations require external
serialization if used from multiple tasks. USB uses GPIO18/19; GPIO21 belongs to
the backlight, so use USB CDC for Serial instead of the default UART0 TX.
