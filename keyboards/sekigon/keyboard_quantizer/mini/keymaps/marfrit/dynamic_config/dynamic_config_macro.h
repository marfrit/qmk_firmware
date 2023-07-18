// copyright 2023 sekigon-gonnoc
// spdx-license-identifier: gpl-2.0-or-later

#pragma once

#include "quantum.h"
#include "ascii_to_keycode_lut.h"

bool process_record_dynamic_config_macro(uint16_t     keycode,
                                         keyrecord_t *record);
void process_macro_send_char(void);
void set_macro_string(const char * str);