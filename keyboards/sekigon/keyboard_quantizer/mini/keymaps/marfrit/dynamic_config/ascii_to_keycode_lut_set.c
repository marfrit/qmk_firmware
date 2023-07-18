// copyright 2023 sekigon-gonnoc
// spdx-license-identifier: gpl-2.0-or-later

#include <stdint.h>
#include <string.h>
#include "ascii_to_keycode_lut.h"
#include "dynamic_config.h"

uint8_t ascii_to_shift_lut[16];
uint8_t ascii_to_keycode_lut[128];

void set_ascii_to_keycode_lut(const uint8_t *shift_lut, const uint8_t *keycode_lut) {
    memcpy(ascii_to_shift_lut, shift_lut, sizeof(ascii_to_shift_lut));
    memcpy(ascii_to_keycode_lut, keycode_lut, sizeof(ascii_to_keycode_lut));
}

static void set_ascii_to_keycode_lut_jp(void) {
    set_ascii_to_keycode_lut(ascii_to_shift_lut_jp, ascii_to_keycode_lut_jp);
}

static void set_ascii_to_keycode_lut_us(void) {
    set_ascii_to_keycode_lut(ascii_to_shift_lut_us, ascii_to_keycode_lut_us);
}

void set_ascii_to_keycode_lang_lut(uint8_t language) {
    if (language == LANG_US) {
        set_ascii_to_keycode_lut_us();
    } else if (language == LANG_JP) {
        set_ascii_to_keycode_lut_jp();
    }
}