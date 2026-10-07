# 电池与电量计

[English](battery.md)

## 硬件依据

[AI Passport 主库产品规格](https://github.com/FoloToy/ai-passport/blob/1178e40713455754ec9a6a2dd8e5dde46f80e513/docs/hardware-design/specifications.zh_CN.md)记录了内置 500 mAh 可充电锂电池、USB Type-C 5 V 输入和 CellWise CW2017 电量计。这些规格对应所记录的板卡和电芯；其他硬件版本应先核对实际配置。

CW2017 的 7 位 I2C 地址为 `0x63`，与 ES8311 共用 Wire：SDA GPIO10、SCL GPIO7、100 kHz。寄存器解释和电芯配置参考[主库电池驱动](https://github.com/FoloToy/ai-passport/blob/1178e40713455754ec9a6a2dd8e5dde46f80e513/components/bsp/src/bsp_battery.c)。

## 读取方式

- `passport.battery.percent()` 读取 `0x04` 的 SOC 整数部分，返回 0–100%；通信失败或 SOC 无效时返回 `-1`。
- `passport.battery.millivolts()` 读取 `0x02–0x03` 的 14 位电压数据，以整数运算换算：`mV = (raw & 0x3fff) * 3125 / 10000`；通信失败时返回 `-1`。
- `passport.battery.version()` 读取 `0x00` 版本寄存器。芯片应答只证明通信成功，不代表电芯参数正确或 SOC 已就绪。

SOC 由电量计依据已配置的电芯模型计算，本库不通过电压查表估算百分比。这些接口不提供充电状态、充电电流、剩余使用时间或实测电池容量。

## 配置与电源管理

所引用的主固件会同时检查更新标志和完整的 80 字节电芯 profile，匹配其指定的 500 mAh 电芯。此次容量文字修正保留了现有 profile 字节表，不代表完成新的标定。需要更新时，先进入休眠态写入并校验 profile，再设置更新标志、重启电量计，并最多等待 5 秒获得有效 SOC。主库还提供电量计休眠操作。

当前 Arduino 驱动为只读接口：`begin()` 检测版本寄存器，不配置 profile、不唤醒或复位电量计，也不管理电量计休眠。`AIPassport::end()` 不会让电量计进入休眠。因此，休眠或未配置的电量计需要在当前 API 之外完成配置或唤醒。更换电芯后，应验证电芯参数，不能直接套用主固件的 profile。

## 示例与验证

[BatteryMonitor](../examples/BatteryMonitor/BatteryMonitor.ino) 开启 `PassportConfig::battery`，每秒输出电量百分比和毫伏电压。将 `-1` 视为不可用，不要显示成 0% 电量。

验证时应覆盖插入和拔出 USB、充放电过程中的读数变化，以及录音/播放期间的 I2C 读取。单次有效读数不能证明 SOC 精度或可用容量；充放电验收应与编译、基本通信检查分别记录。

参考版本：`1178e40713455754ec9a6a2dd8e5dde46f80e513`。后续硬件或电芯参数变更以主库为准，本文不表示两个仓库会自动同步。

修正后的参考版本已提交为 [AI Passport PR #101](https://github.com/FoloToy/ai-passport/pull/101)，等待主库合并。
