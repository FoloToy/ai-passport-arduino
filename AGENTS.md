<p align="right"><strong>English</strong> · <a href="AGENTS.zh_CN.md">简体中文</a></p>

# Repository guidelines for AI agents

Start here when using this library to build a sketch or when maintaining the
library itself. Read only the task-specific documents below after this entry point.
Run all commands from the repository root unless stated otherwise.

## Scope and source of truth

- This is **FoloToyAIPassport**, an Arduino peripheral library with runnable
  examples for the **ESP32-C3 AI Passport, 8 MB flash, no PSRAM**. It is not a
  board-manager package, a complete product firmware, or a launcher-slot installer.
- The public entry point is `src/FoloToyAIPassport.h`, in namespace `folotoy`.
  Reusable drivers belong in `src/`; application UI and workflows belong in
  `examples/<SketchName>/` or the user's own sketch.
- Begin with `git status --short --branch`. Preserve unrelated user changes.
- For current API behavior, read the headers and implementation in this checkout,
  then `docs/api.md` and the matching example. Keep hardware constants in
  `src/PassportPins.h`; do not scatter GPIO numbers through new sketches.
- For hardware facts, consult the [official AI Passport repository](https://github.com/FoloToy/ai-passport),
  especially its [product specifications](https://github.com/FoloToy/ai-passport/blob/main/docs/hardware-design/specifications.md)
  and [BSP pins](https://github.com/FoloToy/ai-passport/blob/main/components/bsp/include/bsp_pins.h).
  Compare the supported board revision and reference commit in `docs/hardware.md`.
  Resolve source conflicts before changing pins or electrical settings; do not guess.
- The official [AI agent guide](https://github.com/FoloToy/ai-passport/blob/main/AGENTS.md)
  and [Sunflower Buddy guide](https://github.com/FoloToy/sunflower-buddy/blob/main/AGENTS.md)
  inform the structure of this guide. Their ESP-IDF commands, LVGL APIs, mandatory
  skill setup, and other boards' pin maps do not apply to this Arduino library.

## Task routing

| Task | Read first |
| --- | --- |
| First sketch or IDE setup | `README.md`, `examples/HelloWorld/HelloWorld.ino` |
| Public API or lifecycle | `docs/api.md`, `src/FoloToyAIPassport.h`, affected driver headers and implementation |
| Display or UI | `src/PassportDisplay.h`, `src/PassportDisplay.cpp`, `examples/DisplayColors/DisplayColors.ino` |
| Buttons or timing | `src/PassportButtons.cpp`, `src/detail/ButtonState.h`, `tests/test_logic.cpp` |
| Audio or recording | `docs/recording.md`, `src/PassportAudio.h`, `src/PassportAudio.cpp`, `examples/PushToTalk/` |
| Battery or shared I2C | `docs/battery.md`, `src/PassportBattery.cpp`, `src/PassportPins.h` |
| Hardware, partitions, or upload | `docs/hardware.md`, `extras/partitions.csv`, `tools/verify_build.py` |
| Checks or acceptance | `.github/workflows/ci.yml`, `tools/compile_examples.sh`, `tools/test_host.py`, `docs/validation.md` |
| Contribution or release | `CONTRIBUTING.md`, `THIRD_PARTY_NOTICES.md`, `library.properties`, `CHANGELOG.md` |

## Set up and build

Use Arduino IDE 2.x or Arduino CLI. The checked-in CI baseline is Arduino CLI
1.5.1 and Arduino-ESP32 **3.3.12**; do not silently upgrade the core while fixing
an unrelated issue. Host tests need Python 3 and a C++11 compiler (`c++`, or `CXX`).
The library is installed from this checkout/ZIP, not assumed to be in Library Manager.

### First-time CLI bootstrap

The shell commands and scripts require Bash, Git, curl, tar, Python 3, and a
C++11 compiler. On macOS, install Apple's command-line tools with
`xcode-select --install` if `c++ --version` fails. On Debian/Ubuntu, the corresponding
packages are `build-essential python3 git curl ca-certificates` (install through
the system package manager with the required permissions). On Windows, use WSL
for this shell/host-test workflow, or follow the Arduino IDE setup in `README.md`;
native Windows host tests have not been validated.

If the repository is already cloned, use that checkout and start with the status
and version checks below; do not clone another repository inside it. Otherwise,
run the clone and `cd` commands from a parent directory. The public clone requires
no GitHub account or private repository access:

```sh
git clone https://github.com/FoloToy/ai-passport-arduino.git
cd ai-passport-arduino
git status --short --branch
python3 --version
c++ --version
```

Install Arduino CLI 1.5.1 and Arduino Lint 1.3.0 from the official installers
if those versions are not already available. These commands put the executables
in a user-owned directory; they do not require `sudo` or change shell startup files:

```sh
mkdir -p "$HOME/.local/bin"
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/v1.5.1/install.sh | BINDIR="$HOME/.local/bin" sh -s 1.5.1
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-lint/1.3.0/etc/install.sh | BINDIR="$HOME/.local/bin" sh -s 1.3.0
export PATH="$HOME/.local/bin:$PATH"
arduino-cli version
arduino-lint --version
```

See the official [CLI installation](https://arduino.github.io/arduino-cli/1.5/installation/)
and [Lint installation](https://arduino.github.io/arduino-lint/latest/installation/)
guides for platform-specific downloads. Check existing executables before replacing
them; choose a separate installation directory when another project needs a different
version. Add that directory to `PATH` in each new shell, or use its absolute executable
paths. `ARDUINO_CLI` only selects the executable used by `compile_examples.sh`.

For a clean first-run check, use empty Arduino data/download/sketchbook directories
instead of changing an existing IDE installation. Keep these exports in the same
shell for all subsequent CLI commands:

```sh
export PASSPORT_ARDUINO_ENV="$(mktemp -d "${TMPDIR:-/tmp}/passport-arduino.XXXXXX")"
export ARDUINO_DIRECTORIES_DATA="$PASSPORT_ARDUINO_ENV/data"
export ARDUINO_DIRECTORIES_DOWNLOADS="$PASSPORT_ARDUINO_ENV/downloads"
export ARDUINO_DIRECTORIES_USER="$PASSPORT_ARDUINO_ENV/sketchbook"
export ARDUINO_BUILD_CACHE_PATH="$PASSPORT_ARDUINO_ENV/cache"
export ARDUINO_UPDATER_ENABLE_NOTIFICATION=false
arduino-cli config dump
```

Confirm the reported directories point at the new environment. Check free disk
space before installation: ESP32 core 3.3.12 installs tools/libraries for several
chips even though this project targets C3. Allow several GB for downloads, extracted
tools, caches, and builds. Network access to GitHub, Arduino downloads, and Espressif
package hosts is required. Arduino CLI 1.5.1 uses its own `network.proxy` setting;
setting only `HTTPS_PROXY` for curl/Git does not configure CLI downloads. If your
environment requires an HTTP proxy, set `ARDUINO_NETWORK_PROXY` to its working URL
in this same shell, for example `export ARDUINO_NETWORK_PROXY=http://127.0.0.1:7890`
when a proxy actually listens there. Use your environment's approved endpoint;
keep proxy credentials out of logs and the repository. See the official
[CLI configuration](https://arduino.github.io/arduino-cli/1.5/configuration/).

### Core, libraries, and checks

Install the same dependencies as CI:

```sh
arduino-cli core update-index --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core install esp32:esp32@3.3.12 --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli lib install "Adafruit ST7735 and ST7789 Library@1.11.0" "Adafruit GFX Library@1.12.6" "Adafruit BusIO@1.17.4"
```

Check installed versions with `arduino-cli version`, `arduino-cli core list`, and
`arduino-cli lib list`. Use the scripts rather than inventing board options:

```sh
arduino-lint --compliance strict --library-manager submit "$PWD"
python3 tools/test_host.py
bash tools/compile_examples.sh
```

Pass the absolute repository path to Arduino Lint: version 1.3.0 treats a literal
`.` argument as the library folder name and reports LS003 even for a valid checkout.

`compile_examples.sh` builds every example with warnings treated as errors, then
checks each actual image and partition binary with `tools/verify_build.py`. It
uses `esp32:esp32:esp32c3:CDCOnBoot=cdc,FlashSize=8M,CPUFreq=160`, writes to
`build/<SketchName>/`, and never uploads. It sets `upload.maximum_size=3145728`
to match the 3 MB factory partition; the core's default 1.25 MB size limit would
otherwise reject larger sketches even with the custom partition file present.
`ARDUINO_CLI` can select a CLI executable.
For one-example iteration:

```sh
arduino-cli compile --fqbn 'esp32:esp32:esp32c3:CDCOnBoot=cdc,FlashSize=8M,CPUFreq=160' --library "$PWD" --warnings all --build-path "$PWD/build/HelloWorld" --build-property upload.maximum_size=3145728 --build-property compiler.cpp.extra_flags=-Werror --build-property compiler.c.extra_flags=-Werror examples/HelloWorld
python3 tools/verify_build.py build/HelloWorld HelloWorld
```

If a tool is missing, follow the bootstrap instructions and rerun its version
check before continuing. If installation is blocked, report the failed command
and missing tool, complete the independent checks that remain possible, and do
not claim a pass. Markdown-only pushes/PRs are excluded by this CI workflow's path filters;
an absent run is not a successful check.

## Building an application

1. Start from the smallest matching example. A new example must have a matching
   directory and `.ino` basename; copy `extras/partitions.csv` beside the sketch.
2. Include `<FoloToyAIPassport.h>` and create one `folotoy::AIPassport` per board.
   `begin()` enables display and buttons by default. Enable `PassportConfig::audio`
   or `battery` only when needed. Check `begin()` and report `lastError()` on failure.
3. Call `passport.update()` about every 5–10 ms and read button events before the
   next call. Use short work chunks/state machines for responsive controls.
4. Keep application behavior outside the driver classes. Build a UI suited to the
   requested application; the self-test/recording screen is an example, not a
   required application shell.
5. Validate the affected behavior and record what still needs a physical board.

## Runtime invariants

- APIs are not reentrant. Serialize resource lifecycle and I/O; call `end()` before
  changing the board configuration. Do not stop audio during a blocked read/write.
- Display uses Adafruit GFX, RGB565, portrait 240 × 320, without a framebuffer.
  Preserve the 30 px black corner mask with `fillRoundedScreen()` or
  `applyCornerMask()` after drawing into corners; keep text/controls inside a
  24–30 px margin. GFX drawing is not automatically clipped by the mask.
- Adafruit's built-in font does not supply Chinese glyphs. Before adding Chinese
  UI, provide a compatible rendering/font solution and verify visible glyphs;
  UTF-8 strings and compilation alone do not establish support.
- Buttons share one ADC resistor ladder. Simultaneous keys cannot be distinguished
  independently. Event flags last until the next `update()`; queries do not consume
  them. Long presses do not also emit clicks. Keep interrupt/callback work bounded.
- Audio is signed 16-bit mono PCM; `read`/`write` counts are **samples, not bytes**.
  Start at 16 kHz. Handle partial transfers and errors; no built-in file decoder,
  speech recognition, or echo cancellation is provided. Poll buttons between small
  chunks and keep audio I/O out of interrupts/callbacks.
- Budget RAM explicitly: three seconds at 16 kHz × two bytes uses 96,000 bytes,
  before driver/UI/radio memory. No PSRAM is available; avoid unbounded buffers.
- Audio and the CW2017 share Wire on GPIO10/7 at 100 kHz. Do not create a second
  owner or change bus speed during transactions. Standalone driver use must
  initialize Wire first. `end()` leaves shared SPI/Wire buses open.
- Battery readings can return `-1`; show unavailable instead of 0%. The gauge
  driver is read-only and does not provision a profile, wake the gauge, or infer
  charging state. Do not silently add profile writes or claim SOC accuracy.
- GPIO21 drives the backlight, so use USB CDC for Serial rather than UART0 TX.
  GPIO18/19 belong to USB. The passive NFC tag has no MCU API here, and the
  hardware power button is separate from the three ADC keys.

## Flash and device boundaries

- Keep every example's `partitions.csv` byte-identical to `extras/partitions.csv`.
  The factory application starts at `0x10000` and is limited to `0x300000` bytes;
  `cardid` is reserved at `0x356000` for `0x4000` bytes. Do not read, generate,
  publish, or overwrite device identity data as part of ordinary sketch work.
- A standard Arduino upload writes more than the application: it can replace the
  launcher, bootloader, partition metadata, and existing settings. The reservation
  alone does not prove compatibility with a device's existing layout. Follow
  `docs/hardware.md` and the user's backup/data requirements before an upload.
- Do not use a generic OTA/filesystem layout, full-chip erase, or launcher-slot
  installation assumptions. App-only testing at `0x10000` also requires checking
  the existing layout; it is not a universally safe upload recipe.
- Device detection is not permission to flash. Obtain task authorization before
  uploading, erasing, restoring, publishing firmware, or sending messages. Keep
  compilation and host checks available without a connected board.

## Changes and delivery

- Keep changes focused; update affected public API documentation and examples.
  Keep English `.md` and Simplified Chinese `.zh_CN.md` documents aligned with
  reciprocal language links, including this guide. Follow existing C++ formatting.
- Put portable logic in testable helpers. Add meaningful host coverage for changed
  thresholds, timing, state transitions, or PCM processing; do not add tests merely
  to restate a documentation edit.
- Preserve third-party licenses/attribution in `src/vendor/`, `LICENSES/`, and
  `THIRD_PARTY_NOTICES.md`. Do not commit build outputs, credentials, flash backups,
  identity partitions, recordings, or unsanitized logs.
- For code changes, run strict lint, host tests, and all example builds before
  delivery. For documentation-only changes, verify paths, commands against the
  scripts/CI, and bilingual consistency; say which executable checks were not run.
- Read `docs/validation.md` as dated evidence for particular versions. Do not
  convert earlier partial board tests into acceptance of the current change.
  Display initialization cannot read back panel success; audio sample counts do
  not establish speech quality. Record actual device observations separately.
- Commit/push within the user's authorized scope. Tags, releases, Library Manager
  inclusion, and community publication need their own requested workflow; a
  public repository or successful build does not establish those statuses.

Report the changed behavior, relevant files, and these results separately:

```text
Arduino Lint: PASS / FAIL / NOT RUN
Host tests: PASS / FAIL / NOT RUN
Example builds and image/partition checks: PASS / FAIL / NOT RUN
Device tests: PASS / FAIL / NOT RUN
Unverified: remaining checks and any missing tools/hardware
GitHub delivery: local changes / pushed branch / PR / merged, with link when applicable
```
