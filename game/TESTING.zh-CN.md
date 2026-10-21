# 孤侠魅影 0.2.0 — 运行层启动测试

本版会实际加载并执行用户已提供的原始游戏库，调用构造函数、JNI 初始化和渲染循环。尚未在 Switch 实机验证首帧、菜单或关卡，不应视为可玩版。

## 安装与启动

1. 安装 `孤侠魅影-运行层启动测试-0.2.0.nsp`。Title ID 与 0.1.1 前端相同，会更新同名应用；继续使用 SD 上已有的 `switch/shadowguardian_nx/` 资料。
2. 从 HOME 菜单打开“孤侠魅影”。启动测试说明画面中按 **A** 尝试启动原始游戏，按 **+** 退出测试说明。
3. 游戏测试阶段优先使用触屏。**+ / B** 发送 Android Back；同时按 **+ 和 −** 结束测试。
4. 若能进入游戏，先检查画面、触摸响应和声音，再测试进入菜单或关卡。手柄的虚拟触摸映射仍是实验实现，不能视为已适配所有游戏场景。

## 原始资料位置

- `sd:/switch/shadowguardian_nx/libshadowguardian.so`，也支持 `lib/armeabi-v7a/libshadowguardian.so`。
- `sd:/switch/shadowguardian_nx/gameloft/games/GloftSGHP/`，也兼容原包的 `Gameloft/Games/GloftSGHP/`。

库的 SHA256 必须为 `ad17a127ae438e829a23e372ddd8100f82fe6a311b0ad295b8cead69d5805ef9`。此前 frontend.log 已确认你的资料匹配，无需重新准备 APK 或素材。

## 回传日志

启动测试现在使用运行层日志，路径均在 `sd:/switch/shadowguardian_nx/`：

- `debug.log`
- `debug.prev.log`（若存在）
- `crash.log`（若存在）

黑屏、退出或报错时，请提供上述实际存在的文件，并说明停在按 A 之前还是之后。若出现错误画面，一并提供。日志中的 `[sg/stage]` 标记会指出最后完成的阶段；每个原生构造函数也会记录偏移。

`frontend.log` 属于旧版准备界面的文件扫描报告，无法替代本版运行层日志。

## 当前接入与限制

已接入：已知原库哈希验证、ELF 重定位/导入映射、14 个 SG 原生入口绑定、JNI 对象模型、SG 专用路径/设备/资源接口、EGL/GLES2、AudioTrack PCM 输出、触摸与实验按键、暂停/恢复回调。

未完成验收：首帧、画面/纹理正确性、初始化所需 Java 接口是否完整、MediaPlayer 的音乐接口、完整控制方案、存档、休眠与长时间稳定性。未知 JNI 请求会记录日志；未冒充这些接口已经实现。

本包不含游戏库、游戏素材、APK、私有密钥。NSP 与源码中只包含兼容层和项目图标。

原库的旧式 ARM SWP 指令通过受限异常处理模拟为原子交换；其他异常仍写 crash.log。该兼容方式可能影响加载和运行速度，需要实机日志确认。

游戏启动暂用语言索引 0 的基线配置；中文语言索引尚未验证，应用名称仍为“孤侠魅影”。
