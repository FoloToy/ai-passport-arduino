#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
cli="${ARDUINO_CLI:-arduino-cli}"
fqbn='esp32:esp32:esp32c3:CDCOnBoot=cdc,FlashSize=8M,CPUFreq=160'
for sketch in "$root"/examples/*; do
  echo "Compiling $(basename "$sketch")"
  "$cli" compile --fqbn "$fqbn" --library "$root" --warnings all \
    --build-property compiler.cpp.extra_flags=-Werror \
    --build-property compiler.c.extra_flags=-Werror "$sketch"
done
