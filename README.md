# Shadow Guardian / 孤侠魅影 — Switch

**当前新增：0.2.0 原生运行层启动测试 NSP，已接入原始游戏库的构造函数、JNI 和渲染循环调用。尚未通过实机首帧/游戏验证，不是已验证可玩版。**

[0.2.0 实机测试步骤与日志位置](game/TESTING.zh-CN.md) · [运行层构建报告](game/BUILD_REPORT.md)

0.1.1 文件准备前端保留在 `frontend/`，下文的四页界面和 NRO 说明描述该准备前端。0.2.0 使用独立 AArch32 启动测试程序，按 A 执行启动；实际素材继续使用原目录。

运行层构建：安装 devkitARM、补丁版 libnx32 和 mesa32/miniz 后，执行 `bash game/build.sh PORTLIBS=/path/to/mesa32`。打包工具为 `game/tools/package_test.py`，不嵌入原始游戏文件。

2026-10-21 对当前仓库和用户已提供的 PortMaster 原始资料进行了核对。当前游戏运行代码由 旧模板 改造而来，原始 Shadow Guardian 库的 JNI 入口与现有启动流程不匹配。旧版 旧模板 的实机记录不能视为本游戏的测试结果。

## 前端功能

- 原生 AArch64 / libnx，1280 × 720 深色金色界面。
- 概览、游戏文件、安装指南、诊断四个页面。
- 手柄 L/R 或方向键切页，A 查看文件，B 返回首页，Y 重扫，X 导出日志，+ 退出。
- 触屏选择侧栏页面；文件检查支持原始 PortMaster 素材。
- 按 SHA256 识别已提供的 `libshadowguardian.so`，统计素材文件与体积。
- 识别 `gameloft/games/GloftSGHP` 和 `Gameloft/Games/GloftSGHP` 两种目录大小写。
- 扫描不修改 APK、游戏库、素材或存档；X 仅写入 `frontend.log`。

文件存在、库哈希匹配和素材统计，均不代表游戏已经可运行。前端不启动游戏，也不执行游戏原生代码。

## 安装与资料

安装 `Shadow-Guardian-Frontend-0.1.1.nsp` 后可从 HOME 图标打开前端；也提供 NRO 以便从 Homebrew Menu 打开。无需 Sphaira。实际安装、显示、输入和休眠行为仍需 Switch 真机验证。

将你自己的原始资料放在 `sd:/switch/shadowguardian_nx/`：

- `libshadowguardian.so`，也支持 `lib/armeabi-v7a/libshadowguardian.so`。
- `gameloft/games/GloftSGHP/`，也接受原包的 `Gameloft/Games/GloftSGHP/`。

现有 PortMaster ZIP 可用 Python 3 的标准库整理，无需重新提供 APK：

```sh
python3 frontend/tools/prepare_data.py ShadowGuardian-Original-PortMaster.zip --output prepared/shadowguardian_nx --dry-run
python3 frontend/tools/prepare_data.py ShadowGuardian-Original-PortMaster.zip --output prepared/shadowguardian_nx
```

准备脚本只接受已审计库，复制游戏库和素材到空目录；不会复制 Linux 程序、驱动、存档或日志。整理文件夹不是游戏运行器。若已有安装，请先备份再合并。

## 构建前端

需要 devkitA64、libnx、switch-tools、Python 3，打包 NSP 另需 hacBrewPack 和私有本地 keyset。构建前端不需要原始游戏文件，也不需要 32 位运行层。

```sh
export DEVKITPRO=/path/to/devkitpro
bash frontend/build.sh
python3 frontend/tools/package_nsp.py --keyset /private/prod.keys --tool /path/to/hacbrewpack --out frontend/build
python3 frontend/tools/verify.py --keyset /private/prod.keys --hactool /path/to/hactool
python3 frontend/tools/test_scan.py
```

宿主文件检测测试需要 C 编译器和 OpenSSL 开发库；Switch 使用 libnx 的 SHA256，不依赖 OpenSSL。字体和图标已生成并提交，仅重新生成字体时需要 Pillow。前端当前使用英文 ASCII 文本。

`frontend/build.sh` 生成的是前端 NRO 和原生可执行程序；`package_nsp.py` 生成含 Program、Control、Meta NCA 的实际可安装 NSP。不要把根目录 AArch32 运行层的 ExeFS 容器当成此安装包。

## 研究与验证

- [本次移植研究](docs/PORT_AUDIT.zh-CN.md)
- [原始库接口和素材审计](docs/ORIGINAL_LIBRARY_AUDIT.json)
- [本次构建报告](docs/FRONTEND_BUILD.md)
- [上游 旧模板 说明存档](docs/UPSTREAM_旧模板_README.md)

根目录旧 `BUILD_REPORT.md`、`VALIDATION.md`、`TECHNICAL_NOTES.md` 等保留为上游历史参考，不是 Shadow Guardian 的验收证据。

## 许可与致谢

保留原始 旧模板 / android32 / libnx 等贡献者署名和许可证，见 `LICENSE`、`THIRD_PARTY.md`、`licenses/`。前端新代码使用 MIT；DejaVu 字体许可见 `frontend/assets/FONT-LICENSE.txt`。图标基于用户提供的《孤侠魅影》图片制作。交付包不含原始游戏库、游戏素材或任何私有密钥。
