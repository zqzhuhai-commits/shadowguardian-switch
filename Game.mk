# Independent AArch32 runtime; does not build legacy game-specific sources.
TARGET := shadowguardian_game
BUILD := build/game
SOURCES := game/source
PORT_NPDM_PROGRAM_ID := 0x0100F53447550000
PORT_NPDM_NAME := shadowguardian
PORT_NPDM_VERSION := 0.2.0
PORT_NPDM_MAIN_STACK := 0x800000
DCR_GL_MESA := 1
PORT_MESA := 20
PORTLIBS ?= $(CURDIR)/portlibs32
include runtime/runtime.mk
# Mesa's GLES1 library supplies fixed-function entrypoints imported by the game.
LIBS += -L$(PORTLIBS)/lib -lGLESv1_CM

# Standard devkitPro elf2nso accepts only AArch64; this runtime is ELF32 ARM.
$(BUILD)/$(TARGET).nso: $(TARGET).elf game/tools/elf32_to_nso.py | $(BUILD)
	python3 game/tools/elf32_to_nso.py $< $@
