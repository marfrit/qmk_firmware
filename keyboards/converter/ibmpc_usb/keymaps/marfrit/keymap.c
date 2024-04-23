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

#include QMK_KEYBOARD_H
#include "marfrit.h"

#ifndef MFADDS
#define USRKC1 KC_F1
#define USRKC2 KC_F2
#endif

#define LAYOUT_wrapper(...) LAYOUT(__VA_ARGS__)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_HIEAO] = LAYOUT_wrapper(
                    KC_F13, KC_F14, KC_F15, KC_F16, KC_F17, KC_F18, KC_F19, KC_F20, KC_F21, KC_F22, KC_F23, KC_F24,
    KC_ESC,         KC_F1,  KC_F2,  KC_F3,  KC_F4,  KC_F5,  KC_F6,  KC_F7,  KC_F8,  EE_CLR,  KC_F10, KC_F11, KC_F12,              QK_BOOT,DB_TOGG,KC_PAUS,            KC_VOLD,KC_VOLU,KC_MUTE,
    KC_GRV,         ___NUMBER_L1____, ___NUMBER_R1____,KC_MINS,KC_EQL, KC_JYEN,KC_BSPC,     KC_INS, KC_HOME,KC_PGUP,    KC_NLCK,KC_PSLS,KC_PAST,KC_PMNS,
    KC_TAB,         ____HIEAO_L1____, ____HIEAO_R1____,    KC_LBRC,KC_RBRC,        KC_BSLS,     KC_DEL, KC_END, KC_PGDN,    KC_P7,  KC_P8,  KC_P9,  KC_PPLS,
    KC_CAPS,        ____HIEAO_L2____, ____HIEAO_R2____,         KC_QUOT,        KC_NUHS,KC_ENT,                                  KC_P4,  KC_P5,  KC_P6,  KC_PCMM,
    KC_LSFT,KC_NUBS,____HIEAO_L3____, ____HIEAO_R3____,         KC_RO,  KC_RSFT,             KC_UP,              KC_P1,  KC_P2,  KC_P3,  KC_PENT,
    KC_LCTL,KC_LGUI,KC_LALT,KC_MHEN,                KC_SPC,                 KC_HENK,KC_KANA,KC_RALT,KC_RGUI,KC_APP, KC_RCTL,     KC_LEFT,KC_DOWN,KC_RGHT,            KC_P0,  KC_PDOT,KC_PEQL
    ),
    [_SYMBOLS] = LAYOUT_wrapper(
                    KC_F13, KC_F14, KC_F15, KC_F16, KC_F17, KC_F18, KC_F19, KC_F20, KC_F21, KC_F22, KC_F23, KC_F24,
    KC_ESC,         KC_F1,  KC_F2,  KC_F3,  KC_F4,  KC_F5,  KC_F6,  KC_F7,  KC_F8,  KC_F9,  KC_F10, KC_F11, KC_F12,              QK_BOOT,KC_SLCK,KC_PAUS,            KC_VOLD,KC_VOLU,KC_MUTE,
    KC_GRV,         ___NUMBER_L1____, ___NUMBER_R1____,KC_MINS,KC_EQL, KC_JYEN,KC_BSPC,     KC_INS, KC_HOME,KC_PGUP,    KC_NLCK,KC_PSLS,KC_PAST,KC_PMNS,
    KC_TAB,         ___SYMBOL_L1____, ___SYMBOL_R1____,    KC_LBRC,KC_RBRC,        KC_BSLS,     KC_DEL, KC_END, KC_PGDN,    KC_P7,  KC_P8,  KC_P9,  KC_PPLS,
    KC_CAPS,        ___SYMBOL_L2____, ___SYMBOL_R2____,         KC_QUOT,        KC_NUHS,KC_ENT,                                  KC_P4,  KC_P5,  KC_P6,  KC_PCMM,
    KC_LSFT,KC_NUBS,___SYMBOL_L3____, ___SYMBOL_R3____,         KC_RO,  KC_RSFT,             KC_UP,              KC_P1,  KC_P2,  KC_P3,  KC_PENT,
    KC_LCTL,KC_LGUI,KC_LALT,KC_MHEN,                KC_SPC,                 KC_HENK,KC_KANA,KC_RALT,KC_RGUI,KC_APP, KC_RCTL,     KC_LEFT,KC_DOWN,KC_RGHT,            KC_P0,  KC_PDOT,KC_PEQL
    ),
    [_NAVIGATION] = LAYOUT_wrapper(
                    KC_F13, KC_F14, KC_F15, KC_F16, KC_F17, KC_F18, KC_F19, KC_F20, KC_F21, KC_F22, KC_F23, KC_F24,
    KC_ESC,         KC_F1,  KC_F2,  KC_F3,  KC_F4,  KC_F5,  KC_F6,  KC_F7,  KC_F8,  KC_F9,  KC_F10, KC_F11, KC_F12,              QK_BOOT,KC_SLCK,KC_PAUS,            KC_VOLD,KC_VOLU,KC_MUTE,
    KC_GRV,         ___NUMBER_L1____, ___NUMBER_R1____,KC_MINS,KC_EQL, KC_JYEN,KC_BSPC,     KC_INS, KC_HOME,KC_PGUP,    KC_NLCK,KC_PSLS,KC_PAST,KC_PMNS,
    KC_TAB,         _NAVIGATION_L1__, _NAVIGATION_R1__,    KC_LBRC,KC_RBRC,        KC_BSLS,     KC_DEL, KC_END, KC_PGDN,    KC_P7,  KC_P8,  KC_P9,  KC_PPLS,
    KC_CAPS,        _NAVIGATION_L2__, _NAVIGATION_R2__,         KC_QUOT,        KC_NUHS,KC_ENT,                                  KC_P4,  KC_P5,  KC_P6,  KC_PCMM,
    KC_LSFT,KC_NUBS,_NAVIGATION_L3__, _NAVIGATION_R3__,         KC_RO,  KC_RSFT,             KC_UP,              KC_P1,  KC_P2,  KC_P3,  KC_PENT,
    KC_LCTL,KC_LGUI,KC_LALT,KC_MHEN,                KC_SPC,                 KC_HENK,KC_KANA,KC_RALT,KC_RGUI,KC_APP, KC_RCTL,     KC_LEFT,KC_DOWN,KC_RGHT,            KC_P0,  KC_PDOT,KC_PEQL
    ),
    [_DIACRITICS] = LAYOUT_wrapper(
                    KC_F13, KC_F14, KC_F15, KC_F16, KC_F17, KC_F18, KC_F19, KC_F20, KC_F21, KC_F22, KC_F23, KC_F24,
    KC_ESC,         USRKC1, USRKC2, KC_F3,  KC_F4,  KC_F5,  KC_F6,  KC_F7,  KC_F8,  KC_F9,  KC_F10, KC_F11, KC_F12,              QK_BOOT,KC_SLCK,KC_PAUS,            KC_VOLD,KC_VOLU,KC_MUTE,
    KC_GRV,         ___NUMBER_L1____, ___NUMBER_R1____,KC_MINS,KC_EQL, KC_JYEN,KC_BSPC,     KC_INS, KC_HOME,KC_PGUP,    KC_NLCK,KC_PSLS,KC_PAST,KC_PMNS,
    KC_TAB,         _DIACRITICS_L1__, _DIACRITICS_R1__,    KC_LBRC,KC_RBRC,        KC_BSLS,     KC_DEL, KC_END, KC_PGDN,    KC_P7,  KC_P8,  KC_P9,  KC_PPLS,
    KC_CAPS,        _DIACRITICS_L2__, _DIACRITICS_R2__,         KC_QUOT,        KC_NUHS,KC_ENT,                                  KC_P4,  KC_P5,  KC_P6,  KC_PCMM,
    KC_LSFT,KC_NUBS,_DIACRITICS_L3__, _DIACRITICS_R3__,         KC_RO,  KC_RSFT,             KC_UP,              KC_P1,  KC_P2,  KC_P3,  KC_PENT,
    KC_LCTL,KC_LGUI,KC_LALT,KC_MHEN,                KC_SPC,                 KC_HENK,KC_KANA,KC_RALT,KC_RGUI,KC_APP, KC_RCTL,     KC_LEFT,KC_DOWN,KC_RGHT,            KC_P0,  KC_PDOT,KC_PEQL
    ),
    [_FUNCTION] = LAYOUT_wrapper(
                    KC_F13, KC_F14, KC_F15, KC_F16, KC_F17, KC_F18, KC_F19, KC_F20, KC_F21, KC_F22, KC_F23, KC_F24,
    KC_ESC,         KC_F1,  KC_F2,  KC_F3,  KC_F4,  KC_F5,  KC_F6,  KC_F7,  KC_F8,  KC_F9,  KC_F10, KC_F11, KC_F12,              QK_BOOT,KC_SLCK,KC_PAUS,            KC_VOLD,KC_VOLU,KC_MUTE,
    KC_GRV,         ___NUMBER_L1____, ___NUMBER_R1____,KC_MINS,KC_EQL, KC_JYEN,KC_BSPC,     KC_INS, KC_HOME,KC_PGUP,    KC_NLCK,KC_PSLS,KC_PAST,KC_PMNS,
    KC_TAB,         __FUNCTION_L1___, __FUNCTION_R1___,    KC_LBRC,KC_RBRC,        KC_BSLS,     KC_DEL, KC_END, KC_PGDN,    KC_P7,  KC_P8,  KC_P9,  KC_PPLS,
    KC_CAPS,        __FUNCTION_L2___, __FUNCTION_R2___,         KC_QUOT,        KC_NUHS,KC_ENT,                                  KC_P4,  KC_P5,  KC_P6,  KC_PCMM,
    KC_LSFT,KC_NUBS,__FUNCTION_L3___, __FUNCTION_R3___,         KC_RO,  KC_RSFT,             KC_UP,              KC_P1,  KC_P2,  KC_P3,  KC_PENT,
    KC_LCTL,KC_LGUI,KC_LALT,KC_MHEN,                KC_SPC,                 KC_HENK,KC_KANA,KC_RALT,KC_RGUI,KC_APP, KC_RCTL,     KC_LEFT,KC_DOWN,KC_RGHT,            KC_P0,  KC_PDOT,KC_PEQL
    ),
};

void keyboard_pre_init_user(void) {
  // Customise these values to desired behaviour
  debug_enable=true;
  //debug_matrix=true;
  debug_keyboard=true;
  //debug_mouse=true;
  dprintln("HERE!");
}
