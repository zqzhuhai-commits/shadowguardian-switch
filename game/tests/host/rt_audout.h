#pragma once
#include <stdint.h>
#define RT_AUDOUT_FRAMES 1024
int rt_audout_open(void);
unsigned rt_audout_rate(void);
int rt_audout_submit(const int16_t *);
void rt_audout_cancel(int);
void rt_audout_close(void);
