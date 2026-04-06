// SPDX-License-Identifier: GPL-2.0-or-later
// QAP'LA default keymap — identity mapping (evdev passthrough)
// Matrix position (row, col) = (evdev_code / 16, evdev_code % 16)
// LAYOUT macro generated from keyboard.json maps only used positions.

#include QMK_KEYBOARD_H

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT(
        // Row 0: ESC, 1-9, 0, -, =, BKSP, TAB
        KC_ESC,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSPC, KC_TAB,
        // Row 1: Q-P, [, ], ENTER, LCTRL, A, S
        KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_ENT,  KC_LCTL, KC_A,    KC_S,
        // Row 2: D-L, ;, ', `, LSHIFT, \, Z-V
        KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT, KC_GRV,  KC_LSFT, KC_BSLS, KC_Z,    KC_X,    KC_C,    KC_V,
        // Row 3: B-/, RSHIFT, KP*, LALT, SPACE, CAPS, F1-F5
        KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT, KC_PAST, KC_LALT, KC_SPC,  KC_CAPS, KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,
        // Row 4: F6-F10, NUMLK, SCRLK, KP7-9, KP-, KP4-6, KP+, KP1
        KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_NUM,  KC_SCRL, KC_P7,   KC_P8,   KC_P9,   KC_PMNS, KC_P4,   KC_P5,   KC_P6,   KC_PPLS, KC_P1,
        // Row 5: KP2, KP3, KP0, KP., 102ND, F11, F12
        KC_P2,   KC_P3,   KC_P0,   KC_PDOT, KC_NUBS, KC_F11,  KC_F12,
        // Row 6: KPENTER, RCTRL, KPSLASH, SYSRQ, RALT, HOME, UP, PGUP, LEFT, RIGHT, END, DOWN, PGDN, INS, DEL
        KC_PENT, KC_RCTL, KC_PSLS, KC_PSCR, KC_RALT, KC_HOME, KC_UP,   KC_PGUP, KC_LEFT, KC_RGHT, KC_END,  KC_DOWN, KC_PGDN, KC_INS,  KC_DEL,
        // Row 7: LGUI, RGUI, APP
        KC_LGUI, KC_RGUI, KC_APP
    ),
};
// clang-format on
