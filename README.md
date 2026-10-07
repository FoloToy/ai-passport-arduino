# FoloToy AI Passport Arduino

[简体中文](README.zh_CN.md)

An Arduino library for the **FoloToy AI Passport ESP32-C3 board**. Build sketches
with its color display, three buttons, microphone, speaker, and battery gauge
without repeating the board's pin configuration.

This is an early `0.1.0` implementation. Builds, host tests, and device checks are tracked in
[validation](docs/validation.md); full physical-board acceptance is still pending.
Only the ESP32-C3 AI Passport pin map is supported.

## Features

- ST7789P3 240 × 320 display with the Adafruit GFX drawing API and backlight control.
- Debounced UP, DOWN, and OK buttons: press, release, click, and long-press events.
- ES8311 microphone and speaker access as signed 16-bit mono PCM.
- CW2017 battery percentage and voltage readings without rewriting its profile.
- Small examples for each peripheral, plus a Wi-Fi scan using the ESP32 core.

No cloud account, API key, or network connection is required by the library.
Audio and battery initialization are opt-in, so sketches can use only what they need.
The passive NFC tag is not connected to the MCU and has no software API here.

Hardware specifications and board behavior follow the [AI Passport main repository](https://github.com/FoloToy/ai-passport). See [battery and fuel gauge notes](docs/battery.md) for the 500 mAh cell, CW2017 readings, and configuration requirements.

## Requirements

- FoloToy AI Passport with ESP32-C3 and 8 MB flash; no PSRAM is required.
- Arduino IDE 2.x or Arduino CLI.
- **esp32 by Espressif Systems**, Arduino-ESP32 3.3.12 (the tested core).
- **Adafruit ST7735 and ST7789 Library** and **Adafruit GFX Library**.
  Install their dependencies when prompted.

## Installation

This library is not yet listed in the Arduino Library Manager.

1. Download this repository as a ZIP, or clone it and ZIP the library folder.
2. In Arduino IDE, choose **Sketch → Include Library → Add .ZIP Library**.
3. Install the dependencies above from Library Manager.
4. Install the Espressif ESP32 board package following its
   [official installation guide](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html).
5. Open **File → Examples → FoloToyAIPassport → HelloWorld**.

In **Tools**, select:

| Setting | Value |
| --- | --- |
| Board | ESP32C3 Dev Module |
| Flash Size | 8 MB (64 Mb) |
| USB CDC On Boot | Enabled |
| CPU Frequency | 160 MHz |
| Erase All Flash Before Sketch Upload | Disabled |
| Port | Your AI Passport's native USB port |

Keep `partitions.csv` alongside each example sketch. For a new sketch, copy
[`extras/partitions.csv`](extras/partitions.csv) into its folder. It limits the
application to 3 MB and reserves `cardid` at `0x356000`.

Uploading a sketch replaces the running firmware. A normal IDE upload also
writes the bootloader and partition table and can affect existing settings.
Back up a provisioned device before experimenting. Do not use a generic
filesystem/OTA partition layout or erase all flash on a provisioned AI Passport.
This library does not read, generate, or modify device identity data, and is not
an installer for meta-pass launcher slots. See [hardware notes](docs/hardware.md).

## Quick start

```cpp
#include <FoloToyAIPassport.h>

folotoy::AIPassport passport;

void setup() {
  Serial.begin(115200);
  if (!passport.begin()) {
    Serial.println(passport.lastError());
    return;
  }

  passport.display.setTextColor(ST77XX_WHITE);
  passport.display.setTextSize(2);
  passport.display.setCursor(16, 24);
  passport.display.println("Hello, Passport!");
}

void loop() {
  passport.update();
  if (passport.buttons.wasClicked(folotoy::Button::Ok)) {
    passport.display.fillScreen(ST77XX_BLUE);
  }
  delay(5);
}
```

`begin()` starts the display and buttons by default. Use `PassportConfig` to enable
battery or audio, or to disable the display in a serial-only sketch. The library
has no hidden button task: call `update()` frequently, then inspect events before
the next update. Long blocking work can miss presses.

```cpp
folotoy::PassportConfig config;
config.audio = true;
config.battery = true;
if (!passport.begin(config)) {
  Serial.println(passport.lastError());
}
```

See the [API reference](docs/api.md) for lifecycle, errors, bus ownership,
PCM format, and timing details.

## Examples

| Sketch | Purpose |
| --- | --- |
| [HelloWorld](examples/HelloWorld/HelloWorld.ino) | Display text and react to OK |
| [DisplayColors](examples/DisplayColors/DisplayColors.ino) | Check colors, orientation, and screen edges |
| [Buttons](examples/Buttons/Buttons.ino) | Inspect button events and ADC voltage |
| [BatteryMonitor](examples/BatteryMonitor/BatteryMonitor.ino) | Read percentage and voltage |
| [AudioTone](examples/AudioTone/AudioTone.ino) | Play a quiet 1 kHz tone |
| [MicrophoneLevel](examples/MicrophoneLevel/MicrophoneLevel.ino) | Report microphone peak level without storing recordings |
| [PushToTalk](examples/PushToTalk/PushToTalk.ino) | Hold OK to record, release to finish; UP replays (max 3 s) |
| [WiFiScan](examples/WiFiScan/WiFiScan.ino) | Show nearby networks without connecting |

## Development

```sh
arduino-cli core update-index --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core install esp32:esp32@3.3.12 --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli lib install "Adafruit ST7735 and ST7789 Library" "Adafruit GFX Library"
arduino-lint --compliance strict --library-manager submit .
python3 tools/test_host.py
bash tools/compile_examples.sh
```

The compile script builds every example for ESP32-C3 with USB CDC and 8 MB flash;
it never uploads to hardware. GitHub Actions runs the same checks. CI configuration
and a passing build are not evidence of a physical device test.

## Contributing and support

Bug reports, small examples, and documentation improvements are welcome. Please
read [CONTRIBUTING.md](CONTRIBUTING.md) before opening a pull request. When reporting
a hardware issue, include the board revision, core version, sketch, and a sanitized
serial log. Use GitHub Issues for reproducible library problems and discussions
about API improvements. Do not include credentials or device identity data.

## License

FoloToy library code and examples are released under the [MIT License](LICENSE).
The included Espressif ES8311 driver subset is licensed under Apache-2.0; its
copyright notices and license are retained. Adafruit libraries are installed
separately. See [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) for sources and licenses.

See [push-to-talk recording](docs/recording.md) for controls, RAM use, and acceptance. Display demos use a 30 px rounded viewport; see the [display API](docs/api.md).
