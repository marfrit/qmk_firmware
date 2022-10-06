#include "marfrit.h"

#define LAYOUT_split_3x6_3_wrapper(...) LAYOUT_split_3x6_3(__VA_ARGS__)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
	[_HIEAO] = LAYOUT_split_3x6_3_wrapper(
        ALL_T(KC_ESC), ____HIEAO_L1____, ____HIEAO_R1____, KC_BSPC,
        KC_TAB,        ____HIEAO_L2____, ____HIEAO_R2____, KC_ENT,
        KC_LSFT,       ____HIEAO_L3____, ____HIEAO_R3____, KC_RSFT,
               KC_LCTL, KC_LALT, KC_SPC, KC_BSPC, KC_LALT, KC_LCTL),
	[_SYMBOLS] = LAYOUT_split_3x6_3_wrapper(
        ALL_T(KC_ESC), ___SYMBOL_L1____, ___SYMBOL_R1____, KC_BSPC,
        KC_TAB,        ___SYMBOL_L2____, ___SYMBOL_R2____, KC_ENT,
        KC_LSFT,       ___SYMBOL_L3____, ___SYMBOL_R3____, KC_RSFT,
               KC_LCTL, KC_LALT, KC_SPC, KC_BSPC, KC_LALT, KC_LCTL),
	[_NAVIGATION] = LAYOUT_split_3x6_3_wrapper(
        ALL_T(KC_ESC), _NAVIGATION_L1__, _NAVIGATION_R1__, KC_BSPC,
        KC_TAB,        _NAVIGATION_L2__, _NAVIGATION_R2__, KC_ENT,
        KC_LSFT,       _NAVIGATION_L3__, _NAVIGATION_R3__, KC_RSFT,
               KC_LCTL, KC_LALT, KC_SPC, KC_BSPC, KC_LALT, KC_LCTL),
	[_DIACRITICS] = LAYOUT_split_3x6_3_wrapper(
        ALL_T(KC_ESC), _DIACRITICS_L1__, _DIACRITICS_R1__, KC_BSPC,
        KC_TAB,        _DIACRITICS_L2__, _DIACRITICS_R2__, KC_ENT,
        KC_LSFT,       _DIACRITICS_L3__, _DIACRITICS_R3__, KC_RSFT,
               KC_LCTL, KC_LALT, KC_SPC, KC_BSPC, KC_LALT, KC_LCTL),
	[_FUNCTION] = LAYOUT_split_3x6_3_wrapper(
        ALL_T(KC_ESC), __FUNCTION_L1___, __FUNCTION_R1___, KC_BSPC,
        KC_TAB,        __FUNCTION_L2___, __FUNCTION_R2___, KC_ENT,
        KC_LSFT,       __FUNCTION_L3___, __FUNCTION_R3___, KC_RSFT,
               KC_LCTL, KC_LALT, KC_SPC, KC_BSPC, KC_LALT, KC_LCTL),
};
