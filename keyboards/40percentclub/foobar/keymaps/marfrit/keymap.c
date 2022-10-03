#include "marfrit.h"

// Use an expanded macro with VA_ARGS to ensure that the common
// rows get expanded out before getting passed to the LAYOUT
// macro.

#define LAYOUT_wrapper(...) LAYOUT_ortho_3x10(__VA_ARGS__)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
	[_HIEAO] = LAYOUT_wrapper(
        ____HIEAO_L1____, ____HIEAO_R1____,
        ____HIEAO_L2____, ____HIEAO_R2____,
        ____HIEAO_L3_FB_, ____HIEAO_R3____
        ),
	[_SYMBOLS] = LAYOUT_wrapper(
        ___SYMBOL_L1____,  ___SYMBOL_R1____,
        ___SYMBOL_L2____,  ___SYMBOL_R2____,
        ___SYMBOL_L3____,  ___SYMBOL_R3____
        ),
	[_NAVIGATION] = LAYOUT_wrapper(
        _NAVIGATION_L1__, _NAVIGATION_R1__,
        _NAVIGATION_L2__, _NAVIGATION_R2__,
        _NAVIGATION_L3__, _NAVIGATION_R3__
        ),
	[_DIACRITICS] = LAYOUT_wrapper(
        _DIACRITICS_L1__, _DIACRITICS_R1__,
        _DIACRITICS_L2__, _DIACRITICS_R2__,
        _DIACRITICS_L3__, _DIACRITICS_R3__
        ),
	[_FUNCTION] = LAYOUT_wrapper(
        __FUNCTION_L1___, __FUNCTION_R1___,
        __FUNCTION_L2___, __FUNCTION_R2___,
        __FUNCTION_L3___, __FUNCTION_R3___
        )
};
