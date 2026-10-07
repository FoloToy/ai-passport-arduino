# 第三方声明

[English](THIRD_PARTY_NOTICES.md)

## FoloToy 板级参考（MIT）

引脚、按键电压窗口、面板初始化参数及电压换算参考 FoloToy AI Passport BSP，版权为 2026 FoloToy，MIT 声明保留于 LICENSE。

主要公开来源：[FoloToy/ai-passport](https://github.com/FoloToy/ai-passport)，提交 `33d3d1d93a1125b356b47b6d83a7a60121be801e`。另参考 MIT 许可的 [alexwwang/meta-pass](https://github.com/alexwwang/meta-pass)，提交 `9fab503f9468f68d0affa1ce3ddd28b77714c69b` 的显示唤醒处理和电压换算；未引入启动器或应用代码。

## 乐鑫 ES8311 驱动子集（Apache-2.0）

`src/vendor/esp_codec_dev/` 包含来自 [espressif/esp-adf](https://github.com/espressif/esp-adf/tree/ba68421aefb14eddc2d5a86f9ce09a7ea68cae69/components/esp_codec_dev) 提交 `ba68421aefb14eddc2d5a86f9ce09a7ea68cae69` 的 ES8311 驱动及所需接口、音量、引用计数和 FreeRTOS 文件。完整上游路径清单见英文对应文件。

版权属于 2023–2026 Espressif Systems (Shanghai) CO LTD，保留 SPDX 头和 `LICENSES/Apache-2.0.txt` 完整许可证。将路径归并到一个内部目录；FoloToy 对 `es8311.c` 增加显式 `<stdlib.h>` 引用，并让 `es8311_set_fs` 传回格式/时钟配置错误，不再忽略返回值；其余上游源码未改动。`PassportAudio.cpp` 的 Wire/I2S 适配代码由 FoloToy 编写，采用 MIT 许可。

## 独立安装的依赖

Adafruit ST7735/ST7789、Adafruit GFX 和 Adafruit BusIO 单独安装，不内置于本库，遵循各自许可证。Arduino-ESP32 和 ESP-IDF 依赖也保留其上游许可。
