<p align="right"><a href="AGENTS.md">English</a> · <strong>简体中文</strong></p>

# AI 仓库使用与开发指南

使用本库编写草图或维护库代码时，先读本文件，再按下表读取当前任务所需的文档。
除非特别说明，以下命令均在仓库根目录执行。

## 定位与事实来源

- 本仓库是 **FoloToyAIPassport** Arduino 外设库与可运行示例，支持
  **ESP32-C3 AI Passport、8 MB Flash、无 PSRAM**。它不是开发板管理器包、
  完整产品固件或启动器槽位安装器。
- 公开入口为 `src/FoloToyAIPassport.h`，命名空间为 `folotoy`。
  可复用驱动放在 `src/`；应用 UI 与业务流程放在 `examples/<SketchName>/`
  或用户自己的草图中。
- 开始时运行 `git status --short --branch`，保留与当前任务无关的用户修改。
- 当前 API 行为以本次检出的头文件和实现为准，再查 `docs/api.zh_CN.md`
  和对应示例。硬件常量集中在 `src/PassportPins.h`，不要在新草图中散布 GPIO 数字。
- 硬件事实参考 [AI Passport 官方仓库](https://github.com/FoloToy/ai-passport)，
  尤其是[产品规格](https://github.com/FoloToy/ai-passport/blob/main/docs/hardware-design/specifications.md)
  和 [BSP 引脚](https://github.com/FoloToy/ai-passport/blob/main/components/bsp/include/bsp_pins.h)。
  对照 `docs/hardware.zh_CN.md` 记录的适用板型和参考提交；发现来源冲突时，先确认差异，
  再修改引脚或电气设置，不要猜测。
- 本指南的组织方式参考官方 [AI 指南](https://github.com/FoloToy/ai-passport/blob/main/AGENTS.md)
  和[小葵指南](https://github.com/FoloToy/sunflower-buddy/blob/main/AGENTS.md)。
  其中的 ESP-IDF 命令、LVGL API、强制技能安装和其他板型的引脚不适用于本 Arduino 库。

## 按任务读取

| 任务 | 优先阅读 |
| --- | --- |
| 第一个草图或 IDE 配置 | `README.zh_CN.md`、`examples/HelloWorld/HelloWorld.ino` |
| 公开 API 或生命周期 | `docs/api.zh_CN.md`、`src/FoloToyAIPassport.h`、相关驱动头文件和实现 |
| 屏幕或 UI | `src/PassportDisplay.h`、`src/PassportDisplay.cpp`、`examples/DisplayColors/DisplayColors.ino` |
| 按键或时序 | `src/PassportButtons.cpp`、`src/detail/ButtonState.h`、`tests/test_logic.cpp` |
| 音频或录音 | `docs/recording.zh_CN.md`、`src/PassportAudio.h`、`src/PassportAudio.cpp`、`examples/PushToTalk/` |
| 电量或共享 I2C | `docs/battery.zh_CN.md`、`src/PassportBattery.cpp`、`src/PassportPins.h` |
| 硬件、分区或上传 | `docs/hardware.zh_CN.md`、`extras/partitions.csv`、`tools/verify_build.py` |
| 检查或验收 | `.github/workflows/ci.yml`、`tools/compile_examples.sh`、`tools/test_host.py`、`docs/validation.zh_CN.md` |
| 贡献或发布 | `CONTRIBUTING.zh_CN.md`、`THIRD_PARTY_NOTICES.zh_CN.md`、`library.properties`、`CHANGELOG.zh_CN.md` |

## 环境与编译

使用 Arduino IDE 2.x 或 Arduino CLI。当前仓库 CI 基线为 Arduino CLI 1.5.1、
Arduino-ESP32 **3.3.12**；修复无关问题时不要顺带升级核心。
主机测试需要 Python 3 和 C++11 编译器（默认 `c++`，可通过 `CXX` 指定）。
本库通过本地检出目录或 ZIP 安装，不要假设已被 Library Manager 收录。

### 首次准备 CLI

下面的命令和脚本需要 Bash、Git、curl、tar、Python 3 和 C++11 编译器。
macOS 上若 `c++ --version` 失败，先用 `xcode-select --install` 安装苹果命令行工具。
Debian/Ubuntu 对应的软件包为 `build-essential python3 git curl ca-certificates`，
按系统包管理器和权限要求安装。Windows 可用 WSL 执行本套 shell 与主机测试流程，
或按 `README.zh_CN.md` 配置 Arduino IDE；原生 Windows 主机测试尚未验证。

已有克隆时直接使用现有目录，从下面的状态和版本检查开始，不要在仓库内部再克隆一份。
尚未克隆时，从父目录执行下面的克隆和 `cd` 命令。
公开克隆不需要 GitHub 账号或私有仓库权限：

```sh
git clone https://github.com/FoloToy/ai-passport-arduino.git
cd ai-passport-arduino
git status --short --branch
python3 --version
c++ --version
```

若当前没有对应版本，用官方安装器安装 Arduino CLI 1.5.1 和 Arduino Lint 1.3.0。
以下命令将程序放在用户目录，无需 `sudo`，也不修改 shell 启动文件：

```sh
mkdir -p "$HOME/.local/bin"
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/v1.5.1/install.sh | BINDIR="$HOME/.local/bin" sh -s 1.5.1
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-lint/1.3.0/etc/install.sh | BINDIR="$HOME/.local/bin" sh -s 1.3.0
export PATH="$HOME/.local/bin:$PATH"
arduino-cli version
arduino-lint --version
```

各平台的下载方式见官方 [CLI 安装指南](https://arduino.github.io/arduino-cli/1.5/installation/)
和 [Lint 安装指南](https://arduino.github.io/arduino-lint/latest/installation/)。
替换已有程序前先检查；其他项目需要不同版本时，改用独立的安装目录。
每次打开新 shell 都要将安装目录加入 `PATH`，或使用程序绝对路径。
`ARDUINO_CLI` 只影响 `compile_examples.sh` 选择的 CLI 程序。

首次复现时，使用全新的 Arduino 数据、下载和草图目录，避免影响已有 IDE 环境。
后续 CLI 命令必须在保留这些环境变量的同一 shell 中执行：

```sh
export PASSPORT_ARDUINO_ENV="$(mktemp -d "${TMPDIR:-/tmp}/passport-arduino.XXXXXX")"
export ARDUINO_DIRECTORIES_DATA="$PASSPORT_ARDUINO_ENV/data"
export ARDUINO_DIRECTORIES_DOWNLOADS="$PASSPORT_ARDUINO_ENV/downloads"
export ARDUINO_DIRECTORIES_USER="$PASSPORT_ARDUINO_ENV/sketchbook"
export ARDUINO_BUILD_CACHE_PATH="$PASSPORT_ARDUINO_ENV/cache"
export ARDUINO_UPDATER_ENABLE_NOTIFICATION=false
arduino-cli config dump
```

确认输出的目录均指向新环境。安装前检查剩余磁盘空间：ESP32 核心 3.3.12
会安装多种芯片的工具和库，即使本项目只使用 C3，也要为下载、解压、缓存和构建预留数 GB。
需要能访问 GitHub、Arduino 下载站和 Espressif 包下载站。
Arduino CLI 1.5.1 使用独立的 `network.proxy` 设置，
只为 curl/Git 设置 `HTTPS_PROXY` 不会配置 CLI 下载代理。
环境需要 HTTP 代理时，在同一 shell 中将 `ARDUINO_NETWORK_PROXY` 设为实际可用的地址；
例如确有代理监听本机 7890 端口时，可执行
`export ARDUINO_NETWORK_PROXY=http://127.0.0.1:7890`。
使用当前环境允许的代理，不要将代理凭据写入日志或仓库。
详见官方 [CLI 配置说明](https://arduino.github.io/arduino-cli/1.5/configuration/)。

### 核心、依赖和检查

安装与 CI 一致的依赖：

```sh
arduino-cli core update-index --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core install esp32:esp32@3.3.12 --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli lib install "Adafruit ST7735 and ST7789 Library@1.11.0" "Adafruit GFX Library@1.12.6" "Adafruit BusIO@1.17.4"
```

用 `arduino-cli version`、`arduino-cli core list` 和 `arduino-cli lib list`
确认已安装版本。优先使用仓库脚本中的板卡选项：

```sh
arduino-lint --compliance strict --library-manager submit "$PWD"
python3 tools/test_host.py
bash tools/compile_examples.sh
```

Arduino Lint 应传入仓库绝对路径：1.3.0 会将字面量参数 `.` 当作库目录名，
即使仓库有效，也会报告 LS003。

`compile_examples.sh` 将全部示例的编译警告视为错误，再通过 `tools/verify_build.py`
检查实际生成的应用与分区二进制。使用的 FQBN 为
`esp32:esp32:esp32c3:CDCOnBoot=cdc,FlashSize=8M,CPUFreq=160`，
产物放在 `build/<SketchName>/`，不执行上传。脚本显式设置
`upload.maximum_size=3145728`，与 3 MB factory 分区一致；否则即使存在自定义分区文件，
核心默认的 1.25 MB 大小检查仍可能提前拒绝较大的草图。
可用 `ARDUINO_CLI` 指定 CLI 程序。
迭代时只编译一个示例：

```sh
arduino-cli compile --fqbn 'esp32:esp32:esp32c3:CDCOnBoot=cdc,FlashSize=8M,CPUFreq=160' --library "$PWD" --warnings all --build-path "$PWD/build/HelloWorld" --build-property upload.maximum_size=3145728 --build-property compiler.cpp.extra_flags=-Werror --build-property compiler.c.extra_flags=-Werror examples/HelloWorld
python3 tools/verify_build.py build/HelloWorld HelloWorld
```

缺少工具时先按安装说明准备，再运行版本检查后继续。
若安装受阻，准确报告失败命令和缺少的工具，继续完成可独立执行的检查，不要声称通过。
当前 CI 的路径过滤会跳过仅修改 Markdown 的 push/PR；没有运行记录不等于检查成功。

## 开发应用的顺序

1. 从最小的相关示例开始。新示例目录名与 `.ino` 文件名必须一致，
   并将 `extras/partitions.csv` 复制到草图旁。
2. 引入 `<FoloToyAIPassport.h>`，每块板创建一个 `folotoy::AIPassport`。
   `begin()` 默认开启屏幕和按键；需要时再启用 `PassportConfig::audio` 或 `battery`。
   检查 `begin()` 返回值，失败时输出 `lastError()`。
3. 大约每 5–10 ms 调用 `passport.update()`，在下一次调用前读取事件。
   用短任务片段和状态机保持按键响应。
4. 应用行为放在驱动类之外。按实际需求设计 UI，自检页或录音页只是示例，
   不是必须沿用的应用外壳。
5. 验证受影响的行为，记录仍需真机检查的项目。

## 运行时约束

- API 不可重入。生命周期和 I/O 必须串行；修改板级配置前先调用 `end()`。
  不要在音频读写阻塞时并发关闭音频。
- 屏幕使用 Adafruit GFX、RGB565、默认竖屏 240 × 320，本库不分配帧缓冲。
  用 `fillRoundedScreen()` 或绘制后调用 `applyCornerMask()` 保持 30 px 黑色圆角遮罩；
  文字和控件保留 24–30 px 安全边距。GFX 绘制不会自动受遮罩裁剪。
- Adafruit 内置字体不包含中文字形。加入中文 UI 前须提供兼容的字体或渲染方案，
  并验证实际显示；UTF-8 字符串和编译成功不能证明中文可显示。
- 三键共用一个 ADC 电阻梯，不能独立识别同时按下的多个键。
  事件标志保留到下一次 `update()`，查询不会消费事件；长按不会同时产生单击。
  中断和回调中的工作必须有界。
- 音频为有符号 16 位单声道 PCM，`read`/`write` 数量单位是**采样点，不是字节**。
  从 16 kHz 开始，处理部分传输和错误。本库不内置文件解码、语音识别或回声消除。
  在小块音频之间轮询按键，不在中断或回调中执行音频 I/O。
- 明确计算 RAM：16 kHz、每点两字节的三秒录音占 96,000 字节，
  还需另计驱动、UI 和无线协议栈。本板没有 PSRAM，避免无界缓冲。
- 音频和 CW2017 共用 GPIO10/7、100 kHz 的 Wire，不要创建第二个总线所有者，
  也不要在传输过程中修改频率。单独使用驱动前由调用者初始化 Wire。
  `end()` 不关闭共享 SPI/Wire 总线。
- 电量读取可能返回 `-1`，应显示不可用而不是 0%。电量计驱动只读，
  不配置 profile、不唤醒电量计，也不推断充电状态；不要隐式加入 profile 写入或声称 SOC 准确。
- GPIO21 用于背光，串口输出应使用 USB CDC，不能占用 UART0 TX。
  GPIO18/19 用于 USB。被动 NFC 标签没有 MCU API，硬件电源键也不同于三颗 ADC 功能键。

## Flash 与真机边界

- 每个示例的 `partitions.csv` 必须与 `extras/partitions.csv` 字节一致。
  factory 应用从 `0x10000` 开始，限制为 `0x300000` 字节；
  `cardid` 保留在 `0x356000`，大小 `0x4000`。
  普通草图开发不要读取、生成、公开或覆盖设备身份数据。
- 标准 Arduino 上传不只写应用，还可能替换启动器、bootloader、分区元数据和已有配置。
  仅保留地址不代表与设备现有分区兼容。上传前遵循 `docs/hardware.zh_CN.md`
  及用户对备份和数据的要求。
- 不套用通用 OTA/文件系统分区、全片擦除或启动器槽位安装方式。
  即使只写 `0x10000` 的应用，也要先核对设备现有布局，不能视为通用安全方案。
- 检测到设备不等于获得烧录授权。上传、擦除、恢复、发布固件或发送消息前，
  先确认属于任务授权范围；编译和主机检查不应依赖设备连接。

## 修改与交付

- 保持改动聚焦，同步更新受影响的公开 API 文档和示例。
  英文 `.md` 与简体中文 `.zh_CN.md` 配对维护，保留双向语言链接，本指南也一样。
  遵循已有 C++ 格式。
- 将可移植逻辑放在可测试的辅助模块中。修改阈值、时序、状态转换或 PCM 处理时，
  增加有意义的主机覆盖；不要为重复文档内容而添加测试。
- 保留 `src/vendor/`、`LICENSES/` 和 `THIRD_PARTY_NOTICES.zh_CN.md` 中的许可与来源。
  不提交构建产物、凭据、Flash 备份、身份分区、录音或未脱敏日志。
- 代码修改交付前运行严格 lint、主机测试和全部示例编译。
  仅修改文档时，核对路径、命令与脚本/CI、中英文一致性，并说明未执行的程序检查。
- `docs/validation.zh_CN.md` 是特定日期和版本的证据，不能把早期的部分真机验证
  当作当前修改的验收。屏幕初始化不能回读面板成功，音频采样数量不能证明语音质量。
  单独记录真机观察。
- 在用户授权范围内提交和推送。标签、版本发布、Library Manager 收录和社区发布
  需要对应的任务流程；仓库公开或编译成功不代表这些状态已完成。

交付时说明行为变化、相关文件，并分别报告：

```text
Arduino Lint：PASS / FAIL / NOT RUN
主机测试：PASS / FAIL / NOT RUN
示例编译及镜像/分区检查：PASS / FAIL / NOT RUN
真机测试：PASS / FAIL / NOT RUN
未验证项：剩余检查及缺少的工具或硬件
GitHub 交付：本地修改 / 已推送分支 / PR / 已合并，适用时附链接
```
