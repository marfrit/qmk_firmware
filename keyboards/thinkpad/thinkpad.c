// Copyright 2022 Marek Kraus (@gamelaster)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "thinkpad.h"
#include "config.h"
#include "ps2.h"

void keyboard_pre_init_user(void) {

    debug_enable = true;
    debug_keyboard = true;
    debug_mouse = true;

    wait_ms(100);

    setPinOutput(POWERPIN1);
    writePinHigh(POWERPIN1);
    setPinOutput(POWERPIN2);
    writePinHigh(POWERPIN2);

    wait_ms(2000);

    return;
}
