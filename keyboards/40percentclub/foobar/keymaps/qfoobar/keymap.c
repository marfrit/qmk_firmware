#include QMK_KEYBOARD_H

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
	[0] = LAYOUT_ortho_3x10(
        KC_K,         KC_U,         KC_Q,          LT(3,KC_DOT),   KC_J,          KC_V,       LT(3,KC_G),   KC_C,         KC_L,         KC_F,
        LGUI_T(KC_H), LALT_T(KC_I), LCTL_T(KC_E),  LSFT_T(KC_A),   KC_O,          KC_D,       RSFT_T(KC_T), RCTL_T(KC_R), LALT_T(KC_N), RGUI_T(KC_S),
        KC_X,         RALT_T(KC_Y), LT(4,KC_QUOT), LT(1, KC_COMM), LT(2,KC_SPC),  LT(2,KC_B), LT(1, KC_P),  LT(4,KC_W),   RALT_T(KC_M), KC_Z
        ),
	[1] = LAYOUT_ortho_3x10(
        KC_AT,   KC_UNDS, KC_LBRC, KC_RBRC, KC_CIRC,  KC_EXLM, KC_LT,   KC_GT,   KC_EQL,  KC_AMPR,
        KC_BSLS, KC_SLSH, KC_LCBR, KC_RCBR, KC_ASTR,  KC_QUES, KC_LPRN, KC_RPRN, KC_MINS, KC_COLN,
        KC_HASH, KC_DLR,  KC_PIPE, KC_TILD, KC_GRV,   KC_PLUS, KC_PERC, KC_DQUO, KC_QUOT, KC_SCLN
        ),
	[2] = LAYOUT_ortho_3x10(
        KC_ESC,  KC_7,    KC_8,    KC_9,    KC_TRNS,  KC_HOME, KC_PGUP, KC_UP,   KC_PGDN, KC_BSPC,
        KC_TAB,  KC_4,    KC_5,    KC_6,    KC_TRNS,  KC_END,  KC_LEFT, KC_DOWN, KC_RGHT, KC_ENT,
        KC_CAPS, KC_1,    KC_2,    KC_3,    KC_0,     KC_UNDO, KC_CUT,  KC_COPY, KC_PSTE, KC_DEL
        ),
	[3] = LAYOUT_ortho_3x10(
        KC_TRNS, RALT(KC_Y), KC_TRNS,    KC_TRNS,    KC_TRNS,     KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS,    RALT(KC_5), RALT(KC_Q), RALT(KC_P),  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, RALT(KC_S),
        KC_TRNS, KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,     KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
        ),
	[4] = LAYOUT_ortho_3x10(
        KC_F1, KC_F2,  KC_F3,  KC_F4,  KC_TRNS,  RGB_TOG, RGB_MODE_FORWARD, KC_TRNS, LALT(KC_F4), RESET,
        KC_F5, KC_F6,  KC_F7,  KC_F8,  KC_TRNS,  KC_TRNS, RGB_MODE_REVERSE, KC_TRNS, KC_TRNS,     KC_TRNS,
        KC_F9, KC_F10, KC_F11, KC_F12, KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,     KC_TRNS
        ),
};

layer_state_t layer_state_set_user(layer_state_t state) {
    switch (get_highest_layer(state)) {
    case 1:
        rgblight_setrgb (0x00,  0x00, 0xFF);
        break;
    case 2:
        rgblight_setrgb (0xFF,  0x00, 0x00);
        break;
    case 3:
        rgblight_setrgb (0x00,  0xFF, 0x00);
        break;
    case 4:
        rgblight_setrgb (0x7A,  0x00, 0xFF);
        break;
    default: //  for any other layers, or the default layer
        rgblight_setrgb (0x00,  0xFF, 0xFF);
        break;
    }
  return state;
}
