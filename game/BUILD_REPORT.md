# Runtime 0.2.3 — touch controls restored

The user confirms the 0.2.2 right-stick camera works, but requests on-screen buttons for cover, reloading and other actions. 0.2.3 removes all control-hiding hooks, the render wrapper and ARM trampoline. The original game draws its controls, virtual joystick, highlights and button gauges. Physical input, camera, automatic boot and disabled normal launch logging are unchanged.

Validation: devkitARM 16.1.0 / patched libnx32 35c0b5aeb84a5456ea1419d98fd833631bda4279 / Mesa20 / miniz 3.0.2 compile and link passed. NSO converter verified three ELF PT_LOAD segments and their hashes. hacBrewPack 3.05 with the title-version patch packaged the NSP, and hactool extraction verified Program/Control/Meta, executable identity, AArch32 NPDM/title ID, all 16 Chinese title slots, icon, NACP version 0.2.3, CNMT version 131075 and content hashes. Program RomFS contains only STATUS.txt.

The physical input code is identical to 0.2.2, whose host regression and isolated original-ARM input tests passed. No new Switch hardware run has occurred. Touch control visibility, cover/reload, physical controls and saves still require console verification.

NSP size: 6794456 bytes.
NSP SHA256: `ebd1ada5f43c26a85e91ea0ad82a05c61edd3c8ec50f8daff04aa010a812c298`.

No original game library/resources or private keys are embedded. Only the verified installable `孤侠魅影-0.2.3.nsp` should be installed; the root build-system NSP is an ExeFS-only container.
