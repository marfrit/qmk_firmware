// SPDX-License-Identifier: GPL-2.0-or-later
// QAP'LA Windows default keymap — identity passthrough
// Matrix position (row, col) = (VK_code / 16, VK_code % 16)

#include QMK_KEYBOARD_H

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT(
        KC_ESC, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0,
        KC_BSPC, KC_TAB, KC_A, KC_SPC, KC_ENT
    ),
};
