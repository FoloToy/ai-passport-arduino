# Validation

[简体中文](validation.zh_CN.md)

Validation date: 2026-10-07. Target: ESP32-C3, 8 MB flash, USB CDC enabled.

| Check | Status |
| --- | --- |
| Arduino Lint 1.3.0, strict, Library Manager submit rules | PASS: 0 errors, 0 warnings |
| C++11 host tests | PASS |
| Seven example builds with Arduino-ESP32 3.3.12 | PENDING |
| Physical-board tests | NOT RUN |

Host tests cover ADC threshold boundaries, debounce, click/long-press separation,
long holds with sparse polling, millis rollover, CW2017 conversion, and identical
non-overlapping example partition layouts with protected cardid reservation.

The core and dependencies are pinned/recorded in the build results once available.
No device was flashed during this validation. Physical display colors/orientation,
button voltages, codec clocking, audible output, non-zero microphone input, battery
accuracy, repeated begin/end, concurrent peripherals, and restoration of the old
firmware remain unverified. Use the examples for board acceptance before a stable
public release.
