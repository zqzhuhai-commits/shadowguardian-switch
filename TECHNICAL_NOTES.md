# 孤侠魅影技术说明

独立 AArch64/libnx 前端位于 `frontend/`，游戏原始库为 ELF32 ARM。两者架构不同。当前前端不执行原始游戏库。运行层尚需按原库导出重新接入 JNI、图形、输入、音频与生命周期，见 [移植研究](docs/PORT_AUDIT.zh-CN.md)。
