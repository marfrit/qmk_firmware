// Copyright 2022 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#include "quantum.h"

u_int16_t startup_timer = 0;

void keyboard_pre_init_user(void) {
    startup_timer = timer_read();
}

void keyboard_post_init_kb(void) {
    debug_enable   = true;
//    debug_matrix   = true;
    debug_keyboard = true;
    debug_mouse    = true;
    keyboard_post_init_user();
}
