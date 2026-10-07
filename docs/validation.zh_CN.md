# 验证记录

[English](validation.md)

验证日期：2026-10-07。目标：ESP32-C3、8 MB Flash，开启 USB CDC。

| 检查 | 状态 |
| --- | --- |
| Arduino Lint 1.3.0 strict / Library Manager submit 规则 | PASS，0 错误、0 警告 |
| C++11 主机测试 | PASS |
| Arduino-ESP32 3.3.12 的七个示例编译 | PENDING |
| 实机测试 | NOT RUN |

主机测试覆盖 ADC 判定边界、消抖、短按/长按分离、稀疏轮询长按、millis 回绕、电压换算，以及示例分区一致、不重叠并保留 cardid。

核心与依赖版本将随编译结果记录。本次未烧录设备。屏幕颜色/方向、按键电压、codec 时钟、实际播放、非零麦克风输入、电量准确性、重复初始化/关闭、外设共存和旧固件恢复均待实机验证。面向社区发布稳定版前，应使用示例完成板级验收。
