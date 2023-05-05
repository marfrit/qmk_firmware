#include "marfrit.h"

#define LAYOUT_terminal102_wrapper(...) LAYOUT_terminal102(__VA_ARGS__)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
	[_HIEAO] = LAYOUT_terminal102_wrapper(
    KC_ESC,                     ____FRW1_12_____,                                    _____PRTETC_____,
    ALL_T(KC_GRV),    ___NUMBER_L1____, ___NUMBER_R1____, KC_MINS, KC_EQL,  KC_BSPC, ____INSHMPU_____,          _____NUMR1______,
    KC_TAB,           ____HIEAO_L1____, ____HIEAO_R1____, KC_LBRC, KC_RBRC, KC_BSLS, ____DELENPD_____,          _____NUMR2______,
    ______CLMC______, ____HIEAO_L2____, ____HIEAO_R2____, KC_QUOT,          KC_ENT,                             _____NUMR3T_____,
    KC_LSFT,          ____HIEAO_L3____, ____HIEAO_R3____,                   KC_RSFT,          KC_UP,            _____NUMR4______,
                                _____CASPC______,                                    KC_LEFT, KC_DOWN, KC_RGHT, _____NUMR5______),
	[_SYMBOLS] = LAYOUT_terminal102_wrapper(
    QK_BOOT,                    ____FRW1_12_____,                                    _____PRTETC_____,
    ALL_T(KC_GRV),    ___NUMBER_L1____, ___NUMBER_R1____, KC_MINS, KC_EQL,  KC_BSPC, ____INSHMPU_____,          _____NUMR1______,
    KC_TAB,           ___SYMBOL_L1____, ___SYMBOL_R1____, KC_LBRC, KC_RBRC, KC_BSLS, ____DELENPD_____,          _____NUMR2______,
    ______CLMC______, ___SYMBOL_L2____, ___SYMBOL_R2____, KC_QUOT,          KC_ENT,                             _____NUMR3T_____,
    KC_LSFT,          ___SYMBOL_L3____, ___SYMBOL_R3____,                   KC_RSFT,          KC_UP,            _____NUMR4______,
                                _____CASPC______,                                    KC_LEFT, KC_DOWN, KC_RGHT, _____NUMR5______),
	[_NAVIGATION] = LAYOUT_terminal102_wrapper(
    EE_CLR,                     ____FRW1_12_____,                                    _____PRTETC_____,
    ALL_T(KC_GRV),    ___NUMBER_L1____, ___NUMBER_R1____, KC_MINS, KC_EQL,  KC_BSPC, ____INSHMPU_____,          _____NUMR1______,
    KC_TAB,           _NAVIGATION_L1__, _NAVIGATION_R1__, KC_LBRC, KC_RBRC, KC_BSLS, ____DELENPD_____,          _____NUMR2______,
    ______CLMC______, _NAVIGATION_L2__, _NAVIGATION_R2__, KC_QUOT,          KC_ENT,                             _____NUMR3T_____,
    KC_LSFT,          _NAVIGATION_L3__, _NAVIGATION_R3__,                   KC_RSFT,          KC_UP,            _____NUMR4______,
                                _____CASPC______,                                    KC_LEFT, KC_DOWN, KC_RGHT, _____NUMR5______),
	[_DIACRITICS] = LAYOUT_terminal102_wrapper(
    QK_MAKE,                    ____FRW1_12_____,                                    _____PRTETC_____,
    ALL_T(KC_GRV),    ___NUMBER_L1____, ___NUMBER_R1____, KC_MINS, KC_EQL,  KC_BSPC, ____INSHMPU_____,          _____NUMR1______,
    KC_TAB,           _DIACRITICS_L1__, _DIACRITICS_R1__, KC_LBRC, KC_RBRC, KC_BSLS, ____DELENPD_____,          _____NUMR2______,
    ______CLMC______, _DIACRITICS_L2__, _DIACRITICS_R2__, KC_QUOT,          KC_ENT,                             _____NUMR3T_____,
    KC_LSFT,          _DIACRITICS_L3__, _DIACRITICS_R3__,                   KC_RSFT,          KC_UP,            _____NUMR4______,
                                _____CASPC______,                                    KC_LEFT, KC_DOWN, KC_RGHT, _____NUMR5______),
	[_FUNCTION] = LAYOUT_terminal102_wrapper(
    KC_ESC,                     ____FRW1_12_____,                                    _____PRTETC_____,
    ALL_T(KC_GRV),    ___NUMBER_L1____, ___NUMBER_R1____, KC_MINS, KC_EQL,  KC_BSPC, ____INSHMPU_____,          _____NUMR1______,
    KC_TAB,           __FUNCTION_L1___, __FUNCTION_R1___, KC_LBRC, KC_RBRC, KC_BSLS, ____DELENPD_____,          _____NUMR2______,
    ______CLMC______, __FUNCTION_L2___, __FUNCTION_R2___, KC_QUOT,          KC_ENT,                             _____NUMR3T_____,
    KC_LSFT,          __FUNCTION_L3___, __FUNCTION_R3___,                   KC_RSFT,          KC_UP,            _____NUMR4______,
                                _____CASPC______,                                    KC_LEFT, KC_DOWN, KC_RGHT, _____NUMR5______),
};
