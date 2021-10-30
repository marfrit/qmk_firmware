#include QMK_KEYBOARD_H

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [0] = LAYOUT( /* Base layer */
    KC_ESC,               KC_F1,        KC_F2,        KC_F3,          KC_F4,         KC_F5,   KC_F6,        KC_F7,        KC_F8,        KC_F9,        KC_F10,  KC_F11,  KC_F12,    KC_PSCR, RESET,   KC_PAUS,
    KC_GRV,  KC_1,        KC_2,         KC_3,         KC_4,           KC_5,          KC_6,    KC_7,         KC_8,         KC_9,         KC_0,         KC_MINS, KC_EQL,  KC_BSPC,   KC_INS,  KC_HOME, KC_PGUP,   KC_NLCK, KC_PSLS, KC_PAST, KC_PMNS,
    KC_TAB,  ALL_T(KC_K), KC_U,         KC_Q,         KC_DOT,         KC_J,          KC_V,    KC_G,         KC_C,         KC_L,         KC_F,         KC_LBRC, KC_RBRC, KC_BSLS,   KC_DEL,  KC_END,  KC_PGDN,   KC_P7,   KC_P8,   KC_P9,   KC_PPLS,
    KC_CAPS, KC_H,        LALT_T(KC_I), LCTL_T(KC_E), LSFT_T(KC_A),   KC_O,          KC_D,    RSFT_T(KC_T), RCTL_T(KC_R), LALT_T(KC_N), KC_S,         KC_QUOT, KC_NUHS, KC_ENT,                                 KC_P4,   KC_P5,   KC_P6,   KC_PCMM,
    KC_LSFT, KC_NUBS,     KC_X,         RALT_T(KC_Y), LT(3, KC_SCLN), LT(1,KC_COMM), KC_SLSH, KC_B,         LT(2, KC_P),  LT(4, KC_W),  RALT_T(KC_M), KC_Z,             KC_RSFT,            KC_UP,              KC_P1,   KC_P2,   KC_P3,   KC_PENT,
    KC_LCTL,              KC_LALT,                           KC_SPC,                                      KC_RALT,          KC_RCTL,   KC_LEFT, KC_DOWN, KC_RGHT,   KC_P0,            KC_PDOT
  ),
  // 15
  // 21
  // 21
  // 17
  // 17
  // 11
  [1] = LAYOUT(
    KC_NO, KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO, KC_NO, KC_NO, KC_NO,
    KC_NO, KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO,
    KC_NO, KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_UNDS, KC_SCLN, KC_COLN, KC_PSCR, KC_QUES, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO,
    KC_NO, KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_PIPE, KC_LT,   KC_GT,   KC_PAUS, KC_EXLM, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO,
    KC_NO, KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_COPY, KC_PSTE, KC_CUT,  KC_UNDO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO,
    KC_NO, KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO),
  [2] = LAYOUT(
    KC_NO, KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO, KC_NO, KC_NO, KC_NO,
    KC_NO, KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO,
    KC_NO, KC_PLUS, KC_MINS, KC_LBRC, KC_RBRC, KC_DQUO, KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO,
    KC_NO, KC_ASTR, KC_SLSH, KC_LPRN, KC_RPRN, KC_QUOT, KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO,
    KC_NO, KC_NO,   KC_EQL,  KC_BSLS, KC_LCBR, KC_RCBR, KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO,
    KC_NO, KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO ,  KC_NO,   KC_NO,   KC_NO),
   [3] = LAYOUT(
    KC_NO, KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO, KC_NO, KC_NO, KC_NO,
    KC_NO, KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO,
    KC_NO, KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO,
    KC_NO, KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_F6,   KC_F7,   KC_F8,   KC_BSPC, KC_ENT,  KC_NO,   KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO,
    KC_NO, KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_F9,   KC_F10,  KC_DEL,  KC_TAB,  KC_NO,   KC_NO,   KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO,
    KC_NO, KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO),
   [4] = LAYOUT(
    KC_NO, KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO, KC_NO, KC_NO, KC_NO,
    KC_NO, KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO,
    KC_NO, KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO,
    KC_NO, KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO,
    KC_NO, KC_NO,   KC_F11,  KC_F12,  KC_ESC,  KC_INS,  KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO,
    KC_NO, KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO)
};
