// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <inttypes.h>
#include <time.h>

static inline void wait_ms(uint32_t ms) {
    struct timespec ts = {
        .tv_sec  = ms / 1000,
        .tv_nsec = (ms % 1000) * 1000000L,
    };
    nanosleep(&ts, NULL);
}

static inline void wait_us(uint32_t us) {
    struct timespec ts = {
        .tv_sec  = us / 1000000,
        .tv_nsec = (us % 1000000) * 1000L,
    };
    nanosleep(&ts, NULL);
}

#define waitInputPinDelay()
