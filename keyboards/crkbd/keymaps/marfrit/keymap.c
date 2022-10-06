#include "marfrit.h"

#define LAYOUT_split_3x6_3_wrapper(...) LAYOUT_split_3x6_3(__VA_ARGS__)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
	[_HIEAO] = LAYOUT_split_3x6_3_wrapper(
        ALL_T(KC_ESC), ____HIEAO_L1____, ____HIEAO_R1____, KC_BSPC,
        KC_TAB,        ____HIEAO_L2____, ____HIEAO_R2____, KC_ENT,
        KC_LSFT,       ____HIEAO_L3____, ____HIEAO_R3____, KC_RSFT,
               KC_LCTL, KC_LALT, KC_SPC, KC_BSPC, KC_LALT, KC_LCTL
        ),
	[_SYMBOLS] = LAYOUT_split_3x6_3_wrapper(
        ALL_T(KC_ESC), ___SYMBOL_L1____, ___SYMBOL_R1____, KC_BSPC,
        KC_TAB,        ___SYMBOL_L2____, ___SYMBOL_R2____, KC_ENT,
        KC_LSFT,       ___SYMBOL_L3____, ___SYMBOL_R3____, KC_RSFT,
               KC_LCTL, KC_LALT, KC_SPC, KC_BSPC, KC_LALT, KC_LCTL
        ),
	[_NAVIGATION] = LAYOUT_split_3x6_3_wrapper(
        ALL_T(KC_ESC), _NAVIGATION_L1__, _NAVIGATION_R1__, KC_BSPC,
        KC_TAB,        _NAVIGATION_L2__, _NAVIGATION_R2__, KC_ENT,
        KC_LSFT,       _NAVIGATION_L3__, _NAVIGATION_R3__, KC_RSFT,
               KC_LCTL, KC_LALT, KC_SPC, KC_BSPC, KC_LALT, KC_LCTL
        ),
	[_DIACRITICS] = LAYOUT_split_3x6_3_wrapper(
        ALL_T(KC_ESC), _DIACRITICS_L1__, _DIACRITICS_R1__, KC_BSPC,
        KC_TAB,        _DIACRITICS_L2__, _DIACRITICS_R2__, KC_ENT,
        KC_LSFT,       _DIACRITICS_L3__, _DIACRITICS_R3__, KC_RSFT,
               KC_LCTL, KC_LALT, KC_SPC, KC_BSPC, KC_LALT, KC_LCTL
        ),
	[_FUNCTION] = LAYOUT_split_3x6_3_wrapper(
        ALL_T(KC_ESC), __FUNCTION_L1___, __FUNCTION_R1___, KC_BSPC,
        KC_TAB,        __FUNCTION_L2___, __FUNCTION_R2___, KC_ENT,
        KC_LSFT,       __FUNCTION_L3___, __FUNCTION_R3___, KC_RSFT,
               KC_LCTL, KC_LALT, KC_SPC, KC_BSPC, KC_LALT, KC_LCTL
        )
};

#ifdef OLED_ENABLE
oled_rotation_t oled_init_user(oled_rotation_t rotation) {
  if (!is_keyboard_master()) {
    return OLED_ROTATION_180;  // flips the display 180 degrees if offhand
  }
  return rotation;
}

#define L_BASE 0
#define L_LOWER 2
#define L_RAISE 4
#define L_ADJUST 8

void oled_render_layer_state(void) {
    oled_write_P(PSTR("Layer: "), false);
    switch (layer_state) {
        case L_BASE:
            oled_write_ln_P(PSTR("Default"), false);
            break;
        case L_LOWER:
            oled_write_ln_P(PSTR("Lower"), false);
            break;
        case L_RAISE:
            oled_write_ln_P(PSTR("Raise"), false);
            break;
        case L_ADJUST:
        case L_ADJUST|L_LOWER:
        case L_ADJUST|L_RAISE:
        case L_ADJUST|L_LOWER|L_RAISE:
            oled_write_ln_P(PSTR("Adjust"), false);
            break;
    }
}


char keylog_str[24] = {};

const char code_to_name[60] = {
    ' ', ' ', ' ', ' ', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
    'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
    'R', 'E', 'B', 'T', '_', '-', '=', '[', ']', '\\',
    '#', ';', '\'', '`', ',', '.', '/', ' ', ' ', ' '};

void set_keylog(uint16_t keycode, keyrecord_t *record) {
  char name = ' ';
    if ((keycode >= QK_MOD_TAP && keycode <= QK_MOD_TAP_MAX) ||
        (keycode >= QK_LAYER_TAP && keycode <= QK_LAYER_TAP_MAX)) { keycode = keycode & 0xFF; }
  if (keycode < 60) {
    name = code_to_name[keycode];
  }

  // update keylog
  snprintf(keylog_str, sizeof(keylog_str), "%dx%d, k%2d : %c",
           record->event.key.row, record->event.key.col,
           keycode, name);
}

void oled_render_keylog(void) {
    oled_write(keylog_str, false);
}

void render_bootmagic_status(bool status) {
    /* Show Ctrl-Gui Swap options */
    static const char PROGMEM logo[][2][3] = {
        {{0x97, 0x98, 0}, {0xb7, 0xb8, 0}},
        {{0x95, 0x96, 0}, {0xb5, 0xb6, 0}},
    };
    if (status) {
        oled_write_ln_P(logo[0][0], false);
        oled_write_ln_P(logo[0][1], false);
    } else {
        oled_write_ln_P(logo[1][0], false);
        oled_write_ln_P(logo[1][1], false);
    }
}

void oled_render_logo(void) {
    static const char PROGMEM crkbd_logo[] = {
        0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f, 0x90, 0x91, 0x92, 0x93, 0x94,
        0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb1, 0xb2, 0xb3, 0xb4,
        0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xd0, 0xd1, 0xd2, 0xd3, 0xd4,
        0};
    oled_write_P(crkbd_logo, false);
}

bool oled_task_user(void) {
    if (is_keyboard_master()) {
        oled_render_layer_state();
        oled_render_keylog();
    } else {
        oled_render_logo();
    }
    return false;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  if (record->event.pressed) {
    set_keylog(keycode, record);
  }
  return true;
}
#endif // OLED_ENABLE
