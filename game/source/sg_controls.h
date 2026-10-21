/* SPDX-License-Identifier: MIT */
#pragma once
#include "sg_api.h"
int sg_controls_install(sg_lookup lookup, void *module,
                        int (*patch)(void *, const void *, size_t));
int sg_controls_active(void);
int sg_controls_hidden(void *gui, unsigned index);
