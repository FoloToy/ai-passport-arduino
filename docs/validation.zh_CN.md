# 验证记录

[English](validation.md)

验证日期：2026-10-07。目标：ESP32-C3、8 MB Flash，开启 USB CDC。

| 检查 | 状态 |
| --- | --- |
| Arduino Lint 1.3.0 strict / Library Manager submit 规则 | PASS，0 错误、0 警告 |
| C++11 主机测试 | PASS |
| Arduino-ESP32 3.3.12 的七个示例编译 | PASS，Linux CI，警告视为错误 |
| 实际镜像的 ESP32-C3/8 MB 头及保留身份分区 | 七个示例全部 PASS |
| macOS ARM64 WiFiScan / MicrophoneLevel 编译与产物检查 | PASS |
| 实机测试 | NOT RUN |

主机测试覆盖 ADC 判定边界、消抖、短按/长按分离、稀疏轮询长按、millis 回绕、电压换算，以及示例分区一致、不重叠并保留 cardid。

已验证源码提交：`80f3b5cd4c852d01ee6e000e152cbbb671266c66`。
[通过的 CI 记录](https://github.com/FoloToy/ai-passport-arduino/actions/runs/37589718957)。工具版本：Arduino CLI 1.5.1、Arduino-ESP32 3.3.12、Adafruit ST7735/ST7789 1.11.0、Adafruit GFX 1.12.6、Adafruit BusIO 1.17.4。全部示例实际镜像声明 ESP32-C3/8 MB，实际分区包含 3 MB factory 应用及 `0x356000/0x4000` 的 cardid。后续仅修改文档的提交不改变已验证源码和构建配置。

本次未烧录设备。屏幕颜色/方向、按键电压、codec 时钟、实际播放、非零麦克风输入、电量准确性、重复初始化/关闭、外设共存和旧固件恢复均待实机验证。面向社区发布稳定版前，应使用示例完成板级验收。
