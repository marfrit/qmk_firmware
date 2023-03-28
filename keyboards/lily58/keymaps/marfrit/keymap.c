#include "marfrit.h"

#define LAYOUT_wrapper(...) LAYOUT(__VA_ARGS__)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
	[_HIEAO] = LAYOUT_wrapper(
/*
 * ,-----------------------------------------.                    ,-----------------------------------------.
 * | ESC  |   1  |   2  |   3  |   4  |   5  |                    |   6  |   7  |   8  |   9  |   0  | BSPC |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * | Tab  |   K  |   U  |   Q  |   .  |   J  |                    |   V  |   G  |   C  |   L  |   F  | DEL  |
 * |------+------+------+------+------+------|                    |------+------+------+------+------+------|
 * |LShift|   H  |   I  |   E  |   A  |   O  |-------.    ,-------|   D  |   T  |   R  |   N  |   S  |Enter |
 * |------+------+------+------+------+------|   [   |    |    ]  |------+------+------+------+------+------|
 * |LGUI  |   X  |   Y  |   ;  |   ,  |   /  |-------|    |-------|   B  |   P  |   W  |   M  |   Z  |RShift|
 * `-----------------------------------------/       /     \      \-----------------------------------------'
 *                   | LAlt | LGUI |LOWER | /Space  /       \Enter \  |RAISE |BackSP| RGUI |
 *                   |      |      |      |/       /         \      \ |      |      |      |
 *                   `----------------------------'           '------''--------------------'
*/
        ALL_T(KC_ESC), ___NUMBER_L1____,                                               ___NUMBER_R1____, KC_BSPC,
        KC_TAB,        ____HIEAO_L1____,                                               ____HIEAO_R1____, KC_DEL,
        KC_LSFT,       ____HIEAO_L2____,                                               ____HIEAO_R2____, KC_ENT,
        KC_LGUI,       ____HIEAO_L3____,                      MUTE_TEAMS,  MUTE_SKYPE, ____HIEAO_R3____, KC_RSFT,
                                         MO(3), MO(2), MO(1), KC_SPC,      KC_BSPC,    MO(1), MO(2), MO(3)),
	[_SYMBOLS] = LAYOUT_wrapper(
        KC_F11,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,                      KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F12,
        KC_TRNS, ___SYMBOL_L1____,                                               ___SYMBOL_R1____,                            KC_TRNS,
        KC_TRNS, ___SYMBOL_L2____,                                               ___SYMBOL_R2____,                            KC_TRNS,
        KC_TRNS, ___SYMBOL_L3____,                            KC_TRNS,  KC_TRNS, ___SYMBOL_R3____,                            KC_TRNS,
                                   KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,  KC_TRNS, MO(3), KC_TRNS, KC_TRNS),

	[_NAVIGATION] = LAYOUT_wrapper(
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,                     KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, _NAVIGATION_L1__,                                               _NAVIGATION_R1__,                             KC_TRNS,
        KC_TRNS, _NAVIGATION_L2__,                                               _NAVIGATION_R2__,                             KC_TRNS,
        KC_TRNS, _NAVIGATION_L3__,                            KC_TRNS,  KC_TRNS, _NAVIGATION_R3__,                             KC_TRNS,
                                   KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS),

	[_DIACRITICS] = LAYOUT_wrapper(
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,                    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,  KC_TRNS, QK_BOOT,
        KC_TRNS, _DIACRITICS_L1__,                                               _DIACRITICS_R1__,                             KC_TRNS,
        KC_TRNS, _DIACRITICS_L2__,                                               _DIACRITICS_R2__,                             KC_TRNS,
        KC_TRNS, _DIACRITICS_L3__,                            KC_TRNS,  KC_TRNS, _DIACRITICS_R3__,                             KC_TRNS,
                                   KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS),

    [_FUNCTION] = LAYOUT_wrapper(
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,                    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,  KC_TRNS, QK_BOOT,
        KC_TRNS, __FUNCTION_L1___,                                               __FUNCTION_R1___,                             KC_TRNS,
        KC_TRNS, __FUNCTION_L2___,                                               __FUNCTION_R2___,                             KC_TRNS,
        KC_TRNS, __FUNCTION_L3___,                            KC_TRNS,  KC_TRNS, __FUNCTION_R3___,                             KC_TRNS,
                                   KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS),
};
