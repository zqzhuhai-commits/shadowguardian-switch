# Runtime 0.2.4 — selective draw filtering trial

Filters only gameplay GUI items 0..10 and 33 onward in layouts 19/20, joystick drawing and action-button gauges. Weapon HUD indices 11..32 and separately drawn pause controls are retained. All native visibility/enabled/alpha/highlight data and touch registration remain untouched. Physical input is unchanged from 0.2.3; B still targets the original jump/climb action, not a separate cover button. Cover remains available through its original touch area.

Four ARM hooks validate every prologue before patching. Assembly preserves integer, floating-point and stack arguments, then either returns without drawing or replays the original position-independent prologue and tail-calls the native body. No rendering-wide state mutation is used.

Validation passed: host regression suite with address/undefined sanitizers; filter policy tests proving no gameplay state writes; linked ARM trampoline execution of hidden and visible paths in Unicorn; original ARM reload/weapon dispatch and all existing restrictions; devkitARM build; NSO segment/hash comparison; NSP extraction with AArch32 NPDM, Chinese title/icon/version, CNMT 131076 and content hash checks. Toolchain remains devkitARM 16.1.0 / patched libnx32 / Mesa20 / miniz 3.0.2.

NSP: 孤侠魅影-0.2.4.nsp, 6797016 bytes.
SHA256: `63acf21d3ce5946de88b8febdb8feaf15af48c278ae7b3b64478d0ffa661e15c`.

No Switch hardware run yet. Confirm exact visual coverage, hidden-position touch actions, physical controls, pause and weapon selection on console. No original game library/resources or private keys are embedded. Root shadowguardian_game.nsp is an ExeFS-only build intermediate and must not be installed.
