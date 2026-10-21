/* Minimal GLES2 declarations for host GL-call contract tests only. */
#pragma once
#include <stdint.h>
typedef unsigned int GLenum,GLuint;
typedef int GLint,GLsizei;
#define GL_TEXTURE_2D 0x0de1
#define GL_RGBA 0x1908
#define GL_UNSIGNED_BYTE 0x1401
#define GL_FRAMEBUFFER 0x8d40
#define GL_FRAMEBUFFER_BINDING 0x8ca6
#define GL_UNPACK_ALIGNMENT 0x0cf5
void glPixelStorei(GLenum,GLint);
#define GL_VIEWPORT 0x0ba2
#define GL_SCISSOR_BOX 0x0c10
void glTexImage2D(GLenum,GLint,GLint,GLsizei,GLsizei,GLint,GLenum,GLenum,const void *);
void glGetIntegerv(GLenum,GLint *);

typedef float GLfloat;
typedef unsigned char GLboolean;
#define GL_SCISSOR_TEST 0x0c11
#define GL_COLOR_CLEAR_VALUE 0x0c22
#define GL_COLOR_BUFFER_BIT 0x4000
#define GL_COLOR_WRITEMASK 0x0c23
#define GL_TRUE 1
GLboolean glIsEnabled(GLenum);
void glGetFloatv(GLenum,GLfloat *);
void glGetBooleanv(GLenum,GLboolean *);
void glColorMask(GLboolean,GLboolean,GLboolean,GLboolean);
void glBindFramebuffer(GLenum,GLuint);
void glEnable(GLenum);
void glDisable(GLenum);
void glClearColor(GLfloat,GLfloat,GLfloat,GLfloat);
void glScissor(GLint,GLint,GLsizei,GLsizei);
void glClear(GLenum);
