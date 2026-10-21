/* SPDX-License-Identifier: MIT. Menu cursor GL state preservation. */
#include "sg.h"
#include <GLES2/gl2.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
static int menu=1,enabled=0,fbo=7,clears;
static int box[]={5,6,7,8};static GLfloat color[]={.1f,.2f,.3f,.4f};
static GLboolean mask[]={0,1,0,1};
int sg_input_cursor(int *x,int *y){*x=400;*y=240;return menu;}
GLboolean glIsEnabled(GLenum n){assert(n==GL_SCISSOR_TEST);return enabled;}
void glGetIntegerv(GLenum n,GLint *v){if(n==GL_FRAMEBUFFER_BINDING)*v=fbo;else {assert(n==GL_SCISSOR_BOX);memcpy(v,box,sizeof box);}}
void glGetFloatv(GLenum n,GLfloat *v){assert(n==GL_COLOR_CLEAR_VALUE);memcpy(v,color,sizeof color);}
void glGetBooleanv(GLenum n,GLboolean *v){assert(n==GL_COLOR_WRITEMASK);memcpy(v,mask,sizeof mask);}
void glColorMask(GLboolean a,GLboolean b,GLboolean c,GLboolean d){GLboolean m[]={a,b,c,d};memcpy(mask,m,sizeof m);}
void glBindFramebuffer(GLenum n,GLuint v){assert(n==GL_FRAMEBUFFER);fbo=v;}
void glEnable(GLenum n){assert(n==GL_SCISSOR_TEST);enabled=1;}
void glDisable(GLenum n){assert(n==GL_SCISSOR_TEST);enabled=0;}
void glClearColor(GLfloat r,GLfloat g,GLfloat b,GLfloat a){GLfloat c[]={r,g,b,a};memcpy(color,c,sizeof c);}
void glScissor(GLint x,GLint y,GLsizei w,GLsizei h){int r[]={x,y,w,h};memcpy(box,r,sizeof r);}
void glClear(GLenum n){assert(n==GL_COLOR_BUFFER_BIT&&!fbo&&enabled);assert(mask[0]&&mask[1]&&mask[2]&&mask[3]);clears++;}
int main(void){
 int oldbox[4];GLfloat oldcolor[4];GLboolean oldmask[4];memcpy(oldbox,box,sizeof box);memcpy(oldcolor,color,sizeof color);memcpy(oldmask,mask,sizeof mask);
 sg_cursor_draw();assert(clears==4&&!enabled&&fbo==7);assert(!memcmp(oldbox,box,sizeof box)&&!memcmp(oldcolor,color,sizeof color)&&!memcmp(oldmask,mask,sizeof mask));
 menu=0;sg_cursor_draw();assert(clears==4);
 menu=1;enabled=1;sg_cursor_draw();assert(enabled&&clears==8);
 puts("PASS: menu-only visible pointer, default framebuffer, color mask/scissor/clear color/FBO state restored");
}
