# Third-party code and notices

The root MIT LICENSE applies only to original wrapper work. Upstream components keep their own copyright and licenses. Preserve this file and `licenses/` alongside binary/source distributions.

| Component | Use / license notice |
|---|---|
| [aks796/android32](https://github.com/aks796/android32) | Runtime submodule, launcher and port OpenSL override. MIT; [notice](licenses/android32.txt). `source/opensles.c` retains upstream attribution and adds local queue/lifetime fixes. |
| [aks796/a8retry_nx](https://github.com/aks796/a8retry_nx) | Generic preference, MD5, Bundle and Java patterns adapted in `dcr_prefs.*` / `tasm2_java.c`. MIT; [notice](licenses/a8retry_nx.txt). |
| [aks796/libnx32](https://github.com/aks796/libnx32) / [switchbrew/libnx](https://github.com/switchbrew/libnx) | Static AArch32 SDK and AArch64 launcher SDK. ISC; [notice](licenses/libnx.txt). |
| [aks796/mesa32](https://github.com/aks796/mesa32) / [Mesa](https://www.mesa3d.org/) | Reused Mesa 20.1.0-rc3 GLES2/EGL/Nouveau portlibs. Mesa's permissive notices: [license overview](licenses/mesa.html) and [source notices](licenses/mesa-source-notices.txt). The latter conservatively retains copyright/license blocks from the reference tree, including families that may not be linked; it does not assert all families are linked. |
| libdrm / Nouveau support | Static graphics support from the existing portlibs. Permissive upstream source notices; [Nouveau notice](licenses/libdrm-nouveau.txt), [common notices](licenses/libdrm-notices.txt). |
| [miniz](https://github.com/richgel999/miniz) | APK ZIP/CRC support from SDK, header version 3.0.0. The installed header retains a legacy public-domain banner; the upstream MIT [notice](licenses/miniz.txt) is preserved. |
| [jsmn](https://github.com/zserge/jsmn) | Vendored original parser header `source/thirdparty/jsmn.h`. MIT; [notice](licenses/jsmn.txt). |
| newlib and SDK runtime support | Statically linked C runtime; multiple permissive notices retained in [COPYING.NEWLIB](licenses/newlib.txt). GCC support libraries are covered by [GPLv3](licenses/gcc-gpl3.txt) with the [GCC Runtime Library Exception](licenses/gcc-runtime-exception.txt); the exception applies to eligible compiled runtime code, not to unrelated GPL sources. |

Build-only tools include devkitPro's compiler/switch tools and Docker container toolchains. They are not shipped as executables in this release. See [REFERENCE_AUDIT.md](REFERENCE_AUDIT.md) for the materials inspected in this project. Original binary-derived ABI facts and guarded instruction values describe compatibility; no original game native library or gameplay data is included.

The launcher artwork (`launcher/icon.jpg` and `frontend/assets/icon.jpg`) is derived from the user-provided Shadow Guardian image. Original game imagery and trademarks belong to their respective rights holders and are not covered by the source code license. No endorsement is implied.

The experimental SG runtime uses native ABI/initialization and input-convention references from [MetalSyntax/Shadow-Guardian-vita](https://github.com/MetalSyntax/Shadow-Guardian-vita). Its MIT notice is retained in [licenses/shadowguardian-vita-MIT.txt](licenses/shadowguardian-vita-MIT.txt). See game/BUILD_REPORT.md for the exact reference revision and test limits.
