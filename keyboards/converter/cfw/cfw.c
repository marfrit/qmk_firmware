/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "debug.h"
#include "ch.h"
// #include "pico/multicore.h"

void keyboard_post_init_user(void) {
    while (true) {
        chThdSleepMilliseconds(2000);
        dprintf("Main (C0) running on core %d", port_get_core_id());
    }
}
