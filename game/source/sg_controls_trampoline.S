/* SPDX-License-Identifier: MIT. Preserve native softfp arguments and stack. */
.syntax unified
.arm
.fpu neon
.text
.macro FILTER name, predicate, first, second
.global sg_controls_\name\()_hook
.type sg_controls_\name\()_hook,%function
sg_controls_\name\()_hook:
 push {r0-r3,r12,lr}
 vpush {d0-d7}
 bl \predicate
 cmp r0,#0
 vpop {d0-d7}
 pop {r0-r3,r12,lr}
 bxne lr
 .word \first
 .word \second
 ldr r12,=sg_controls_\name\()_tail
 ldr pc,[r12]
.size sg_controls_\name\()_hook,.-sg_controls_\name\()_hook
.endm
FILTER graph,sg_controls_hidden,0xe92d4ff0,0xed2d8b02
FILTER rect,sg_controls_hidden,0xe92d4ff0,0xed2d8b04
FILTER joystick,sg_controls_active,0xe92d41f0,0xe1a04000
FILTER gauge,sg_controls_active,0xe92d4ff0,0xed2d8b08
.section .note.GNU-stack,"",%progbits
