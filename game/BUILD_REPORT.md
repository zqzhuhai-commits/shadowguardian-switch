# Runtime 0.2.0 — 2026-10-21

The new AArch32 runtime builds from `game/source` with `Game.mk`; legacy game-specific source files are not linked. It uses the original supplied native library only at execution time, after matching its exact size and SHA256.

Build: devkitARM GCC 16.1.0, libnx32 `35c0b5aeb84a5456ea1419d98fd833631bda4279`, Mesa32 release archive (Mesa 20.1 libraries), miniz 3.0.2. The standard devkitPro ELF-to-NSO tool accepts AArch64 only, so `elf32_to_nso.py` emits an uncompressed NSO with exact ELF32 PT_LOAD segments and SHA256 hashes. Runtime's existing ExeFS packer and NPDM generator are retained.

Reference reviewed: MetalSyntax/Shadow-Guardian-vita `043e95be90358dfb57061bbc9412f23f6d8af0ad`; native signatures/initialization order and touch action convention. Its original MIT notices are preserved in `licenses/shadowguardian-vita-MIT.txt`. No platform-specific license-service or binary patches were imported. The SG library contains legacy ARM SWP instructions (826 unconditional matches in its .text); a dedicated exception hook emulates executed SWP/SWPB via atomic exchange only within the verified module code range, with writable-range checks and preserved CPSR. Unrelated exceptions still reach the normal crash handler. This avoids modifying the original library, but exception-based atomic operations can be slow. SG ABI binding and stdio bridge reuse the user's prior M3 source.

Checks performed:

- Native wrapper cross-compiles and links as ELF32 ARM. Own runtime compile warnings fixed; inherited runtime emits format warnings due to devkitARM long typedefs. Those warnings do not establish game compatibility.
- Original ELF's 214 non-GL imports mapped (123 shims, 91 passthrough), zero missing. 65 GL imports are resolved by the runtime GL layer at execution time; their actual runtime resolution is a console acceptance item.
- Host API test verifies bootstrap call order, all seven JNI string object arguments, renderer profile/dimensions, missing exports, invalid VM version and allocation failure gates. It mocks native functions and does not run the original game.
- Production AudioTrack PCM converter passes 44.1kHz stereo -> 48kHz, mono duplication, reset/pause, invalid inputs and output failure tests under ASan/UBSan (LeakSanitizer disabled).
- Production SWP hook host tests verify word/byte exchange, r8-r12 operands, flags/register preservation and opcode/Thumb/code-range/writable/alignment guards. Actual console exception delivery and performance still require validation.
- NSP verified by extracting Program, Control and Meta NCA. Executable matches the current build; NPDM is AArch32 with the expected title ID. All 16 language title slots and shared new icon match. CNMT references and content SHA256 hashes match; packaged RomFS has only STATUS.txt. NSO segment bytes and hashes match the linked ELF.

No original library/resource or private key is embedded in this package. No console test of this runtime has occurred. The user's earlier log validates the 0.1.1 frontend/file scan only. First frame, menu, gameplay, Java/audio completeness, saves, performance and lifecycle remain unverified.
