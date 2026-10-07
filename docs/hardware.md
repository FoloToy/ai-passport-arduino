# Hardware notes

[简体中文](hardware.zh_CN.md)

The supported target is the ESP32-C3 FoloToy AI Passport with 8 MB flash. The
mapping was cross-checked against the public
[FoloToy board resources](https://github.com/FoloToy/ai-passport/blob/33d3d1d93a1125b356b47b6d83a7a60121be801e/components/bsp/include/bsp_pins.h).
Panel initialization follows that project's `bsp_display.c` at the same commit.
The initial Arduino port uses a conservative 40 MHz SPI clock, lower than the
current upstream 80 MHz setting. This is a source-derived port; it has not been
accepted on a physical board yet.

| Signal | GPIO / address |
| --- | --- |
| LCD MOSI / SCLK / CS / DC | 9 / 8 / 1 / 20 |
| LCD reset | Software reset; no MCU reset GPIO |
| Backlight | GPIO21, 5 kHz, 10-bit PWM |
| UP/DOWN/OK ladder | GPIO0, ADC1; external pull-up |
| I2C SDA / SCL | 10 / 7; 100 kHz |
| ES8311 control | 7-bit I2C address 0x18 |
| CW2017 gauge | 7-bit I2C address 0x63 |
| I2S MCLK / BCLK / WS | 6 / 5 / 3 |
| I2S DOUT / DIN | 2 / 4 |
| USB Serial/JTAG | GPIO18 / GPIO19 |

ESP32-C3 does not provide PSRAM. Stream audio in small chunks, and account for
radio stacks when estimating memory. The NFC tag is passive and separate from the
MCU. The hardware power button is separate from the three ADC function buttons.

## Flash layout and upload boundaries

Every example includes the same custom partition file:

| Partition | Offset | Size |
| --- | --- | --- |
| nvs | 0x9000 | 0x5000 |
| otadata | 0xe000 | 0x2000 |
| factory | 0x10000 | 0x300000 |
| cardid | 0x356000 | 0x4000 |

The NVS/otadata arrangement matches the Arduino upload boot_app0 convention; it
is not identical to the ESP-IDF production/launcher layout. The reserve for cardid
is included so an application/filesystem cannot overlap it. No cardid data is
shipped. Standard examples do not use a filesystem or OTA updater.

A normal Arduino upload is a standalone-firmware installation and writes bootloader,
partition metadata, boot_app0, and the sketch. It can overwrite part of a previous
NVS layout and replaces the current launcher. It does not establish compatibility
with production firmware or meta-pass slot installation. Back up the full flash
before modifying a provisioned device and validate restoration separately.
Do not enable erase-all-flash or add a filesystem over unassigned regions without
checking the device's actual layout. The library itself performs no flashing.

## Board acceptance

Use `DisplayColors` for red/green/blue, clipping, orientation, and brightness checks.
Use `Buttons` to measure released and pressed voltages on the actual revision.
Use `AudioTone` for tone frequency and `MicrophoneLevel` for non-zero capture.
Check plausible gauge readings over charge/discharge states and repeated I2C
access while audio runs. Test repeated begin/end and audio rate changes for heap
leaks. Compile success is not a substitute for these observations.
