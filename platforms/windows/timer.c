// SPDX-License-Identifier: GPL-2.0-or-later
// QAP'LA: Windows QueryPerformanceCounter-based timer

#include "timer.h"
#include <windows.h>

static LARGE_INTEGER start_time;
static LARGE_INTEGER frequency;

void timer_init(void) {
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start_time);
}

void timer_clear(void) {
    QueryPerformanceCounter(&start_time);
}

static uint32_t elapsed_ms(void) {
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (uint32_t)((now.QuadPart - start_time.QuadPart) * 1000 / frequency.QuadPart);
}

uint16_t timer_read(void) {
    return (uint16_t)elapsed_ms();
}

uint32_t timer_read32(void) {
    return elapsed_ms();
}

uint32_t timer_read_internal(void) {
    return elapsed_ms();
}
