# 贡献说明

[English](CONTRIBUTING.md)

欢迎改进 FoloToyAIPassport。请保持修改集中、便于审查。报告问题时说明问题、预期行为和最小草图；硬件问题请注明具体板卡版本、Arduino-ESP32 版本和脱敏错误输出。

提交 PR 前：

1. 引脚集中于 `src/PassportPins.h`，示例保持简短可运行。
2. 遵循现有 C++ 风格，记录公共 API 变化。
3. 中英文文档同步更新。
4. 执行严格 Arduino Lint、`python3 tools/test_host.py` 和 `bash tools/compile_examples.sh`，依赖安装见 README。
5. 区分编译、主机测试与实机测试。硬件修改发布稳定版本前，需要测量受影响外设。
6. 保留许可证和上游版权；新增第三方代码或素材应注明来源。

不提交构建产物、凭据、原始 Flash 备份、设备身份分区或私人日志。讨论应尊重他人、善意协作，围绕可复现技术问题展开。敏感安全问题通过 [FoloToy 安全联系渠道](https://folotoy.com/security/)报告，不放在公开 issue。

## 准备公开发布

仓库初始为私有。面向社区发布前应完成并记录 `docs/validation.md` 的实机验收，确认支持硬件版本的引脚；仓库公开可访问后，将 `library.properties` 中指向公开板级资料的 URL 更新为本库地址。检查通过后更新版本、日志并建立对应 tag。

Arduino Library Manager 的收录需要另行申请；索引真正包含本库之前，不宣称已经收录。
