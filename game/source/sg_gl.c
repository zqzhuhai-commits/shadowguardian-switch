/* SPDX-License-Identifier: MIT. Compatibility must work with RT_GL_CHECK=0. */
#include "sg_atc.h"
#include "sg.h"
#include "gl_layer.h"
#include "util.h"
#include <GLES2/gl2.h>
#include <stdlib.h>
#include <string.h>

static void (*compressed_upload)(GLenum, GLint, GLenum, GLsizei, GLsizei,
                                 GLint, GLsizei, const void *);
static unsigned successes, failures;
static void sg_compressed_upload(GLenum target, GLint level, GLenum format,
        GLsizei width, GLsizei height, GLint border, GLsizei bytes, const void *data) {
    if (format != SG_ATC_RGB && format != SG_ATC_EXPLICIT_ALPHA &&
        format != SG_ATC_INTERPOLATED_ALPHA) {
        compressed_upload(target, level, format, width, height, border, bytes, data);
        return;
    }
    /* Preserve driver validation for invalid target/level/border arguments. */
    if (target != GL_TEXTURE_2D || level < 0 || border != 0) {
        compressed_upload(target, level, format, width, height, border, bytes, data);
        return;
    }
    uint8_t *rgba = NULL;
    int result = sg_atc_decode(format, width, height, bytes, data, &rgba);
    if (result) {
        if (failures++ < 16)
            debugPrintf("[sg/atc] rejected format=0x%x %dx%d bytes=%d result=%d\n",
                        format, width, height, bytes, result);
        /* Do not dereference malformed data or upload an unsupported format. */
        return;
    }
    /* RGBA rows are tightly packed even for non-multiple-of-four sizes.
     * Preserve the game's pixel-store state across this compatibility upload. */
    GLint unpack = 4;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &unpack);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    /* GLES2 accepts unsized GL_RGBA, matching the external format. */
    glTexImage2D(target, level, GL_RGBA, width, height, border,
                 GL_RGBA, GL_UNSIGNED_BYTE, rgba);
    glPixelStorei(GL_UNPACK_ALIGNMENT, unpack);
    free(rgba);
    if (successes++ < 16)
        debugPrintf("[sg/atc] decoded format=0x%x level=%d %dx%d bytes=%d -> RGBA8\n",
                    format, level, width, height, bytes);
}
/* Scale only the default framebuffer. Offscreen textures retain their native
 * dimensions, while the game keeps its 800x480 asset/UI coordinate system. */
static GLint viewport[4] = {0, 0, SG_WIDTH, SG_HEIGHT};
static GLint scissor_box[4] = {0, 0, SG_WIDTH, SG_HEIGHT};
static void (*real_viewport)(GLint, GLint, GLsizei, GLsizei);
static void (*real_scissor)(GLint, GLint, GLsizei, GLsizei);
static void (*real_bind_framebuffer)(GLenum, GLuint);
static void (*real_get_integer)(GLenum, GLint *);
static int scale(int64_t v, int output, int logical) {
    return (int)((int64_t)v * output / logical);
}
static void apply_rect(void (*fn)(GLint, GLint, GLsizei, GLsizei), const GLint *r) {
    GLint fbo = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbo);
    if (fbo) fn(r[0], r[1], r[2], r[3]);
    else {
        int x = scale(r[0], SG_OUTPUT_WIDTH, SG_WIDTH);
        int y = scale(r[1], SG_OUTPUT_HEIGHT, SG_HEIGHT);
        int right = scale((int64_t)r[0] + r[2], SG_OUTPUT_WIDTH, SG_WIDTH);
        int top = scale((int64_t)r[1] + r[3], SG_OUTPUT_HEIGHT, SG_HEIGHT);
        fn(x, y, right - x, top - y);
    }
}
static void sg_viewport(GLint x, GLint y, GLsizei w, GLsizei h) {
    if (w < 0 || h < 0) { real_viewport(x,y,w,h); return; }
    viewport[0]=x; viewport[1]=y; viewport[2]=w; viewport[3]=h;
    apply_rect(real_viewport, viewport);
}
static void sg_scissor(GLint x, GLint y, GLsizei w, GLsizei h) {
    if (w < 0 || h < 0) { real_scissor(x,y,w,h); return; }
    scissor_box[0]=x; scissor_box[1]=y; scissor_box[2]=w; scissor_box[3]=h;
    apply_rect(real_scissor, scissor_box);
}
static void sg_bind_framebuffer(GLenum target, GLuint fbo) {
    real_bind_framebuffer(target, fbo);
    if (target != GL_FRAMEBUFFER) return;
    if (real_viewport) apply_rect(real_viewport, viewport);
    if (real_scissor) apply_rect(real_scissor, scissor_box);
}
static void sg_get_integer(GLenum name, GLint *value) {
    if (value && name == GL_VIEWPORT) memcpy(value, viewport, sizeof viewport);
    else if (value && name == GL_SCISSOR_BOX) memcpy(value, scissor_box, sizeof scissor_box);
    else real_get_integer(name, value);
}
uintptr_t port_gl_wrap(const char *name, uintptr_t real) {
    if (!strcmp(name, "glCompressedTexImage2D") && real) {
        compressed_upload = (void *)real;
        return (uintptr_t)sg_compressed_upload;
    }
#define WRAP(name_, slot_, fn_) \
    if (!strcmp(name, name_) && real) { slot_ = (void *)real; return (uintptr_t)fn_; }
    WRAP("glViewport", real_viewport, sg_viewport)
    WRAP("glScissor", real_scissor, sg_scissor)
    WRAP("glBindFramebuffer", real_bind_framebuffer, sg_bind_framebuffer)
    WRAP("glGetIntegerv", real_get_integer, sg_get_integer)
#undef WRAP
    return 0;
}
