# Validation

[简体中文](validation.zh_CN.md)

Validation date: 2026-10-07. Target: ESP32-C3, 8 MB flash, USB CDC enabled.

| Check | Status |
| --- | --- |
| Arduino Lint 1.3.0, strict, Library Manager submit rules | PASS: 0 errors, 0 warnings |
| C++11 host tests | PASS |
| Seven example builds with Arduino-ESP32 3.3.12 | PASS: Linux CI, warnings treated as errors |
| Built ESP32-C3/8 MB headers and protected partition tables | PASS for all seven examples |
| macOS ARM64 WiFiScan and MicrophoneLevel builds/artifact checks | PASS |
| Physical-board tests | NOT RUN |

Host tests cover ADC threshold boundaries, debounce, click/long-press separation,
long holds with sparse polling, millis rollover, CW2017 conversion, and identical
non-overlapping example partition layouts with protected cardid reservation.

Verified source commit: `80f3b5cd4c852d01ee6e000e152cbbb671266c66`.
[Successful CI run](https://github.com/FoloToy/ai-passport-arduino/actions/runs/37589718957).
Tool versions: Arduino CLI 1.5.1, Arduino-ESP32 3.3.12, Adafruit ST7735/ST7789
1.11.0, Adafruit GFX 1.12.6, Adafruit BusIO 1.17.4. All seven examples produced
an ESP32-C3 image declaring 8 MB flash and a partition table with a 3 MB factory
application and `cardid` at `0x356000`, size `0x4000`. Documentation-only follow-up
commits do not change the validated source or build configuration.
No device was flashed during this validation. Physical display colors/orientation,
button voltages, codec clocking, audible output, non-zero microphone input, battery
accuracy, repeated begin/end, concurrent peripherals, and restoration of the old
firmware remain unverified. Use the examples for board acceptance before a stable
public release.
