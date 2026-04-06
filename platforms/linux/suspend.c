// SPDX-License-Identifier: GPL-2.0-or-later

#include <stdint.h>

void suspend_idle(uint32_t ms) {
    (void)ms;
}

void suspend_power_down(void) {}

void suspend_wakeup_init(void) {}
