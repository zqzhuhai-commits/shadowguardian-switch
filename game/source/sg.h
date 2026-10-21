/* SPDX-License-Identifier: MIT */
#pragma once
#include "sg_api.h"
#define SG_CLASS "com/gameloft/android/ANMP/GloftSGHP/ML/"
#define SG_WIDTH 800
#define SG_HEIGHT 480
/* Native asset/UI coordinates; the default framebuffer is always 720p. */
#define SG_OUTPUT_WIDTH 1280
#define SG_OUTPUT_HEIGHT 720
extern SgApi sg_api;
void sg_input_init(void);
int sg_input_poll(void);
void sg_input_release(void);
void sg_audio_close(void);
void sg_audio_pause(int pause);
int sg_audio_write(const unsigned char *bytes, unsigned size, unsigned rate, int channels);
void sg_jni_check(const char *stage);

void sg_swp_region(uint32_t start,uint32_t end);
unsigned sg_swp_count(void);
