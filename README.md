# 孤侠魅影 / Shadow Guardian — Nintendo Switch

当前源码版本：**0.2.3 触屏按键恢复版**。用户确认 0.2.2 的右摇杆视角已生效，但隐藏触屏按钮影响了躲藏、换弹等操作。本版恢复原游戏全部触控按钮、虚拟摇杆、按钮高亮和计量条，保留现有物理按键及视角控制。

ARM32 交叉编译和 NSP 解包校验已通过；新版本仍需 Switch 实机验收。Title ID 为 `0100F53447550000`，继续使用已有原库、素材和存档。

## 0.2.3 更新

- 移除全部隐藏操作按钮的渲染挂钩，恢复原游戏触控界面，可直接触屏躲藏、换弹和执行其他操作。
- 保留 0.2.2 的右摇杆视角、移动、B 情境动作/菜单返回、Y 换弹、X 下一把武器等物理映射。
- 保留自动加载、关闭普通启动日志和 1280×720 全屏输出。

[完整按键映射](game/CONTROLS_PROGRESS.zh-CN.md) · [构建验证](game/BUILD_REPORT.md) · [实机验收](game/TESTING.zh-CN.md)

## 原始资料

继续使用已有文件，0.2.2 不包含原始游戏库、素材或存档：

```text
sd:/switch/shadowguardian_nx/libshadowguardian.so
sd:/switch/shadowguardian_nx/gameloft/games/GloftSGHP/
```

也支持 `lib/armeabi-v7a/libshadowguardian.so` 和 `Gameloft/Games/GloftSGHP/` 大小写。原库 SHA256：`ad17a127ae438e829a23e372ddd8100f82fe6a311b0ad295b8cead69d5805ef9`。

按键坐标对应默认触控布局；如果曾在原游戏中移动按钮，需要恢复默认布局或重新核对坐标。

## 构建

运行层使用 `Game.mk`、`game/source/` 与共享 `runtime/`。根目录旧 `Makefile/source` 和 `frontend/` 不用于当前游戏运行层。

依赖：devkitARM 16.1.0、[补丁版 libnx32](https://github.com/aks796/libnx32) `35c0b5aeb84a5456ea1419d98fd833631bda4279`、[Mesa32](https://github.com/aks796/mesa32) 的 Mesa 20 库与 miniz 3.0.2。

```sh
bash game/tests/run_host.sh
# 可选：执行用户本地原库的孤立 ARM 输入分支测试
python3 -m pip install -r game/tests/requirements-original.txt
python3 game/tests/original_input_test.py /private/path/libshadowguardian.so
export DEVKITPRO=/path/to/devkitpro
bash game/build.sh PORTLIBS=/path/to/mesa32
python3 game/tools/package_test.py --keyset /private/prod.keys --tool /path/to/hacbrewpack
python3 game/tools/verify_test.py --keyset /private/prod.keys --hactool /path/to/hactool
```

hacBrewPack 3.05 使用 [版本号补丁](game/tools/hacbrewpack-title-version.patch)，支持 `HACBREWPACK_TITLE_VERSION`，0.2.3 使用 CNMT 版本 `131075`。根目录 `shadowguardian_game.nsp` **仅为 ExeFS 容器，不能当作完整安装包**；完整安装包目标为 `build/game/孤侠魅影-0.2.3.nsp`，需独立完成 Program/Control/Meta NCA 打包与验证。

主机测试与 ARM 分支模拟不能替代 Switch 实机验收。按键坐标、渲染隐藏、菜单点击、存档、休眠与持续游玩仍需实机确认。

## 资料与许可

- [原始库与资源审计](docs/ORIGINAL_LIBRARY_AUDIT.json)
- [早期移植审计](docs/PORT_AUDIT.zh-CN.md)
- [文件准备前端](frontend/)
- [参考移植](https://github.com/MetalSyntax/Shadow-Guardian-vita)
- [AMD ATC 参考实现](https://github.com/GPUOpen-Tools/compressonator)

第三方许可保留于 `licenses/` 和相应源码目录。仓库和交付包不包含原始游戏库、资源、APK 或私有密钥。
