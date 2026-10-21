/* SPDX-License-Identifier: MIT. Exercise production PCM conversion boundaries. */
#include "sg.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
static int blocks,mono,fail;
int rt_audout_open(void){return 0;}
unsigned rt_audout_rate(void){return 48000;}
void rt_audout_cancel(int on){(void)on;}
void rt_audout_close(void){}
int rt_audout_submit(const int16_t *p){
 if(fail)return -1;
 for(int i=0;i<1024;i++){assert(p[2*i]==1234);assert(p[2*i+1]==(mono?1234:-5678));}
 blocks++;return 0;
}
int main(void){
 unsigned n=44100;unsigned char *p=malloc(n*4);assert(p);
 for(unsigned i=0;i<n;i++){uint16_t l=1234,r=(uint16_t)-5678;p[i*4]=l;p[i*4+1]=l>>8;p[i*4+2]=r;p[i*4+3]=r>>8;}
 assert(sg_audio_write(p,n*4,44100,2)==(int)n*4);assert(blocks==46);
 sg_audio_pause(1);assert(sg_audio_write(p,4096,48000,2)==4096&&blocks==46);sg_audio_pause(0);
 mono=1;unsigned char m[2048];for(int i=0;i<1024;i++){m[2*i]=1234&255;m[2*i+1]=1234>>8;}
 assert(sg_audio_write(m,sizeof m,48000,1)==sizeof m&&blocks==47);
 assert(sg_audio_write(m,1,48000,1)==-1);assert(sg_audio_write(m,sizeof m,12345,1)==-1);
 assert(sg_audio_write(NULL,0,48000,1)==-1);assert(sg_audio_write(m,sizeof m,48000,3)==-1);
 fail=1;assert(sg_audio_write(m,sizeof m,48000,1)==-1);sg_audio_close();free(p);
 puts("PASS: 44.1k stereo resampling, 48k mono duplication, pause/reset, invalid inputs and submission failure");
}
