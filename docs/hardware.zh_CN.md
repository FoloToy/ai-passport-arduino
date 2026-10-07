# 硬件说明

[English](hardware.md)

目标为 ESP32-C3、8 MB Flash 的 FoloToy AI Passport。引脚已与 [FoloToy 官方公开板级资料](https://github.com/FoloToy/ai-passport/blob/33d3d1d93a1125b356b47b6d83a7a60121be801e/components/bsp/include/bsp_pins.h)核对；面板序列参考同一提交的 `bsp_display.c`。Arduino 首版采用较保守的 40 MHz SPI，低于当前上游 80 MHz 设置。当前属于源码移植，尚未完成实机验收。

| 信号 | GPIO / 地址 |
| --- | --- |
| LCD MOSI / SCLK / CS / DC | 9 / 8 / 1 / 20 |
| LCD reset | 软件复位，无 MCU GPIO |
| 背光 | GPIO21，5 kHz，10 位 PWM |
| UP/DOWN/OK 分压键 | GPIO0 / ADC1，外部上拉 |
| I2C SDA / SCL | 10 / 7，100 kHz |
| ES8311 | 7 位 I2C 地址 0x18 |
| CW2017 | 7 位 I2C 地址 0x63 |
| I2S MCLK / BCLK / WS | 6 / 5 / 3 |
| I2S DOUT / DIN | 2 / 4 |
| USB Serial/JTAG | GPIO18 / GPIO19 |

ESP32-C3 没有 PSRAM，应按小块流式处理音频，并为无线栈预留内存。NFC 为独立被动标签；硬件电源键与三颗 ADC 功能键分开。

## 分区与上传边界

全部示例携带同一份自定义分区表：nvs `0x9000/0x5000`，otadata `0xe000/0x2000`，factory `0x10000/0x300000`，cardid `0x356000/0x4000`。

NVS/otadata 布局适配 Arduino 上传的 boot_app0 约定，与生产/启动器 ESP-IDF 布局不同。cardid 留作保留分区，避免应用/文件系统覆盖；仓库不包含身份数据。示例不启用文件系统或 OTA 更新器。

Arduino 常规上传会写入 bootloader、分区表、boot_app0 和草图，是安装独立固件。它可能覆盖旧 NVS 布局的一部分，并替换现有启动器，不代表兼容生产固件或 meta-pass 槽位安装。修改已有设备前先备份完整 Flash，并另行验证恢复。不要全片擦除，也不要未经核对就把未分配空间改为文件系统。本库自身不执行烧录。

## 实机验收

使用 DisplayColors 检查红绿蓝、裁剪、方向与背光；Buttons 测量实际版本的松开/按下电压；AudioTone 检查频率，MicrophoneLevel 检查非零输入；充放电状态下检查电量/电压，并验证音频期间 I2C 共存。重复 begin/end 和音频采样率切换应检查内存泄漏。编译通过不能代替这些观察。
