// SPDX-License-Identifier: GPL-2.0-or-later
// QAP'LA: macOS mach_absolute_time based timer

#include "timer.h"
#include <mach/mach_time.h>

static uint64_t start_time;
static mach_timebase_info_data_t timebase;

void timer_init(void) {
    mach_timebase_info(&timebase);
    start_time = mach_absolute_time();
}

void timer_clear(void) {
    start_time = mach_absolute_time();
}

static uint32_t elapsed_ms(void) {
    uint64_t now = mach_absolute_time();
    uint64_t elapsed_ns = (now - start_time) * timebase.numer / timebase.denom;
    return (uint32_t)(elapsed_ns / 1000000);
}

uint16_t timer_read(void) { return (uint16_t)elapsed_ms(); }
uint32_t timer_read32(void) { return elapsed_ms(); }
uint32_t timer_read_internal(void) { return elapsed_ms(); }
