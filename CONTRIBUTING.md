# Contributing

[简体中文](CONTRIBUTING.zh_CN.md)

Thank you for helping improve FoloToyAIPassport. Small, focused changes are easier
to review. Please describe the problem, expected behavior, and a minimal sketch
when opening an issue. Include the exact board revision, Arduino-ESP32 version,
and sanitized error output for hardware reports.

For pull requests:

1. Keep board pins in `src/PassportPins.h` and keep examples small and runnable.
2. Follow existing C++ formatting and document public API changes.
3. Update English and Chinese documentation together.
4. Run strict Arduino Lint, `python3 tools/test_host.py`, and
   `bash tools/compile_examples.sh` (see README for dependencies).
5. Separate compilation/host-test results from physical-device results. Hardware
   changes need measured checks for the affected peripheral before a stable release.
6. Keep licenses and upstream copyright notices intact. Include attribution for
   any new third-party code or assets.

Do not commit build outputs, credentials, raw flash backups, identity partitions,
or private logs. Use respectful language, assume good intent, and keep discussion
focused on reproducible technical issues. Report sensitive security issues through
[FoloToy's security contact](https://folotoy.com/security/) rather than a public issue.

## Preparing a public release

The repository starts private. Before making a release available to the community:

- Complete and record board acceptance from `docs/validation.md`.
- Confirm the documented pin map against the supported hardware revision.
- Update `library.properties` URL from the public board-resources project to this
  library's public repository URL after it becomes accessible without login.
- Update the changelog and version; create a matching version tag only after checks pass.
- Request Arduino Library Manager inclusion separately; do not claim it is indexed
  until the Arduino index actually contains the library.
