// SPDX-License-Identifier: GPL-2.0-or-later
// QAP'LA: POSIX clock_gettime based timer

#include "timer.h"
#include <time.h>

static struct timespec start_time;

void timer_init(void) {
    clock_gettime(CLOCK_MONOTONIC, &start_time);
}

void timer_clear(void) {
    clock_gettime(CLOCK_MONOTONIC, &start_time);
}

static uint32_t elapsed_ms(void) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    uint32_t sec  = (uint32_t)(now.tv_sec - start_time.tv_sec);
    int32_t  nsec = (int32_t)(now.tv_nsec - start_time.tv_nsec);
    if (nsec < 0) {
        sec--;
        nsec += 1000000000;
    }
    return sec * 1000 + (uint32_t)(nsec / 1000000);
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
