# FoloToy AI Passport

[English](README.md)

面向 FoloToy AI Passport ESP32-C3 硬件的 Arduino 库。用统一的板级接口访问彩色屏幕、三颗按键、麦克风、扬声器和电量计，减少重复配置引脚的工作。

当前为 `0.1.0` 首版实现。编译和主机测试结果见[验证记录](docs/validation.zh_CN.md)；尚未完成实机验收。仅支持本库记录的 ESP32-C3 AI Passport 引脚布局。

## 功能

- ST7789P3 240 × 320 屏幕，支持 Adafruit GFX 绘图接口和背光调节。
- UP、DOWN、OK 三键消抖，提供按下、释放、单击和长按事件。
- ES8311 麦克风与扬声器，使用有符号 16 位单声道 PCM。
- CW2017 电量与电压读取，不改写电池 profile。
- 各外设的独立示例，以及基于 ESP32 核心的 Wi-Fi 扫描示例。

库本身不需要云账号、API 密钥或网络连接。音频和电池默认关闭，按需初始化。被动 NFC 标签未连接到 MCU，本库不提供其读写 API。

## 环境与安装

需要 ESP32-C3、8 MB Flash 的 AI Passport、Arduino IDE 2.x 或 Arduino CLI，以及测试使用的 Arduino-ESP32 3.3.12。无需 PSRAM。

本库尚未进入 Arduino Library Manager：

1. 下载仓库 ZIP，或将克隆后的库目录压缩为 ZIP。
2. 在 Arduino IDE 选择「项目 → 导入库 → 添加 .ZIP 库」。
3. 在库管理器安装 Adafruit ST7735 and ST7789 Library、Adafruit GFX Library，并安装提示的依赖。
4. 按[乐鑫官方说明](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html)安装 esp32 开发板包。
5. 打开「文件 → 示例 → FoloToyAIPassport → HelloWorld」。

工具菜单设置：

| 设置 | 值 |
| --- | --- |
| Board | ESP32C3 Dev Module |
| Flash Size | 8 MB (64 Mb) |
| USB CDC On Boot | Enabled |
| CPU Frequency | 160 MHz |
| Erase All Flash Before Sketch Upload | Disabled |
| Port | AI Passport 的原生 USB 端口 |

示例目录中的 `partitions.csv` 必须与 `.ino` 放在一起。新项目请复制 [extras/partitions.csv](extras/partitions.csv) 到草图目录。它限制应用为 3 MB，并保留 `0x356000` 的 `cardid` 分区。

上传草图会替换现有固件；Arduino IDE 常规上传还会写入 bootloader 和分区表，并可能影响已有配置。实验前应备份已配置设备。不要对已有设备使用通用文件系统/OTA 分区表或全片擦除。本库不读取、生成或修改设备身份，也不是 meta-pass 启动器槽位的安装包。详见[硬件说明](docs/hardware.zh_CN.md)。

## 快速上手

```cpp
#include <FoloToyAIPassport.h>

folotoy::AIPassport passport;

void setup() {
  Serial.begin(115200);
  if (!passport.begin()) {
    Serial.println(passport.lastError());
    return;
  }
  passport.display.setTextColor(ST77XX_WHITE);
  passport.display.setTextSize(2);
  passport.display.setCursor(16, 24);
  passport.display.println("Hello, Passport!");
}

void loop() {
  passport.update();
  if (passport.buttons.wasClicked(folotoy::Button::Ok)) {
    passport.display.fillScreen(ST77XX_BLUE);
  }
  delay(5);
}
```

`begin()` 默认初始化屏幕和按键。用 `PassportConfig` 开启音频和电池，或在串口示例中关闭屏幕。库不创建后台按键任务；需要频繁调用 `update()`，并在下一次调用前读取事件。长时间阻塞可能漏掉按键。

```cpp
folotoy::PassportConfig config;
config.audio = true;
config.battery = true;
if (!passport.begin(config)) {
  Serial.println(passport.lastError());
}
```

生命周期、错误、总线归属、PCM 格式与时序见 [API 文档](docs/api.zh_CN.md)。

## 示例

| 示例 | 内容 |
| --- | --- |
| [HelloWorld](examples/HelloWorld/HelloWorld.ino) | 显示文字并响应 OK |
| [DisplayColors](examples/DisplayColors/DisplayColors.ino) | 检查颜色、方向和屏幕边缘 |
| [Buttons](examples/Buttons/Buttons.ino) | 查看按键事件与 ADC 电压 |
| [BatteryMonitor](examples/BatteryMonitor/BatteryMonitor.ino) | 读取电量和电压 |
| [AudioTone](examples/AudioTone/AudioTone.ino) | 播放低音量 1 kHz 测试音 |
| [MicrophoneLevel](examples/MicrophoneLevel/MicrophoneLevel.ino) | 输出麦克风峰值，不保存录音 |
| [WiFiScan](examples/WiFiScan/WiFiScan.ino) | 扫描并显示附近网络，不连接 |

## 开发与贡献

```sh
arduino-cli core update-index --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core install esp32:esp32@3.3.12 --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli lib install "Adafruit ST7735 and ST7789 Library" "Adafruit GFX Library"
arduino-lint --compliance strict --library-manager submit .
python3 tools/test_host.py
bash tools/compile_examples.sh
```

脚本编译全部 ESP32-C3 示例，启用 USB CDC 和 8 MB Flash，不执行烧录。GitHub Actions 使用同样的检查；CI 配置或编译通过不代表实机验证。

欢迎提交问题、示例和文档改进。请先阅读[贡献说明](CONTRIBUTING.zh_CN.md)，报告问题时附板卡版本、ESP32 核心版本、最小草图和脱敏串口日志，不上传凭据或设备身份数据。

## 许可

FoloToy 编写的库代码和示例采用 [MIT License](LICENSE)。内置的乐鑫 ES8311 驱动子集采用 Apache-2.0，保留其版权与许可。Adafruit 库单独安装。来源及许可详见 [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)。
