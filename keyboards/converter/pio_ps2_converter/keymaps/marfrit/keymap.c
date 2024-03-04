/*
Copyright 2012 Jun Wako <wakojun@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "marfrit.h"

#define LAYOUT_wrapper(...) LAYOUT(__VA_ARGS__)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /* 122 keys, but 127 matrix positions (stabilizer inserts can be
    replaced by flipper an converted to keys) */
    [_HIEAO] = LAYOUT_wrapper(
                    KC_F13, KC_F14, KC_F15, KC_F16, KC_F17, KC_F18, KC_F19, KC_F20, KC_F21, KC_F22, KC_F23, KC_F24,
    KC_ESC,         KC_F1,  KC_F2,  KC_F3,  KC_F4,  KC_F5,  KC_F6,  KC_F7,  KC_F8,  KC_F9,  KC_F10, KC_F11, KC_F12,              MUTE_TEAMS, QK_BOOT,DB_TOGG,    KC_VOLD,KC_VOLU,KC_MUTE,
    KC_GRV, KC_1,   KC_2,   KC_3,   KC_4,   KC_5,   KC_6,   KC_7,   KC_8,   KC_9,   KC_0,   KC_MINS,KC_EQL, XXX,    KC_BSPC,     KC_INS, KC_HOME,KC_PGUP,    KC_NUM,KC_PSLS,KC_PAST,KC_PMNS,
    KC_TAB, ____HIEAO_L1____, ____HIEAO_R1____,     KC_LBRC,KC_RBRC,        KC_BSLS,     KC_DEL, KC_END, KC_PGDN,    KC_P7,  KC_P8,  KC_P9,  KC_PPLS,
    ______CLMC______,____HIEAO_L2____, ____HIEAO_R2____, KC_QUOT,        KC_NUHS,KC_ENT,                                  KC_P4,  KC_P5,  KC_P6,  KC_PCMM,
    KC_LSFT,KC_NUBS,____HIEAO_L3____, ____HIEAO_R3____,   XXX, KC_RSFT,             KC_UP,              KC_P1,  KC_P2,  KC_P3,  KC_PENT,
    KC_CAPS,KC_LGUI,KC_LALT,    XXX,                KC_SPC,                 XXX,    XXX,    KC_RALT,KC_RGUI,KC_APP, KC_RCTL,     KC_LEFT,KC_DOWN,KC_RGHT,            KC_P0,  KC_PDOT,KC_PEQL
    ),
    [_SYMBOLS] = LAYOUT_wrapper(
                    KC_F13, KC_F14, KC_F15, KC_F16, KC_F17, KC_F18, KC_F19, KC_F20, KC_F21, KC_F22, KC_F23, KC_F24,
    KC_ESC,         KC_F1,  KC_F2,  KC_F3,  KC_F4,  KC_F5,  KC_F6,  KC_F7,  KC_F8,  KC_F9,  KC_F10, KC_F11, KC_F12,              DB_TOGG,QK_BOOT,KC_PAUS,    KC_VOLD,KC_VOLU,KC_MUTE,
    KC_GRV, KC_1,   KC_2,   KC_3,   KC_4,   KC_5,   KC_6,   KC_7,   KC_8,   KC_9,   KC_0,   KC_MINS,KC_EQL, XXX,    KC_BSPC,     KC_INS, KC_HOME,KC_PGUP,    KC_NUM,KC_PSLS,KC_PAST,KC_PMNS,
    KC_TAB, ___SYMBOL_L1____, ___SYMBOL_R1____,     KC_LBRC,KC_RBRC,        KC_BSLS,     KC_DEL, KC_END, KC_PGDN,    KC_P7,  KC_P8,  KC_P9,  KC_PPLS,
    ______CLMC______,___SYMBOL_L2____, ___SYMBOL_R2____, KC_QUOT,        KC_NUHS,KC_ENT,                                  KC_P4,  KC_P5,  KC_P6,  KC_PCMM,
    KC_LSFT,KC_NUBS,___SYMBOL_L3____, ___SYMBOL_R3____,   XXX, KC_RSFT,             KC_UP,              KC_P1,  KC_P2,  KC_P3,  KC_PENT,
    KC_CAPS,KC_LGUI,KC_LALT,    XXX,                KC_SPC,                 XXX,    XXX,    KC_RALT,KC_RGUI,KC_APP, KC_RCTL,     KC_LEFT,KC_DOWN,KC_RGHT,            KC_P0,  KC_PDOT,KC_PEQL
    ),
    [_NAVIGATION] = LAYOUT_wrapper(
                    KC_F13, KC_F14, KC_F15, KC_F16, KC_F17, KC_F18, KC_F19, KC_F20, KC_F21, KC_F22, KC_F23, KC_F24,
    KC_ESC,         KC_F1,  KC_F2,  KC_F3,  KC_F4,  KC_F5,  KC_F6,  KC_F7,  KC_F8,  KC_F9,  KC_F10, KC_F11, KC_F12,              DB_TOGG,QK_BOOT,KC_PAUS,    KC_VOLD,KC_VOLU,KC_MUTE,
    KC_GRV, KC_1,   KC_2,   KC_3,   KC_4,   KC_5,   KC_6,   KC_7,   KC_8,   KC_9,   KC_0,   KC_MINS,KC_EQL, XXX,    KC_BSPC,     KC_INS, KC_HOME,KC_PGUP,    KC_NUM,KC_PSLS,KC_PAST,KC_PMNS,
    KC_TAB, _NAVIGATION_L1__, _NAVIGATION_R1__,     KC_LBRC,KC_RBRC,        KC_BSLS,     KC_DEL, KC_END, KC_PGDN,    KC_P7,  KC_P8,  KC_P9,  KC_PPLS,
    ______CLMC______,_NAVIGATION_L2__, _NAVIGATION_R2__, KC_QUOT,        KC_NUHS,KC_ENT,                                  KC_P4,  KC_P5,  KC_P6,  KC_PCMM,
    KC_LSFT,KC_NUBS,_NAVIGATION_L3__, _NAVIGATION_R3__,   XXX, KC_RSFT,             KC_UP,              KC_P1,  KC_P2,  KC_P3,  KC_PENT,
    KC_CAPS,KC_LGUI,KC_LALT,    XXX,                KC_SPC,                 XXX,    XXX,    KC_RALT,KC_RGUI,KC_APP, KC_RCTL,     KC_LEFT,KC_DOWN,KC_RGHT,            KC_P0,  KC_PDOT,KC_PEQL
    ),
    [_DIACRITICS] = LAYOUT_wrapper(
                    KC_F13, KC_F14, KC_F15, KC_F16, KC_F17, KC_F18, KC_F19, KC_F20, KC_F21, KC_F22, KC_F23, KC_F24,
    KC_ESC,         KC_F1,  KC_F2,  KC_F3,  KC_F4,  KC_F5,  KC_F6,  KC_F7,  KC_F8,  KC_F9,  KC_F10, KC_F11, KC_F12,              DB_TOGG,QK_BOOT,KC_PAUS,    KC_VOLD,KC_VOLU,KC_MUTE,
    KC_GRV, KC_1,   KC_2,   KC_3,   KC_4,   KC_5,   KC_6,   KC_7,   KC_8,   KC_9,   KC_0,   KC_MINS,KC_EQL, XXX,    KC_BSPC,     KC_INS, KC_HOME,KC_PGUP,    KC_NUM,KC_PSLS,KC_PAST,KC_PMNS,
    KC_TAB, _DIACRITICS_L1__, _DIACRITICS_R1__,     KC_LBRC,KC_RBRC,        KC_BSLS,     KC_DEL, KC_END, KC_PGDN,    KC_P7,  KC_P8,  KC_P9,  KC_PPLS,
    ______CLMC______,_DIACRITICS_L2__, _DIACRITICS_R2__, KC_QUOT,        KC_NUHS,KC_ENT,                                  KC_P4,  KC_P5,  KC_P6,  KC_PCMM,
    KC_LSFT,KC_NUBS,_DIACRITICS_L3__, _DIACRITICS_R3__,   XXX, KC_RSFT,             KC_UP,              KC_P1,  KC_P2,  KC_P3,  KC_PENT,
    KC_CAPS,KC_LGUI,KC_LALT,    XXX,                KC_SPC,                 XXX,    XXX,    KC_RALT,KC_RGUI,KC_APP, KC_RCTL,     KC_LEFT,KC_DOWN,KC_RGHT,            KC_P0,  KC_PDOT,KC_PEQL
    ),
    [_FUNCTION] = LAYOUT_wrapper(
                    KC_F13, KC_F14, KC_F15, KC_F16, KC_F17, KC_F18, KC_F19, KC_F20, KC_F21, KC_F22, KC_F23, KC_F24,
    KC_ESC,         KC_F1,  KC_F2,  KC_F3,  KC_F4,  KC_F5,  KC_F6,  KC_F7,  KC_F8,  KC_F9,  KC_F10, KC_F11, KC_F12,              DB_TOGG,QK_BOOT,KC_PAUS,    KC_VOLD,KC_VOLU,KC_MUTE,
    KC_GRV, KC_1,   KC_2,   KC_3,   KC_4,   KC_5,   KC_6,   KC_7,   KC_8,   KC_9,   KC_0,   KC_MINS,KC_EQL, XXX,    KC_BSPC,     KC_INS, KC_HOME,KC_PGUP,    KC_NUM,KC_PSLS,KC_PAST,KC_PMNS,
    KC_TAB, __FUNCTION_L1___, __FUNCTION_R1___,     KC_LBRC,KC_RBRC,        KC_BSLS,     KC_DEL, KC_END, KC_PGDN,    KC_P7,  KC_P8,  KC_P9,  KC_PPLS,
    ______CLMC______,__FUNCTION_L2___, __FUNCTION_R2___, KC_QUOT,        KC_NUHS,KC_ENT,                                  KC_P4,  KC_P5,  KC_P6,  KC_PCMM,
    KC_LSFT,KC_NUBS,__FUNCTION_L3___, __FUNCTION_R3___,   XXX, KC_RSFT,             KC_UP,              KC_P1,  KC_P2,  KC_P3,  KC_PENT,
    KC_CAPS,KC_LGUI,KC_LALT,    XXX,                KC_SPC,                 XXX,    XXX,    KC_RALT,KC_RGUI,KC_APP, KC_RCTL,     KC_LEFT,KC_DOWN,KC_RGHT,            KC_P0,  KC_PDOT,KC_PEQL
    ),
};
