/* SPDX-License-Identifier: MIT. Independent golden vectors and size guards. */
#include "sg_atc.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static void selectors(unsigned char *p) {
    unsigned long long bits=0;
    for(unsigned i=0;i<16;i++)bits|=(unsigned long long)(i%8)<<(3*i);
    for(unsigned i=0;i<6;i++)p[i]=(unsigned char)(bits>>(8*i));
}
int main(void) {
    unsigned char block[17]={0},*out=NULL;
    /* Black / white endpoints, each 2-bit color index in every row. */
    block[2]=block[3]=255;memset(block+4,0xe4,4);
    assert(!sg_atc_decode(SG_ATC_RGB,4,4,8,block,&out));
    const unsigned char colors[]={0,95,159,255};
    for(unsigned i=0;i<16;i++)for(unsigned c=0;c<4;c++)assert(out[i*4+c]==(c==3?255:colors[i%4]));
    free(out);
    /* Black mode: index 0 black, index 1 clamped to zero,
       index 2 RGB555 white, index 3 RGB565 white. */
    block[0]=block[1]=255;
    assert(!sg_atc_decode(SG_ATC_RGB,4,4,8,block,&out));
    const unsigned char dark[]={0,192,255,255};
    for(unsigned i=0;i<16;i++)assert(out[i*4]==dark[i%4]);
    free(out);
    /* Explicit alpha consumes nibbles in row-major little-endian order. */
    for(unsigned i=0;i<8;i++)block[i]=(2*i)|((2*i+1)<<4);
    memset(block+8,0,8);
    assert(!sg_atc_decode(SG_ATC_EXPLICIT_ALPHA,4,4,16,block,&out));
    for(unsigned i=0;i<16;i++)assert(out[i*4+3]==i*17);
    free(out);
    block[0]=255;block[1]=0;selectors(block+2);
    assert(!sg_atc_decode(SG_ATC_INTERPOLATED_ALPHA,4,4,16,block,&out));
    const unsigned char alpha8[]={255,0,219,182,146,109,73,36};
    for(unsigned i=0;i<16;i++)assert(out[i*4+3]==alpha8[i%8]);
    free(out);
    block[0]=0;block[1]=255;
    assert(!sg_atc_decode(SG_ATC_INTERPOLATED_ALPHA,4,4,16,block,&out));
    const unsigned char alpha6[]={0,255,51,102,153,204,0,255};
    for(unsigned i=0;i<16;i++)assert(out[i*4+3]==alpha6[i%8]);
    free(out);
    /* An odd 3x5 image needs two blocks, even when most texels clip. */
    unsigned char odd[32]={0};memset(odd,0xff,8);memset(odd+16,0x11,8);
    assert(!sg_atc_decode(SG_ATC_EXPLICIT_ALPHA,3,5,32,odd,&out));
    for(unsigned i=0;i<15;i++)assert(out[i*4+3]==(i<12?255:17));
    free(out);
    /* Unaligned input is legal; short input must be rejected before access. */
    memset(block,0,sizeof block);
    assert(!sg_atc_decode(SG_ATC_RGB,1,1,8,block+1,&out));free(out);
    out=(void *)1;assert(sg_atc_decode(SG_ATC_RGB,4,4,7,block,&out)<0&&!out);
    assert(sg_atc_decode(0x8c91,4,4,8,block,&out)<0&&!out);
    assert(sg_atc_decode(SG_ATC_RGB,-1,4,8,block,&out)<0&&!out);
    assert(sg_atc_decode(SG_ATC_RGB,INT_MAX,INT_MAX,8,block,&out)<0&&!out);
    assert(sg_atc_decode(SG_ATC_RGB,4,4,8,NULL,&out)<0&&!out);
    assert(sg_atc_decode(SG_ATC_RGB,4,4,8,block,NULL)<0);
    puts("PASS: ATC color modes, alpha modes, selector order, edge clipping, alignment and invalid sizes");
}
