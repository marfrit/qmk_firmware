#include "marfrit.h"

#define LAYOUT_ansi_hhkb_split_shift_split_backspace_wrapper(...) LAYOUT_ansi_hhkb_split_shift_split_backspace(__VA_ARGS__)

//18
//17
//16
//17
//10

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_HIEAO] = LAYOUT_ansi_hhkb_split_shift_split_backspace_wrapper(
        KC_ESC,           ___NUMBER_L1____, ___NUMBER_R1____, KC_MINS, KC_EQL,  KC_BSLS, KC_GRV,  _____PRTETC_____,
        KC_TAB,           ____HIEAO_L1____, ____HIEAO_R1____, KC_LBRC, KC_RBRC,         KC_BSPC,  ____INSHMPU_____,
        ______CLMC______, ____HIEAO_L2____, ____HIEAO_R2____, KC_QUOT,                   KC_ENT,  ____DELENPD_____,
        KC_LSFT,          ____HIEAO_L3____, ____HIEAO_R3____,             KC_RSFT, MO(_SYMBOLS),  KC_P0,   KC_UP,   KC_PDOT,
        KC_CAPS, KC_LGUI, KC_LALT,   KC_SPC,                  KC_RALT, KC_NLCK, KC_RCTRL,         KC_LEFT, KC_DOWN, KC_RIGHT
    ),
    [_SYMBOLS] = LAYOUT_ansi_hhkb_split_shift_split_backspace_wrapper(
        KC_ESC,           ____FRW1_12_____,                                     KC_BSLS, KC_GRV,  _____PRTETC_____,
        KC_TAB,           ___SYMBOL_L1____, ___SYMBOL_R1____, KC_LBRC, KC_RBRC,         KC_BSPC,  ____INSHMPU_____,
        ______CLMC______, ___SYMBOL_L2____, ___SYMBOL_R2____, KC_QUOT,                   KC_ENT,  ____DELENPD_____,
        KC_LSFT,          ___SYMBOL_L3____, ___SYMBOL_R3____,             KC_RSFT, MO(_SYMBOLS),  KC_P0,   KC_UP,   KC_PDOT,
        KC_CAPS, KC_LGUI, KC_LALT,   KC_SPC,                  KC_RALT, KC_NLCK, KC_RCTRL,         KC_LEFT, KC_DOWN, KC_RIGHT
    ),
    [_NAVIGATION] = LAYOUT_ansi_hhkb_split_shift_split_backspace_wrapper(
        KC_ESC,           ___NUMBER_L1____, ___NUMBER_R1____, KC_MINS, KC_EQL,  KC_BSLS, KC_GRV,  _____PRTETC_____,
        KC_TAB,           _NAVIGATION_L1__, _NAVIGATION_R1__, KC_LBRC, KC_RBRC,         KC_BSPC,  ____INSHMPU_____,
        ______CLMC______, _NAVIGATION_L2__, _NAVIGATION_R2__, KC_QUOT,                   KC_ENT,  ____DELENPD_____,
        KC_LSFT,          _NAVIGATION_L3__, _NAVIGATION_R3__,             KC_RSFT, MO(_SYMBOLS),  KC_P0,   KC_UP,   KC_PDOT,
        KC_CAPS, KC_LGUI, KC_LALT,   KC_SPC,                  KC_RALT, KC_NLCK, KC_RCTRL,         KC_LEFT, KC_DOWN, KC_RIGHT
    ),
    [_DIACRITICS] = LAYOUT_ansi_hhkb_split_shift_split_backspace_wrapper(
        KC_ESC,           ___NUMBER_L1____, ___NUMBER_R1____, KC_MINS, KC_EQL,  KC_BSLS, KC_GRV,  _____PRTETC_____,
        KC_TAB,           _DIACRITICS_L1__, _DIACRITICS_R1__, KC_LBRC, KC_RBRC,         KC_BSPC,  ____INSHMPU_____,
        ______CLMC______, _DIACRITICS_L2__, _DIACRITICS_R2__, KC_QUOT,                   KC_ENT,  ____DELENPD_____,
        KC_LSFT,          _DIACRITICS_L3__, _DIACRITICS_R3__,             KC_RSFT, MO(_SYMBOLS),  KC_P0,   KC_UP,   KC_PDOT,
        KC_CAPS, KC_LGUI, KC_LALT,   KC_SPC,                  KC_RALT, KC_NLCK, KC_RCTRL,         KC_LEFT, KC_DOWN, KC_RIGHT
    ),
    [_FUNCTIONL] = LAYOUT_ansi_hhkb_split_shift_split_backspace_wrapper(
        KC_ESC,           ___NUMBER_L1____, ___NUMBER_R1____, KC_MINS, KC_EQL,  KC_BSLS, KC_GRV,  _____PRTETC_____,
        KC_TAB,           __FUNCTION_L1___, __FUNCTION_R1___, KC_LBRC, KC_RBRC,         KC_BSPC,  ____INSHMPU_____,
        ______CLMC______, __FUNCTION_L2___, __FUNCTION_R2___, KC_QUOT,                   KC_ENT,  ____DELENPD_____,
        KC_LSFT,          __FUNCTION_L3___, __FUNCTION_R3___,             KC_RSFT, MO(_SYMBOLS),  KC_P0,   KC_UP,   KC_PDOT,
        KC_CAPS, KC_LGUI, KC_LALT,   KC_SPC,                  KC_RALT, KC_NLCK, KC_RCTRL,         KC_LEFT, KC_DOWN, KC_RIGHT
    )
};
