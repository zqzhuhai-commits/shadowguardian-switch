/* SPDX-License-Identifier: MIT. Physical menu pointer, in output pixels. */
#include "sg.h"
#include <GLES2/gl2.h>
void sg_cursor_draw(void){
 int x,y;if(!sg_input_cursor(&x,&y))return;
 x=x*SG_OUTPUT_WIDTH/SG_WIDTH;y=SG_OUTPUT_HEIGHT-1-y*SG_OUTPUT_HEIGHT/SG_HEIGHT;
 GLint box[4],fbo;GLfloat color[4];GLboolean enabled=glIsEnabled(GL_SCISSOR_TEST),mask[4];
 glGetBooleanv(GL_COLOR_WRITEMASK,mask);glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
 glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo);glGetIntegerv(GL_SCISSOR_BOX,box);glGetFloatv(GL_COLOR_CLEAR_VALUE,color);
 glBindFramebuffer(GL_FRAMEBUFFER,0);glEnable(GL_SCISSOR_TEST);
 glClearColor(0,0,0,1);glScissor(x-9,y-3,19,7);glClear(GL_COLOR_BUFFER_BIT);glScissor(x-3,y-9,7,19);glClear(GL_COLOR_BUFFER_BIT);
 glClearColor(1,1,1,1);glScissor(x-7,y-1,15,3);glClear(GL_COLOR_BUFFER_BIT);glScissor(x-1,y-7,3,15);glClear(GL_COLOR_BUFFER_BIT);
 glScissor(box[0],box[1],box[2],box[3]);glClearColor(color[0],color[1],color[2],color[3]);
 if(!enabled)glDisable(GL_SCISSOR_TEST);
 glColorMask(mask[0],mask[1],mask[2],mask[3]);
 glBindFramebuffer(GL_FRAMEBUFFER,(GLuint)fbo);
}
