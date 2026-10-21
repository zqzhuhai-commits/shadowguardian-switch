# 孤侠魅影 / Shadow Guardian — Nintendo Switch

当前版本：**0.2.1 运行层修复测试版**。游戏输出窗口和主画布视口为 **1280×720 全屏**，原始资源与触摸逻辑使用 800×480 坐标，通过兼容层映射到整个屏幕。现有素材保留原始清晰度，UI 会随全屏映射略作拉伸。

**本版已完成编译和主机测试，仍需 Switch 实机验证。尚未确认可进入菜单或关卡，不是正式可玩版。**

[安装与实机测试](game/TESTING.zh-CN.md) · [构建与验证报告](game/BUILD_REPORT.md)

## 0.2.1 更新

- 正确解码 ATC RGB、显式 Alpha、插值 Alpha 三种纹理格式，再上传 RGBA 像素；修正此前主分支的格式常量、颜色算法、Alpha 解码及边界问题。
- 纹理兼容挂接在游戏 GL 接口中，关闭 GL 调试检查时也生效。
- 为原生初始化传入实际的 Java 类对象，修正资源加载器、设备接口缓存空方法 ID 的原因。
- 补齐原库 `GLUtils/Device` 的字节数组接口与 `TELEPHONY_SERVICE`；Switch 不提供手机 IMEI、SIM 卡或手机号。
- 使用与 `sprites_1_6 / gui_1_6` 数据对应的 800×480 资源坐标。默认帧缓冲的视口与裁剪映射到 1280×720，离屏帧缓冲保持自身尺寸；触摸坐标反向映射。
- 启动前检查两个资源包的块数量及菜单精灵第 27 块签名，缺失或不匹配时停止启动并记录日志。
- 保留名称“孤侠魅影”和统一项目图标；NSP 的全部 16 个语言名称槽一致。

0.2.0 实机日志已确认：原始库哈希匹配、214 个非 GL 导入全部解析、215 个构造函数完成、EGL/GLES2 和原生初始化完成，前三次渲染返回。随后出现 ATC 格式错误、JNI 空方法 ID，以及 `LoadSpritesMenu+0x74` 的空指针访问。0.2.1 针对这些线索修复兼容层；**是否消除菜单崩溃须由新日志确认**。原日志中的语言警告、`missing(SC,...)` 文本警告也需继续核对。

## 安装

1. 安装 `孤侠魅影-运行层启动测试-0.2.1.nsp`，Title ID 为 `0100F53447550000`，更新已有同名应用。
2. 保留 SD 卡上的原始资料：

```text
sd:/switch/shadowguardian_nx/libshadowguardian.so
sd:/switch/shadowguardian_nx/gameloft/games/GloftSGHP/
```

也支持 `lib/armeabi-v7a/libshadowguardian.so` 和原包的 `Gameloft/Games/GloftSGHP/` 大小写。已通过文件检查的原始资料无需重复准备。

原始库 SHA256：`ad17a127ae438e829a23e372ddd8100f82fe6a311b0ad295b8cead69d5805ef9`。

3. 从 HOME 打开“孤侠魅影”，按 **A** 尝试启动。测试中 **+ / B** 发送返回键，**+ 和 − 同时按下**结束测试。优先检查触屏、菜单和声音，再测试关卡。
4. 回传 `sd:/switch/shadowguardian_nx/` 内的 `debug.log`、`debug.prev.log`、`crash.log`（存在的文件），并说明停止位置。旧的 `frontend.log` 只记录文件扫描。

## 构建

运行层为 AArch32，入口是 `Game.mk` 和 `game/source/`。根目录旧 `Makefile/source` 不用于当前运行层。准备前端在 `frontend/`，是独立 AArch64 NRO；它负责文件检查，不运行游戏。

依赖：devkitARM、补丁版 [libnx32](https://github.com/aks796/libnx32)、[Mesa32](https://github.com/aks796/mesa32) 的 Mesa 20 库，以及 miniz 3.0.2。

```sh
export DEVKITPRO=/path/to/devkitpro
bash game/tests/run_host.sh
bash game/build.sh PORTLIBS=/path/to/mesa32
python3 game/tools/package_test.py --keyset /private/prod.keys --tool /path/to/hacbrewpack
python3 game/tools/verify_test.py --keyset /private/prod.keys --hactool /path/to/hactool
```

打包工具需要支持 `HACBREWPACK_TITLE_VERSION` 环境变量的 hacBrewPack，0.2.1 使用 CNMT 版本 `131073`。根目录编译生成的 `shadowguardian_game.nsp` 仅是 ExeFS 容器；可安装的完整测试 NSP 在 `build/game/`。

主机测试验证 JNI 初始化参数、ATC 解码、720p 视口/离屏状态、音频转换与 SWP 指令兼容边界，不能替代实机游戏验证。音乐、完整按键、存档、休眠与长时间稳定性仍待验收。

## 资料与许可

- [原始库与资源审计](docs/ORIGINAL_LIBRARY_AUDIT.json)
- [早期移植审计](docs/PORT_AUDIT.zh-CN.md)
- [0.1.1 文件准备前端](frontend/)
- [参考移植](https://github.com/MetalSyntax/Shadow-Guardian-vita)
- [AMD ATC 参考实现](https://github.com/GPUOpen-Tools/compressonator)

第三方许可保留在 `licenses/` 和相应源码目录。仓库和测试 NSP 不包含原始游戏库、游戏资源、APK 或私有密钥。
