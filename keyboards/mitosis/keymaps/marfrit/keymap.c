#include "marfrit.h"

#define LAYOUT_wrapper(...) LAYOUT(__VA_ARGS__)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
	[_HIEAO] = LAYOUT_wrapper(
        ____HIEAO_L1____,                       ____HIEAO_R1____,
        ____HIEAO_L2____,                       ____HIEAO_R2____,
        ____HIEAO_L3____,                       ____HIEAO_R3____,
      KC_LGUI, KC_LCTL, KC_SPC, KC_ENT,         KC_SPC,  KC_SPC, KC_RCTL, KC_RGUI,
      KC_LGUI, FUNC(KC_PGUP), SYMB(KC_PGDN), NAVI(KC_SPC), NAVI(KC_BSPC), SYMB(KC_DEL), FUNC(KC_ENT), KC_LGUI
        ),
	[_SYMBOLS] = LAYOUT_wrapper(
        ___SYMBOL_L1____,                       ___SYMBOL_R1____,
        ___SYMBOL_L2____,                       ___SYMBOL_R2____,
        ___SYMBOL_L3____,                       ___SYMBOL_R3____,
      KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
      KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
         ),
	[_NAVIGATION] = LAYOUT_wrapper(
        _NAVIGATION_L1__,                       _NAVIGATION_R1__,
        _NAVIGATION_L2__,                       _NAVIGATION_R2__,
        _NAVIGATION_L3__,                       _NAVIGATION_R3__,
      KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
      KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
         ),
	[_DIACRITICS] = LAYOUT_wrapper(
        _DIACRITICS_L1__,                       _DIACRITICS_R1__,
        _DIACRITICS_L2__,                       _DIACRITICS_R2__,
        _DIACRITICS_L3__,                       _DIACRITICS_R3__,
      KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
      KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
         ),
	[_FUNCTION] = LAYOUT_wrapper(
        __FUNCTION_L1___,                       __FUNCTION_R1___,
        __FUNCTION_L2___,                       __FUNCTION_R2___,
        __FUNCTION_L3___,                       __FUNCTION_R3___,
      KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
      KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
         )
};

