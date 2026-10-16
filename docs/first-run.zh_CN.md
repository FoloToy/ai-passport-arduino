# AI 首次使用仓库的复现检查

[English](first-run.md)

日期：2026-10-16。范围：按 `AGENTS.md` 复现公开克隆、工具安装、隔离的 Arduino 环境、
lint、主机测试与示例编译。本次没有烧录或测试设备。

## 发现与修正

| 发现 | 修正 |
| --- | --- |
| 指南假设已有 Arduino CLI 和 Arduino Lint。 | 补上官方固定版本安装器、前置工具、PATH 和版本检查。 |
| 可能隐式使用已有 IDE 数据和构建缓存。 | 补上全新的数据、下载、草图、缓存目录及配置检查。 |
| curl/Git 的代理设置没有配置 Arduino CLI 1.5.1 的下载代理。 | 说明 `ARDUINO_NETWORK_PROXY` 及需要访问的站点。 |
| `arduino-lint --compliance strict --library-manager submit .` 将 `.` 当作目录名，触发 LS003。 | 实际复现失败，再用仓库绝对路径验证通过；同步修正中英文指南和 README。 |
| 实际分区为应用保留 3 MB，但 CLI 仍按 1,310,720 字节检查程序大小。 | 在编译脚本和单示例命令中设置 `upload.maximum_size=3145728`，保留实际镜像与分区校验。 |
| 已在仓库中的读者可能再次在内部执行克隆。 | 说明何时复用已有克隆、何时从父目录克隆。 |

核心的默认板卡与容量设置已对照
[Arduino-ESP32 3.3.12 boards.txt](https://github.com/espressif/arduino-esp32/blob/3.3.12/boards.txt)。
IDE 仍可能沿用默认的 1.25 MB 限制；较大的草图可使用文档中的 CLI 流程，
同时保留提供的自定义分区文件。

## macOS 工具准备与主机检查

在 Apple Silicon Mac 上，以 `68b30c3` 的全新公开 HTTPS 克隆开始，
不使用预装的 Arduino 核心或草图库。Arduino 目录与程序均放在本次专用临时目录。
版本：Arduino CLI 1.5.1、Arduino Lint 1.3.0、Python 3.9.6、Apple Clang 17.0.0。

| 检查 | 结果 |
| --- | --- |
| 公开 HTTPS 克隆 | PASS |
| 官方 CLI/Lint 安装器和版本检查 | PASS |
| 隔离的 CLI 配置 | PASS |
| 显式设置 CLI 代理后下载索引 | PASS |
| 使用绝对路径的严格 lint | PASS：无错误、无警告 |
| C++11 主机测试与示例分区检查 | PASS |
| 完整 ESP32 核心下载和安装 | STOPPED：网络传输缓慢，已清理未完成的下载 |
| macOS 示例编译 | NOT RUN |

macOS 固件编译仍未验证。为减少下载等待，曾并行下载部分官方压缩包，
并按索引校验 SHA-256；这没有完成 macOS 核心安装。
保留小型检查日志后，已清理临时工具、缓存、下载和克隆目录。

## Linux 全新环境验证

使用仓库现有 GitHub Actions 工作流，在全新 Ubuntu 24.04 runner 上安装
Arduino CLI 1.5.1、ESP32 核心 3.3.12、Adafruit ST7735/ST7789 1.11.0、
Adafruit GFX 1.12.6 和 Adafruit BusIO 1.17.4，再执行仓库检查。

[首次运行，68b30c3](https://github.com/FoloToy/ai-passport-arduino/actions/runs/38847766565)
的两个任务和全部九个示例均通过，日志也暴露了错误的默认应用大小检查。
[修正脚本后运行，45d14a5](https://github.com/FoloToy/ai-passport-arduino/actions/runs/38848507754)
单独验证容量设置修正：

| 检查 | 结果 |
| --- | --- |
| 核心与固定版本依赖安装 | PASS |
| 严格 lint 与主机测试 | PASS |
| 九个示例编译，警告视为错误 | PASS |
| ESP32-C3 / 8 MB 镜像头和受保护分区二进制 | PASS |
| CLI 应用上限为 3,145,728 字节 | PASS |
| 真机测试 | NOT RUN |

九个示例为 AudioTone、BatteryMonitor、BoardSelfTest、Buttons、DisplayColors、
HelloWorld、MicrophoneLevel、PushToTalk 和 WiFiScan。
`tools/verify_build.py` 检查实际应用和分区二进制，包括 3 MB factory 分区与
`cardid` 保留区域；编译本身不能证明硬件行为或语音质量。

本次不覆盖原生 Windows 主机测试、实物验收或原固件恢复。
真机证据另见带日期的[设备验证记录](validation.zh_CN.md)。
