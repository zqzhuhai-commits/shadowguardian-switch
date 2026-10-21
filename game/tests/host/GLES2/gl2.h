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
