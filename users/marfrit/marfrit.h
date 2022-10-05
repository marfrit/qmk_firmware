#pragma once
#include QMK_KEYBOARD_H

#define MUTE_HOLD_DELAY 400

enum layers {
  _HIEAO,
  _SYMBOLS,
  _NAVIGATION,
  _DIACRITICS,
  _FUNCTION,
};

enum custom_keycodes {
    MUTE_TEAMS = SAFE_RANGE,
    MUTE_SKYPE,
};

#define SYMB(_KC) LT(_SYMBOLS,    _KC)
#define NAVI(_KC) LT(_NAVIGATION, _KC)
#define DIAC(_KC) LT(_DIACRITICS, _KC)
#define FUNC(_KC) LT(_FUNCTION,   _KC)

#define ___NUMBER_L1____    KC_1, KC_2, KC_3, KC_4, KC_5
#define ___NUMBER_R1____                                  KC_6, KC_7, KC_8, KC_9, KC_0

#define _____PRTETC_____    KC_PSCR, RESET,  KC_PAUS
#define ____INSHMPU_____    KC_INS, KC_HOME, KC_PGUP
#define ____DELENPD_____    KC_DEL, KC_END,  KC_PGDN
#define _____CASPC______    KC_LCTL, KC_LALT, KC_SPC, KC_RALT, KC_RCTL

#define _____NUMR1______    KC_NLCK, KC_PSLS, KC_PAST, KC_PMNS
#define _____NUMR2T_____    KC_P7,   KC_P8,   KC_P9,   KC_PPLS
#define _____NUMR3T_____    KC_P4,   KC_P5,   KC_P6,   KC_PCMM
#define _____NUMR4______    KC_P1,   KC_P2,   KC_P3,   KC_PENT
#define _____NUMR5______    KC_P0,            KC_PDOT

#define ____FKEYS_B1____    KC_F1, KC_F2,  KC_F3,  KC_F4
#define ____FKEYS_B2____    KC_F5, KC_F6,  KC_F7,  KC_F8
#define ____FKEYS_B3____    KC_F9, KC_F10, KC_F11, KC_F12
#define ____FRW1_12_____    ____FKEYS_B1____, ____FKEYS_B2____, ____FKEYS_B3____

#define ____HIEAO_L1____    KC_K, KC_U,         KC_Q,          DIAC(KC_DOT),  KC_J
#define ____HIEAO_L2____    LGUI_T(KC_H), LALT_T(KC_I), LCTL_T(KC_E),  LSFT_T(KC_A),  KC_O
#define ____HIEAO_L3____    KC_X, RALT_T(KC_Y), FUNC(KC_SCLN), SYMB(KC_COMM), NAVI(KC_SLSH)
#define ____HIEAO_L3_FB_    KC_X, RALT_T(KC_Y), FUNC(KC_SCLN), SYMB(KC_COMM), NAVI(KC_SPC)
#define ____HIEAO_R1____                                                                     KC_V,       DIAC(KC_G),   KC_C,         KC_L,         KC_F
#define ____HIEAO_R2____                                                                     KC_D,       RSFT_T(KC_T), RCTL_T(KC_R), LALT_T(KC_N), RGUI_T(KC_S)
#define ____HIEAO_R3____                                                                     NAVI(KC_B), SYMB(KC_P),   FUNC(KC_W),   RALT_T(KC_M), KC_Z

#define ___SYMBOL_L1____    KC_AT,   KC_UNDS, KC_LBRC, KC_RBRC, KC_CIRC
#define ___SYMBOL_L2____    KC_BSLS, KC_SLSH, KC_LCBR, KC_RCBR, KC_ASTR
#define ___SYMBOL_L3____    KC_HASH, KC_DLR,  KC_PIPE, KC_TILD, KC_GRV
#define ___SYMBOL_R1____                                                 KC_EXLM, KC_LT,   KC_GT,   KC_EQL,  KC_AMPR
#define ___SYMBOL_R2____                                                 KC_QUES, KC_LPRN, KC_RPRN, KC_MINS, KC_COLN
#define ___SYMBOL_R3____                                                 KC_PLUS, KC_PERC, KC_DQUO, KC_QUOT, KC_SCLN

#define _NAVIGATION_L1__    KC_ESC,  KC_7, KC_8, KC_9, KC_TRNS
#define _NAVIGATION_L2__    KC_TAB,  KC_4, KC_5, KC_6, KC_TRNS
#define _NAVIGATION_L3__    KC_CAPS, KC_1, KC_2, KC_3, KC_0
#define _NAVIGATION_R1__                                        KC_HOME, KC_PGUP, KC_UP,   KC_PGDN, KC_BSPC
#define _NAVIGATION_R2__                                        KC_END,  KC_LEFT, KC_DOWN, KC_RGHT, KC_ENT
#define _NAVIGATION_R3__                                        KC_UNDO, KC_CUT,  KC_COPY, KC_PSTE, KC_DEL

#define _DIACRITICS_L1__    KC_TRNS, RALT(KC_Y), KC_TRNS,    KC_TRNS,    KC_TRNS
#define _DIACRITICS_L2__    KC_TRNS, KC_TRNS,    RALT(KC_5), RALT(KC_Q), RALT(KC_P)
#define _DIACRITICS_L3__    KC_TRNS, KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS
#define _DIACRITICS_R1__                                                             KC_TRNS, RGB_TOG, RGB_VAI, RGB_MOD,  KC_TRNS
#define _DIACRITICS_R2__                                                             KC_TRNS, BL_OFF,  RGB_VAD, RGB_RMOD, RALT(KC_S)
#define _DIACRITICS_R3__                                                             KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,  KC_TRNS

#define __FUNCTION_L1___    KC_F1,   KC_F2,  KC_F3,  KC_F4,  KC_TRNS
#define __FUNCTION_L2___    KC_F5,   KC_F6,  KC_F7,  KC_F8,  KC_TRNS
#define __FUNCTION_L3___    KC_F9,   KC_F10, KC_F11, KC_F12, KC_TRNS
#define __FUNCTION_R1___                                              RGB_TOG, RGB_MODE_FORWARD, KC_TRNS, LALT(KC_F4), RESET
#define __FUNCTION_R2___                                              KC_TRNS, RGB_MODE_REVERSE, KC_TRNS, KC_TRNS,     KC_TRNS
#define __FUNCTION_R3___                                              KC_TRNS, KC_TRNS,          KC_TRNS, KC_TRNS,     KC_TRNS
