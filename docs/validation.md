# Validation

[简体中文](validation.zh_CN.md)

Validation date: 2026-10-07. ESP32-C3, 8 MB flash, Arduino-ESP32 3.3.12,
Arduino CLI 1.5.1, USB CDC enabled.

| Check | Status |
| --- | --- |
| Strict Arduino Lint | PASS: no errors or warnings |
| C++11 host tests | PASS: button thresholds/timing/rollover, voltage conversion, example partitions |
| Nine local example builds | PASS: all warnings treated as errors |
| ESP32-C3 / 8 MB image headers and protected partitions | PASS for all nine local builds |
| Earlier seven-example Linux CI | PASS for source `80f3b5cd4c852d01ee6e000e152cbbb671266c66`; not validation of the later changes |
| RGB display and text | PASS: user confirmed the earlier BoardSelfTest screen |
| Speaker test tone | PASS: user confirmed clear continuous sound |
| UP / DOWN / OK short and long presses | PASS: six operations matched device events |
| Audio init / end cycling | PASS: 15 iterations over five sample rates; repeat retained stable heap |
| Gauge communication | PASS: earlier reading 99%, approximately 4.15 V; accuracy not established |
| Wi-Fi scan | PASS: earlier device scan returned 30 networks |
| PushToTalk three-second capture | PASS: 48,000 samples, zero clipped samples |
| PushToTalk replay and clear control flow | PASS: serial reports full replay and clearing; speech quality needs listening confirmation |
| New rounded UI | PASS: user confirmed the rounded layout and recording prompt |
| Speech quality / volume | Earlier 12 dB / 35% trial: user confirmed clear speech but low volume; current 24 dB / 85% listening retest pending |
| Short recording / repeat recording / playback cancellation | NOT RUN for the new PushToTalk example |
| Original firmware restoration | NOT YET VERIFIED |

PushToTalk app image SHA-256:
`97d1e1af761a9e4e5f647b6ac1a4508d2695acaa076d217f77d3a0c73d743c36`.
The device reported 96,000 bytes allocated for recording and 172,792 bytes of free
heap at startup. The first three-second recording reported peak 1235, RMS 134.23,
zero clipped samples and 2989 ms elapsed wall time. These readings describe that
recording, not a guarantee for all microphone distances or speech levels.

The on-device test writes only the application at `0x10000`, retaining the existing
bootloader and partition table. No extra firmware dump/readback is performed after
writing; esptool's built-in write verification is used. The retained older partition
table has no coredump partition, so Arduino's prebuilt core logs that absence at boot.
The new example partition files reserve a coredump region, but a full installation
of that partition table has not been device-tested. Do not confuse this boot warning
with a recorded application panic.

Remaining acceptance includes speech listening, new UI rendering, short/repeated
recording and cancellation, display rotation/brightness, concurrent peripheral
stress, charge/discharge accuracy, and restoration. No stable/public release is
claimed. Raw device logs and original flash backups remain outside the repository.

[Earlier successful CI run](https://github.com/FoloToy/ai-passport-arduino/actions/runs/37589718957).

The earlier 24 dB / 65% PushToTalk retest image has SHA-256 `2d4e6bfca813bd4902057c3b2982727ec0586193d0801fc3875b827fa6d4fd60`. It was rebuilt with warnings treated as errors and written with built-in digest verification. It has been superseded by the 85% playback version; its listening/cancellation acceptance was not completed.

The current 24 dB / 85% playback image has SHA-256 `2e4ada75322d6d5c003e811928d511dad03e6f2a3b724a7a9d87e06179625bce`. Build with warnings as errors, image/partition checks, built-in write verification, and device startup passed. User listening acceptance remains pending. The `playbackVolume` constant in PushToTalk accepts the audio API range 0–100%.
