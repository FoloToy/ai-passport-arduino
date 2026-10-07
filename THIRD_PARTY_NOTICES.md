# Third-party notices

[简体中文](THIRD_PARTY_NOTICES.zh_CN.md)

## FoloToy board references (MIT)

Pin assignments, button voltage windows, panel-specific initialization parameters,
and battery voltage conversion were adapted from the FoloToy AI Passport BSP.
Copyright (c) 2026 FoloToy. The MIT notice is retained in `LICENSE`.

Primary public reference: [FoloToy/ai-passport](https://github.com/FoloToy/ai-passport),
commit `33d3d1d93a1125b356b47b6d83a7a60121be801e`. The related MIT-licensed
[alexwwang/meta-pass](https://github.com/alexwwang/meta-pass), commit
`9fab503f9468f68d0affa1ce3ddd28b77714c69b`, was also consulted for display wake-up
handling and gauge conversion. No launcher/application code is included.

## Espressif ES8311 driver subset (Apache-2.0)

`src/vendor/esp_codec_dev/` contains the ES8311 device driver and its
required interface, volume, reference-management, and FreeRTOS support files from
[espressif/esp-adf](https://github.com/espressif/esp-adf/tree/ba68421aefb14eddc2d5a86f9ce09a7ea68cae69/components/esp_codec_dev),
commit `ba68421aefb14eddc2d5a86f9ce09a7ea68cae69`.

Upstream paths under `components/esp_codec_dev/`:

- `device/es8311/es8311.c`, `device/es8311/es8311_reg.h`
- `device/include/es8311_codec.h`
- `device/common/codec_ref_mgr.c`, `device/priv_include/codec_ref_mgr.h`, `device/priv_include/es_common.h`
- `interface/audio_codec_if.h`, `interface/audio_codec_ctrl_if.h`, `interface/audio_codec_gpio_if.h`
- `include/esp_codec_dev_types.h`, `include/esp_codec_dev_vol.h`, `include/esp_codec_dev_os.h`
- `esp_codec_dev_vol.c`, `platform/esp_codec_dev_os.c`

Copyright 2023–2026 Espressif Systems (Shanghai) CO LTD. SPDX headers are retained.
The complete license is included in `LICENSES/Apache-2.0.txt`. Files were flattened
into one internal directory; `es8311.c` adds an explicit `<stdlib.h>` include and propagates format/clock
configuration errors in `es8311_set_fs` instead of discarding them. These are
FoloToy modifications; all other vendored source contents remain unchanged. The Wire/I2S
adapter in `PassportAudio.cpp` is FoloToy code licensed under MIT.

## Installed dependencies

The [Adafruit ST7735/ST7789](https://github.com/adafruit/Adafruit-ST7735-Library),
[Adafruit GFX](https://github.com/adafruit/Adafruit-GFX-Library), and Adafruit BusIO
libraries are installed independently and not vendored here. Their respective
license notices apply. This library uses the Arduino-ESP32 core and ESP-IDF APIs;
those dependencies retain their upstream licenses.
