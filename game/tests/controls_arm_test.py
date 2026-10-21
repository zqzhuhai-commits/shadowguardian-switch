#!/usr/bin/env python3
"""Execute the linked ARM draw filters, including both predicate outcomes."""
import struct
import sys
from elftools.elf.elffile import ELFFile
from unicorn import Uc, UC_ARCH_ARM, UC_MODE_ARM, UC_HOOK_CODE
from unicorn.arm_const import *
with open(sys.argv[1] if len(sys.argv)>1 else 'shadowguardian_game.elf','rb') as source:
    elf=ELFFile(source)
    symbols={s.name:s['st_value'] for s in elf.get_section_by_name('.symtab').iter_symbols()}
    segments=[(s['p_vaddr'],s['p_memsz'],s.data()) for s in elf.iter_segments() if s['p_type']=='PT_LOAD']
for kind,stack_delta in [('graph',44),('rect',52),('joystick',24),('gauge',68)]:
    for hidden in (0,1):
        uc=Uc(UC_ARCH_ARM,UC_MODE_ARM)
        for address,size,data in segments:
            uc.mem_map(address&~4095,((address%4096+size+4095)//4096)*4096)
            uc.mem_write(address,data)
        uc.mem_map(0x1000000,0x20000)
        stack=0x1010000;ret=0x1000000;tail=ret+4
        uc.reg_write(UC_ARM_REG_C1_C0_2,0xf00000)
        uc.reg_write(UC_ARM_REG_FPEXC,0x40000000)
        uc.reg_write(UC_ARM_REG_SP,stack);uc.reg_write(UC_ARM_REG_LR,ret)
        args=[0x123,39,0x555,0x666]
        for r,v in zip([UC_ARM_REG_R0,UC_ARM_REG_R1,UC_ARM_REG_R2,UC_ARM_REG_R3],args):uc.reg_write(r,v)
        for i in range(8):uc.reg_write(UC_ARM_REG_D0+i,0x100000000+i)
        uc.mem_write(stack,struct.pack('<II',0xaabbccdd,0x11223344))
        uc.mem_write(symbols[f'sg_controls_{kind}_tail'],struct.pack('<I',tail))
        predicate=symbols['sg_controls_hidden' if kind in ('graph','rect') else 'sg_controls_active']
        reached=[]
        def step(machine,address,size,_):
            if address==predicate:
                for r in [UC_ARM_REG_R0,UC_ARM_REG_R1,UC_ARM_REG_R2,UC_ARM_REG_R3,UC_ARM_REG_R12]:machine.reg_write(r,0xdead)
                for i in range(8):machine.reg_write(UC_ARM_REG_D0+i,0xdeadbeef)
                machine.reg_write(UC_ARM_REG_R0,hidden)
                machine.reg_write(UC_ARM_REG_PC,machine.reg_read(UC_ARM_REG_LR))
            elif address in (ret,tail):reached.append(address);machine.emu_stop()
        uc.hook_add(UC_HOOK_CODE,step)
        uc.emu_start(symbols[f'sg_controls_{kind}_hook'],ret+8,count=200)
        assert reached==[ret if hidden else tail]
        assert uc.reg_read(UC_ARM_REG_SP)==stack-(0 if hidden else stack_delta)
        assert [uc.reg_read(r) for r in [UC_ARM_REG_R0,UC_ARM_REG_R1,UC_ARM_REG_R2,UC_ARM_REG_R3]]==args
        assert [uc.reg_read(UC_ARM_REG_D0+i) for i in range(8)]==[0x100000000+i for i in range(8)]
        assert bytes(uc.mem_read(stack,8))==struct.pack('<II',0xaabbccdd,0x11223344)
print('PASS: linked ARM draw filters preserve register/stack/FP arguments; hidden and visible paths')
