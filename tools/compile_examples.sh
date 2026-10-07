#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
cli="${ARDUINO_CLI:-arduino-cli}"
fqbn='esp32:esp32:esp32c3:CDCOnBoot=cdc,FlashSize=8M,CPUFreq=160'
for sketch in "$root"/examples/*; do
  echo "Compiling $(basename "$sketch")"
  build="$root/build/$(basename "$sketch")"
  "$cli" compile --fqbn "$fqbn" --library "$root" --warnings all --build-path "$build" \
    --build-property compiler.cpp.extra_flags=-Werror \
    --build-property compiler.c.extra_flags=-Werror "$sketch"
  python3 "$root/tools/verify_build.py" "$build" "$(basename "$sketch")"
done
