/* SPDX-License-Identifier: MIT */
#ifndef SG_ATC_H
#define SG_ATC_H
#include <stddef.h>
#include <stdint.h>
#define SG_ATC_RGB 0x8c92u
#define SG_ATC_EXPLICIT_ALPHA 0x8c93u
#define SG_ATC_INTERPOLATED_ALPHA 0x87eeu
/* Success returns a malloc-owned tightly packed RGBA8 image. On failure,
 * *out is NULL. Validate every dimension/size before reading input. */
int sg_atc_decode(unsigned format, int width, int height, int bytes,
                  const void *data, uint8_t **out);
#endif
