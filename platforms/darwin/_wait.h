// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <inttypes.h>
#include <unistd.h>
#include <time.h>

static inline void wait_ms(uint32_t ms) {
    usleep(ms * 1000);
}

static inline void wait_us(uint32_t us) {
    usleep(us);
}

#define waitInputPinDelay()
