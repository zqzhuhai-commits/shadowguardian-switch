/* SPDX-License-Identifier: MIT. Exercise production wrappers with a GL sink. */
#include "sg.h"
#include "sg_atc.h"
#include "gl_layer.h"
#include <GLES2/gl2.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
static int unpack=8;
static int rect[4],scissor[4],fbo,uploads,compressed;
void debugPrintf(const char *fmt,...){(void)fmt;}
void glGetIntegerv(GLenum name,GLint *v){assert(name==GL_FRAMEBUFFER_BINDING||name==GL_UNPACK_ALIGNMENT);*v=name==GL_UNPACK_ALIGNMENT?unpack:fbo;}
void glPixelStorei(GLenum name,GLint v){assert(name==GL_UNPACK_ALIGNMENT);unpack=v;}
void glTexImage2D(GLenum target,GLint level,GLint internal,GLsizei w,GLsizei h,GLint border,GLenum format,GLenum type,const void *data){
 assert(target==GL_TEXTURE_2D&&level==0&&internal==GL_RGBA&&w==4&&h==4&&border==0&&format==GL_RGBA&&type==GL_UNSIGNED_BYTE&&data&&unpack==1);uploads++;
}
static void v(GLint x,GLint y,GLsizei w,GLsizei h){int r[]={x,y,w,h};memcpy(rect,r,sizeof r);}
static void s(GLint x,GLint y,GLsizei w,GLsizei h){int r[]={x,y,w,h};memcpy(scissor,r,sizeof r);}
static void b(GLenum t,GLuint f){assert(t==GL_FRAMEBUFFER);fbo=f;}
static void g(GLenum n,GLint *p){assert(n==GL_FRAMEBUFFER_BINDING);*p=fbo;}
static void c(GLenum t,GLint l,GLenum f,GLsizei w,GLsizei h,GLint border,GLsizei bytes,const void *d){(void)t;(void)l;(void)f;(void)w;(void)h;(void)border;(void)bytes;(void)d;compressed++;}
int main(void){
 void (*viewport)(GLint,GLint,GLsizei,GLsizei)=(void *)port_gl_wrap("glViewport",(uintptr_t)v);
 void (*clip)(GLint,GLint,GLsizei,GLsizei)=(void *)port_gl_wrap("glScissor",(uintptr_t)s);
 void (*bind)(GLenum,GLuint)=(void *)port_gl_wrap("glBindFramebuffer",(uintptr_t)b);
 void (*query)(GLenum,GLint *)=(void *)port_gl_wrap("glGetIntegerv",(uintptr_t)g);
 viewport(0,0,800,480);assert(rect[0]==0&&rect[1]==0&&rect[2]==1280&&rect[3]==720);
 clip(100,100,200,200);assert(scissor[0]==160&&scissor[1]==150&&scissor[2]==320&&scissor[3]==300);
 GLint values[4];query(GL_VIEWPORT,values);assert(values[2]==800&&values[3]==480);
 bind(GL_FRAMEBUFFER,7);assert(rect[2]==800&&rect[3]==480&&scissor[0]==100);
 viewport(0,0,256,128);assert(rect[2]==256&&rect[3]==128);
 bind(GL_FRAMEBUFFER,0);assert(rect[2]==409&&rect[3]==192);
 viewport(0,0,800,480);assert(rect[2]==1280&&rect[3]==720);
 viewport(0,0,-1,480);assert(rect[2]==-1);query(GL_VIEWPORT,values);assert(values[2]==800);
 void (*upload)(GLenum,GLint,GLenum,GLsizei,GLsizei,GLint,GLsizei,const void *)=(void *)port_gl_wrap("glCompressedTexImage2D",(uintptr_t)c);
 unsigned char data[16]={0};upload(GL_TEXTURE_2D,0,SG_ATC_EXPLICIT_ALPHA,4,4,0,16,data);assert(uploads==1&&!compressed&&unpack==8);
 upload(GL_TEXTURE_2D,0,SG_ATC_EXPLICIT_ALPHA,4,4,0,15,data);assert(uploads==1&&!compressed&&unpack==8);
 upload(GL_TEXTURE_2D,0,0x8d64,4,4,0,8,data);assert(compressed==1);
 assert(!port_gl_wrap("glViewport",0));
 puts("PASS: 1280x720 default framebuffer, logical queries, offscreen sizes, FBO transitions, ATC upload and malformed-input rejection");
}
