/* SPDX-License-Identifier: MIT. Exercise production SWP exception recovery. */
#define _GNU_SOURCE
#include "exc_handler.h"
#include "sg.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
static uint32_t address;static int denied;
Result svcQueryMemory(MemoryInfo *m,uint32_t *page,uint32_t p){(void)page;(void)p;memset(m,0,sizeof *m);m->addr=address;m->size=4096;m->perm=denied?1:3;return 0;}
uint32_t *rt_exc_reg(ThreadExceptionInfo32 *i,ThreadExceptionFrame32 *f,unsigned n){if(n<8)return &i->r[n];if(n<13)return &f->r8_12[n-8];if(n==13)return &i->sp;if(n==14)return &i->lr;return NULL;}
int main(void){
 void *mem=mmap(NULL,8192,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_32BIT,-1,0);assert(mem!=MAP_FAILED&&(uintptr_t)mem<=0xffffffffu);
 uint32_t pc=(uint32_t)(uintptr_t)mem;address=pc+4096;uint32_t *inst=mem,*value=(void *)(uintptr_t)address;
 sg_swp_region(pc,pc+4096);ThreadExceptionInfo32 i={0};ThreadExceptionFrame32 f={0};
 *inst=0xe1053096;*value=123;i.pc=pc;i.r[5]=address;i.r[6]=77;i.r[3]=999;i.pstate=0xa0000010;
 assert(port_exception_hook(EXC_INSTRUCTION_ABORT,&i,&f)==1);
 assert(*value==77&&i.r[3]==123&&i.r[6]==77&&i.pc==pc+4&&i.pstate==0xa0000010);
 *inst=0xe10a2099;*value=0x11223344;i.pc=pc;f.r8_12[2]=address;f.r8_12[1]=0x55667788;
 assert(port_exception_hook(EXC_UNDEFINED_INSTRUCTION,&i,&f)==1);assert(i.r[2]==0x11223344&&*value==0x55667788);
 *inst=0xe1453096;*value=0x12345678;i.pc=pc;i.r[5]=address;i.r[6]=0xab;
 assert(port_exception_hook(EXC_INSTRUCTION_ABORT,&i,&f)==1);assert(i.r[3]==0x78&&*value==0x123456ab);
 i.pc=pc;denied=1;assert(!port_exception_hook(EXC_INSTRUCTION_ABORT,&i,&f));denied=0;
 i.pc=pc;i.pstate|=32;assert(!port_exception_hook(EXC_INSTRUCTION_ABORT,&i,&f));i.pstate&=~32;
 *inst=0xe1a00000;assert(!port_exception_hook(EXC_INSTRUCTION_ABORT,&i,&f));
 *inst=0xe1053096;i.r[5]=address+1;assert(!port_exception_hook(EXC_INSTRUCTION_ABORT,&i,&f));
 i.pc=pc+4096;assert(!port_exception_hook(EXC_INSTRUCTION_ABORT,&i,&f));
 assert(sg_swp_count()==3);munmap(mem,8192);puts("PASS: word/byte atomic swaps, high registers, CPSR preservation, Thumb/range/permission/alignment/opcode guards");
}
