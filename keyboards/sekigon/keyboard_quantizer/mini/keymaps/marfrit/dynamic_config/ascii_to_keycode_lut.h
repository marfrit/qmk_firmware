// copyright 2023 sekigon-gonnoc
// spdx-license-identifier: gpl-2.0-or-later

#pragma once

#include <stdint.h>

extern const uint8_t ascii_to_shift_lut_jp[16];
extern const uint8_t ascii_to_shift_lut_us[16];
extern const uint8_t ascii_to_keycode_lut_jp[128];
extern const uint8_t ascii_to_keycode_lut_us[128];
void set_ascii_to_keycode_lang_lut(uint8_t language);