// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <inttypes.h>
#include <windows.h>

static inline void wait_ms(uint32_t ms) {
    Sleep(ms);
}

static inline void wait_us(uint32_t us) {
    // Windows Sleep granularity is ~1ms minimum.
    // For sub-ms we busy-wait using QueryPerformanceCounter.
    if (us >= 1000) {
        Sleep(us / 1000);
    } else {
        LARGE_INTEGER freq, start, now;
        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&start);
        LONGLONG target = start.QuadPart + (freq.QuadPart * us / 1000000);
        do { QueryPerformanceCounter(&now); } while (now.QuadPart < target);
    }
}

#define waitInputPinDelay()
