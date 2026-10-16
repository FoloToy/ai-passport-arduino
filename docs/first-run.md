# First-run AI onboarding check

[简体中文](first-run.zh_CN.md)

Date: 2026-10-16. Scope: public clone, tool bootstrap, isolated Arduino setup,
lint, host tests, and example compilation following `AGENTS.md`. No device was
flashed or tested in this check.

## Findings and corrections

| Finding | Correction |
| --- | --- |
| The guide assumed Arduino CLI and Arduino Lint already existed. | Added official, versioned installers, prerequisites, PATH setup, and version checks. |
| Existing IDE data and build caches could be used implicitly. | Added empty data/download/sketchbook/cache directories and configuration inspection. |
| curl/Git proxy settings did not configure Arduino CLI 1.5.1 downloads. | Documented `ARDUINO_NETWORK_PROXY` and the required network endpoints. |
| `arduino-lint --compliance strict --library-manager submit .` failed with LS003 because `.` was treated as the folder name. | Reproduced the failure, then passed with the absolute repository path; corrected both guides and READMEs. |
| The built partition binary reserved 3 MB for the application, but CLI size output still enforced 1,310,720 bytes. | Set `upload.maximum_size=3145728` in the compile script and single-example command, retaining actual binary/partition verification. |
| A reader already inside a clone could repeat the clone command there. | Clarified when to reuse a checkout and when to clone from a parent directory. |

The core's board defaults and its size settings were checked against
[Arduino-ESP32 3.3.12 boards.txt](https://github.com/espressif/arduino-esp32/blob/3.3.12/boards.txt).
The IDE can still inherit the default 1.25 MB limit; use the documented CLI workflow
for larger sketches and keep the supplied custom partition file.

## macOS bootstrap and host checks

An Apple Silicon Mac used a new public HTTPS clone at `68b30c3`, without using
existing Arduino core installations or sketchbook libraries. All Arduino directories and
executables were placed in a task-specific temporary directory. Versions:
Arduino CLI 1.5.1, Arduino Lint 1.3.0, Python 3.9.6, Apple Clang 17.0.0.

| Check | Result |
| --- | --- |
| Public HTTPS clone | PASS |
| Official CLI/Lint installers and version checks | PASS |
| Isolated CLI configuration | PASS |
| Index download with explicit CLI proxy | PASS |
| Strict lint with absolute repository path | PASS: no errors or warnings |
| C++11 host tests and example partition checks | PASS |
| Full ESP32 core download/install | STOPPED: slow network transfer; partial downloads removed |
| macOS example compilation | NOT RUN |

Remaining macOS firmware compilation is unverified. To reduce download wait,
some official archives were fetched concurrently with index SHA-256 verification;
this did not complete the macOS core installation. Temporary tools, caches,
downloads, and checkout were removed after retaining the small audit logs.

## Linux clean-run verification

The existing GitHub Actions workflow ran on fresh Ubuntu 24.04 runners. It installed
Arduino CLI 1.5.1, ESP32 core 3.3.12, Adafruit ST7735/ST7789 1.11.0, Adafruit GFX
1.12.6, and Adafruit BusIO 1.17.4, then executed the repository checks.

The [initial run at 68b30c3](https://github.com/FoloToy/ai-passport-arduino/actions/runs/38847766565)
passed both jobs and all nine builds. Its logs exposed the incorrect default
application size check. The [corrected-script run at 45d14a5](https://github.com/FoloToy/ai-passport-arduino/actions/runs/38848507754)
is the separate validation of the size-setting correction:

| Check | Result |
| --- | --- |
| Core and pinned library installation | PASS |
| Strict lint and host tests | PASS |
| Nine example builds, warnings treated as errors | PASS |
| ESP32-C3 / 8 MB image headers and protected partition binaries | PASS |
| CLI maximum application size of 3,145,728 bytes | PASS |
| Device tests | NOT RUN |

The nine examples are AudioTone, BatteryMonitor, BoardSelfTest, Buttons,
DisplayColors, HelloWorld, MicrophoneLevel, PushToTalk, and WiFiScan.
`tools/verify_build.py` checks the actual application/partition binaries, including
the 3 MB factory partition and the reserved `cardid` range; compilation alone
does not establish hardware behavior or speech quality.

Native Windows host tests, physical-board acceptance, and firmware restoration
are outside this check. See [device validation](validation.md) for the separate,
dated hardware evidence.
