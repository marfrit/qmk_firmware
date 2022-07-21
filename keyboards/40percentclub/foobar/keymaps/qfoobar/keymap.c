#include QMK_KEYBOARD_H
#include "keymap_german.h"

// tapdance keycodes
enum td_keycodes {
  LT2_SLSH // Our example key: `Layer 2` when held, `Shift+7` when tapped. Add additional keycodes for each tapdance.
};

// define a type containing as many tapdance states as you need
typedef enum {
  SINGLE_TAP,
  SINGLE_HOLD
} td_state_t;

// create a global instance of the tapdance state type
static td_state_t td_state;

// declare your tapdance functions:

// function to determine the current tapdance state
td_state_t cur_dance(qk_tap_dance_state_t *state);

// `finished` and `reset` functions for each tapdance keycode
void altlp_finished (qk_tap_dance_state_t *state, void *user_data);
void altlp_reset (qk_tap_dance_state_t *state, void *user_data);


const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
	[0] = LAYOUT_ortho_3x10(KC_K, KC_U, KC_Q, KC_DOT, KC_J, KC_V, KC_G, KC_C, KC_L, KC_F,
    LGUI_T(KC_H), LALT_T(KC_I), LCTL_T(KC_E), LSFT_T(KC_A), LT(2,KC_O), KC_D, RSFT_T(KC_T), RCTL_T(KC_R), LALT_T(KC_N), RGUI_T(KC_S),
//    KC_X, RALT_T(KC_Y), KC_QUOT, TD(LT2_SLSH), LT(1,KC_SPC), LT(2,KC_B), KC_P, KC_W, RALT_T(KC_M), KC_Z),
    KC_X, RALT_T(KC_Y), KC_QUOT, LT(2, KC_A), LT(1,KC_SPC), LT(2,KC_B), KC_P, KC_W, RALT_T(KC_M), KC_Z),
	[1] = LAYOUT_ortho_3x10(KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_F6, KC_F7, KC_F8, KC_BSPC, KC_ENT, KC_F11, KC_F12, KC_ESC, KC_INS, KC_TRNS, MO(3), KC_F9, KC_F10, KC_DEL, KC_TAB),
	[2] = LAYOUT_ortho_3x10(KC_PLUS, KC_MINS, KC_LBRC, KC_RBRC, KC_DQUO, KC_UNDS, KC_SCLN, KC_COLN, KC_PSCR, KC_QUES, KC_ASTR, KC_SLSH, KC_LPRN, KC_RPRN, KC_QUOT, KC_PIPE, KC_LT, KC_GT, KC_PAUS, KC_EXLM, KC_EQL, KC_NUBS, KC_LCBR, KC_RCBR, MO(4), KC_TRNS, LCTL(KC_C), LCTL(KC_V), LCTL(KC_X), LCTL(KC_Y)),
	[3] = LAYOUT_ortho_3x10(KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, RESET, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_LEFT, KC_DOWN, KC_UP, KC_RGHT, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_HOME, KC_PGDN, KC_PGUP, KC_END),
	[4] = LAYOUT_ortho_3x10(KC_EXLM, KC_AT, KC_HASH, KC_DLR, KC_PERC, KC_CIRC, KC_AMPR, KC_ASTR, KC_QUES, RESET, KC_TILD, KC_GRV, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS)
};

// determine the tapdance state to return
td_state_t cur_dance (qk_tap_dance_state_t *state) {
  if (state->count == 1) {
    if (state->interrupted || !state->pressed) { return SINGLE_TAP; }
    else { return SINGLE_HOLD; }
  }
  else { return 2; } // any number higher than the maximum state value you return above
}

// handle the possible states for each tapdance keycode you define:

void altlp_finished (qk_tap_dance_state_t *state, void *user_data) {
  td_state = cur_dance(state);
  switch (td_state) {
    case SINGLE_TAP:
        register_code(KC_LSFT);
        register_code(KC_7);
      break;
    case SINGLE_HOLD:
      layer_on(2);
      break;
  }
}

void altlp_reset (qk_tap_dance_state_t *state, void *user_data) {
  //if the key was held down and now is released then switch off the layer
  if (td_state==SINGLE_HOLD) {
    layer_off(2);
  }
  if (td_state==SINGLE_TAP) {
        unregister_code(KC_7);
        unregister_code(KC_LSFT);
  }
  td_state = 0;
}
// define `ACTION_TAP_DANCE_FN_ADVANCED()` for each tapdance keycode, passing in `finished` and `reset` functions
qk_tap_dance_action_t tap_dance_actions[] = {
  [LT2_SLSH] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, altlp_finished, altlp_reset)
};

