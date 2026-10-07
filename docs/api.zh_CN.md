# API 参考

[English](api.md)

入口为 `<FoloToyAIPassport.h>`，公共类型位于 `folotoy` 命名空间。每块板只创建一个 `AIPassport`。显示与音频资源类不可复制；接口设计为同一草图任务调用，不可重入。

## 板级生命周期

- `begin(config)` 返回请求的外设是否初始化成功；默认开启屏幕和按键，音频和电池关闭。失败时关闭已启动的音频/显示资源，不擦除配置、不关闭共享 Wire。
- `update()` 采样和消抖按键，建议每 5–10 ms 调用一次，随后在下次更新前读取事件。
- `end()` 关闭音频、按键轮询、屏幕和背光，不关闭共享 SPI/Wire。就绪状态重复 begin 复用当前配置；更改配置前先 end。
- `lastError()` 返回带阶段的静态错误字符串，成功为 `"ok"`。

`PassportConfig` 默认值：`display=true`、`buttons=true`、`battery=false`、`audio=false`、`brightness=50`（0–100%）、`sampleRate=16000` Hz。

## 显示

`passport.display` 继承 `Adafruit_ST7789`，可使用 `fillScreen`、`drawPixel`、`drawRect`、`setCursor`、`print`、`setRotation` 等 Adafruit GFX 接口。坐标单位为像素，颜色为 RGB565；本库不分配完整帧缓冲。

- `beginDisplay(brightness=50)` 发送面板初始化并配置背光。SPI 没有 MISO，返回成功只说明软件初始化完成，不代表面板实际应答。
- `setBrightness(percent)` 设置 0–100% 背光，超过 100 会限制到 100。
- `end()` 关闭显示和背光；`isReady()` 表示软件初始化状态。

默认竖屏 240 × 320，RGB 顺序、无镜像、启用本面板所需反色。更换面板后需要实测方向与颜色。显示期间保留 SPI 和背光 PWM 资源。

`fillRoundedScreen(color)` 填充屏幕并绘制 30 像素黑色圆角遮罩，参考主固件的显示区域规则；`applyCornerMask()` 在绘制触及四角的画面后恢复遮罩。两者不需要帧缓冲，并适配当前旋转尺寸。文字与操作区建议留出 24–30 像素边距。Adafruit GFX/底层 SPI 绘制不会自动裁剪：覆盖四角后应调用 `applyCornerMask()`。

## 按键

`Button`：`None`、`Up`、`Down`、`Ok`。

- `begin(debounceMs=25, longPressMs=500)` 初始化 GPIO0/ADC1。
- `update()` 读取校准毫伏并推进状态机；`millivolts()` 返回最近电压。
- `held()` 返回当前稳定按键；`isPressed(button)` 表示是否持续按住。
- `wasPressed`、`wasReleased`、`wasClicked`、`wasLongPressed` 的事件有效到下一次 update。查询不会消耗事件。

短按释放产生 click，长按只产生一次 long press，不再产生 click。本版不提供双击。分压键只能解码一个按键，不能独立识别组合按键。

电压区间：UP 0–149 mV，DOWN 150–446 mV，OK 447–1899 mV，释放 ≥1900 mV。计时支持 millis 回绕。

## 电池

`passport.battery` 通过已初始化的 Wire 读取 CW2017：

- `begin(wire=Wire)` 检查版本寄存器。
- `percent()` 返回 0–100 整数，I2C 失败或 SOC 无效返回 -1。
- `millivolts()` 返回 mV，失败返回 -1。
- `version()` 返回原始版本值，失败返回 -1。

只读接口不写电池 profile、不唤醒或复位电量计。SOC 准确性与就绪状态依赖已有配置；休眠或未配置的芯片可能需要生产固件完成配置。板级 begin 将共享 Wire 超时设为 100 ms。

主固件参考与电芯 profile 配置要求见[电池与电量计说明](battery.zh_CN.md)。

## 音频

`passport.audio` 独占 I2S0 TX/RX 和 ES8311，经 Wire 控制，MCLK 为采样率 ×256。内部为双槽 I2S，外部 API 为单声道。发送时复制到两槽，接收时取左侧麦克风槽；不启用 DAC 参考通道。不包含 WAV/MP3 解码、网络流、语音识别或回声消除。

- `begin(wire=Wire, sampleRate=16000)`：接受 8000、16000、32000、44100、48000 Hz。建议先以 16 kHz 验收。初始音量 30%，麦克风增益 30 dB。更换采样率会关闭并重建音频资源。
- `end()` 静音/暂停 codec 并释放 I2S，不关闭 Wire。
- `setVolume(percent)`：0 静音，1–100 调节数字衰减。
- `setMicrophoneGain(decibels)`：接受有限值 0–42 dB，由 codec 量化为硬件支持的档位。
- `write(samples, count, timeoutMs=1000)`：播放有符号 16 位单声道 PCM，返回 I2S 接收的样本数。
- `read(samples, count, timeoutMs=1000)`：录音并返回填入缓冲区的样本数。
- `lastError()`、`isReady()`、`sampleRate()` 提供状态。

count 是样本数，不是字节数。I/O 会阻塞，使用固定 512 字节临时双槽缓冲，timeoutMs 是整个调用的时间预算。部分传输会返回已完成数量并记录错误。begin/end、音量/增益和 I/O 必须串行调用；不可在阻塞 I/O 期间从另一任务停止音频，也不可从中断/按键回调执行音频 I/O。

## 总线与独立使用

板级对象只在请求音频/电池时初始化 Wire：SDA GPIO10、SCL GPIO7、100 kHz。单独使用电池/音频对象时，调用者需要先初始化 Wire。不要在同一引脚创建第二个 I2C 所有者，或在事务中修改时钟。多任务共享 Wire 时需要外部串行化。

USB 使用 GPIO18/19；GPIO21 是背光，应使用 USB CDC Serial，避免 UART0 默认 TX 的冲突。
