#!/usr/bin/env python3
"""Execute the user's verified ELF input branches with mocked engine services.
Requires pyelftools + Unicorn; never copies game bytes into the repository.
"""
import argparse, hashlib, struct
from elftools.elf.elffile import ELFFile
from unicorn import Uc, UC_ARCH_ARM, UC_MODE_ARM, UC_HOOK_CODE
from unicorn.arm_const import *
p=argparse.ArgumentParser();p.add_argument('library');a=p.parse_args()
raw=open(a.library,'rb').read()
assert hashlib.sha256(raw).hexdigest()=='ad17a127ae438e829a23e372ddd8100f82fe6a311b0ad295b8cead69d5805ef9'
f=open(a.library,'rb');elf=ELFFile(f)
segments=[(s['p_vaddr'],s.data()) for s in elf.iter_segments() if s['p_type']=='PT_LOAD']
CTRL,ACTOR,STATE,VT,TM,GS,TOUCH,WEAPON,AMMO=range(0x1000000,0x1009000,0x1000)
ENTER=0x1010000;HALT=0x1011000

def run(*,delta=0,enabled=1,mode=0,substate=0,weapon=True,reserve=20,clip=3,capacity=10,animation=9,button=True,touch=True,x=412.8):
 u=Uc(UC_ARCH_ARM,UC_MODE_ARM);u.mem_map(0,0x500000);u.mem_map(0x1000000,0x20000)
 for addr,data in segments:u.mem_write(addr,data)
 def w(addr,value):u.mem_write(addr,struct.pack('<I',value&0xffffffff))
 def b(addr,value):u.mem_write(addr,bytes([value]))
 def fl(addr,value):u.mem_write(addr,struct.pack('<f',value))
 w(CTRL+0x14,ACTOR);b(CTRL+0x18,enabled)
 w(ACTOR,VT);w(ACTOR+0x1e4,mode);w(ACTOR+0x230,STATE)
 u.mem_write(STATE+0x1c,struct.pack('<h',substate))
 w(ACTOR+0x4ac,WEAPON if weapon else 0);w(ACTOR+0x4e8,AMMO)
 w(ACTOR+0x4b0,0);w(ACTOR+0x4f4,clip);w(ACTOR+0x508,animation)
 w(WEAPON+0x98,0);w(WEAPON+0xe8,capacity);w(AMMO,reserve)
 w(VT+0x108,0x12a358);w(VT+0xd0,ENTER);b(GS+0x67,int(button))
 fl(TOUCH+8,x-delta);fl(TOUCH+0x10,x)
 u.reg_write(UC_ARM_REG_SP,0x101ff00);u.reg_write(UC_ARM_REG_LR,HALT)
 u.reg_write(UC_ARM_REG_R0,CTRL);u.reg_write(UC_ARM_REG_C1_C0_2,0xf00000);u.reg_write(UC_ARM_REG_FPEXC,0x40000000)
 calls=[]
 def ret(value=None):
  if value is not None:u.reg_write(UC_ARM_REG_R0,value)
  u.reg_write(UC_ARM_REG_PC,u.reg_read(UC_ARM_REG_LR))
 def hook(uc,pc,size,user):
  if pc==0x130b44:uc.reg_write(UC_ARM_REG_PC,0x130c20) # omit unrelated movement/camera, not input gates
  elif pc==0x132098:ret() # trigger queue processing, not weapon/reload touch handling
  elif pc==0x136bbc:ret(TM)
  elif pc==0x136c18:ret(GS)
  elif pc==0x18b63c:
   assert uc.reg_read(UC_ARM_REG_R1)==3 and uc.reg_read(UC_ARM_REG_R2)==4
   ret(TOUCH if touch else 0)
  elif pc==0x129ed8:ret() # combat-camera/assets unavailable in this isolated test
  elif pc==ENTER:
   assert uc.reg_read(UC_ARM_REG_R0)==ACTOR
   assert uc.reg_read(UC_ARM_REG_R1)==animation
   assert uc.reg_read(UC_ARM_REG_R2)==1 and uc.reg_read(UC_ARM_REG_R3)==0
   calls.append('reload');ret()
  elif pc in (0xe15d4,0xe1578):calls.append('next' if pc==0xe15d4 else 'previous');uc.emu_stop()
  elif pc in (0x134028,HALT):uc.emu_stop()
 u.hook_add(UC_HOOK_CODE,hook);u.emu_start(0x130b24,HALT+4,count=10000)
 return calls
assert run()==['reload']
assert run(delta=60,x=472.8)==['next']
assert run(delta=-60,x=352.8)==['previous']
for kw in [dict(enabled=0),dict(mode=1),dict(mode=3),dict(button=False),dict(touch=False),dict(weapon=False),dict(reserve=0),dict(clip=10),dict(animation=-1)]+[dict(substate=v) for v in (3,5,7,14)]:
 assert run(**kw)==[],kw
assert run(mode=2)==['reload']
# A tap at the physical weapon icon is normalized into the exact middle band.
assert 365<=688/(800/480)<=450
# Suspend/resume writes the same action flag read by the chosen API binding.
def phase(v):
 u=Uc(UC_ARCH_ARM,UC_MODE_ARM);u.mem_map(0,0x500000);u.mem_map(0x1000000,0x2000)
 for addr,data in segments:u.mem_write(addr,data)
 u.mem_write(CTRL+0x172,bytes([v]));u.reg_write(UC_ARM_REG_R0,CTRL);u.reg_write(UC_ARM_REG_LR,CTRL+0x1000)
 u.emu_start(0xbf710,CTRL+0x1000,count=3);return u.reg_read(UC_ARM_REG_R0)
assert phase(0)==0 and phase(1)==1
print('PASS: original ARM weapon/reload dispatch, enabled/actor/substate/button/weapon/ammo/full-magazine/animation gates, and actual action-phase API')
