# Runtime 0.2.1 — 2026-10-21

The original 0.2.0 hardware log confirms 214 resolved non-GL imports, 215 completed constructors, working Mesa EGL/GLES2 initialization and the native bootstrap, 40 executed SWP exchanges, and three returned render calls. It then reports unsupported ATC uploads, null JNI method IDs, missing sprite chunks and a null sprite write at `LoadSpritesMenu+0x74` (ELF offset `0x1b40fc`). This does not establish a playable game.

## Changes

- Removed the incorrect main-branch ETC-style ATC decoder. The SG port GL hook now decodes RGB555/RGB565 ATC palettes (both modes), explicit nibble alpha and interpolated alpha; correct constants are 0x8c92, 0x8c93, 0x87ee. Decoder validates sizes/overflow/null input before access, clips edge blocks, handles unaligned input and uses GLES2-compatible unsized GL_RGBA uploads. Compatibility is independent of RT_GL_CHECK.
- Native bootstrap supplies the real declaring jclass for ShadowGuardian, GLResLoader, GLUtils/Device and GameRenderer. Original disassembly shows NewGlobalRef then GetStaticMethodID on these arguments; passing NULL cached NULL methods in 0.2.0.
- ADevice::Init's obfuscated a/b/c/d/f methods use byte arrays; unavailable phone metadata returns a NUL-terminated empty byte array instead of a String or NULL. Added Context.TELEPHONY_SERVICE and basic no-SIM telephony answers. No phone identity or service success is fabricated.
- Resource/UI coordinates use 800x480, matching the supplied sprites_1_6 and gui_1_6 packs. EGL window is 1280x720; port GL hooks scale only default-framebuffer viewport/scissor, maintain logical query results and reapply geometry across FBO switches. Offscreen image dimensions remain unchanged. Touch maps 1280x720 coordinates back to the logical space. Fullscreen maps the original UI to 16:9 with slight stretching; original asset detail is unchanged.
- Validate both pack chunk counts and the menu sprite chunk signature before calling native code. No crash-suppressing binary patch was applied.
- NACP version 0.2.1; CNMT version 131073; title ID, all 16 Chinese title slots and project icon retained.

## Validation

- Production decoder golden vectors: both palette modes, both alpha modes, selector ordering including cross-byte selectors, 3x5 edge clipping, unaligned inputs, short/null/unknown format and extreme dimensions. ASan/UBSan passed.
- 50,000 deterministic random RGB blocks compare byte-for-byte with the actual AMD Compressonator `atiDecodeRGBBlockATITC` implementation. Both normal and black modes covered. Alpha paths have independent golden vectors; no claim of GPU-driver or console validation.
- Production GL wrapper tests: 800x480 -> 1280x720, logical viewport queries, scissor scaling, FBO switch/reapplication, unchanged offscreen sizes, invalid dimensions, successful ATC-to-RGBA upload, malformed-input rejection and non-ATC passthrough. ASan/UBSan passed.
- Bootstrap tests verify seven string object identities, four declaring class objects, 800x480 native dimensions, lifecycle order and missing-class failure. Existing PCM and SWP host regression tests passed under ASan/UBSan.
- devkitARM GCC 16.1.0 / libnx32 35c0b5aeb84a5456ea1419d98fd833631bda4279 / Mesa 20 / miniz 3.0.2: ELF32 ARM cross-build and link passed. NSO segments and hashes exactly match ELF PT_LOAD contents. Inherited runtime format warnings remain; this build adds no suppression for them.
- NSP extraction verification is performed by game/tools/verify_test.py before delivery: Program/Control/Meta, exact executable, AArch32 NPDM, title ID, all names/icon/version, CNMT content hashes, and STATUS-only RomFS.

## Remaining verification

0.2.1 has not been run on Switch. Menu loading, image correctness, gameplay, music, complete controls, saves, suspend/resume and stability remain open. The old unsupported-language and missing(SC,...) warnings have not been declared fixed. Resource profile behavior and JNI completeness must be confirmed from the new log; removing the texture error alone does not prove that the menu crash is gone.

No original game data or private key is distributed. See licenses/shadowguardian-vita-MIT.txt and licenses/AMD-Compressonator-MIT.txt for reference notices.
