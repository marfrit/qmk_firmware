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
int cur_dance (qk_tap_dance_state_t *state);

// `finished` and `reset` functions for each tapdance keycode
void altlp_finished (qk_tap_dance_state_t *state, void *user_data);
void altlp_reset (qk_tap_dance_state_t *state, void *user_data);

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
	[0] = LAYOUT(
        ALL_T(KC_ESC), KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_BSPC,
        KC_TAB, KC_K, KC_U, KC_Q, LT(3,KC_DOT), KC_J, KC_V, LT(3,KC_G), KC_C, KC_L, KC_F, KC_DEL,
        KC_LSFT, KC_H, LALT_T(KC_I), LCTL_T(KC_E), LSFT_T(KC_A), KC_O, KC_D, RSFT_T(KC_T), RCTL_T(KC_R), LALT_T(KC_N), KC_S, KC_ENT,
        KC_LGUI, KC_X, RALT_T(DE_Y), DE_SCLN, LT(1,DE_COMM), LT(2, DE_SLSH), DE_LPRN, DE_RPRN, LT(2,KC_B), LT(1,KC_P), KC_W, RALT_T(KC_M), DE_Z, KC_RSFT,
        MO(3), MO(2), MO(1), KC_SPC, KC_BSPC, MO(1), MO(2), MO(3)),
	[1] = LAYOUT(
        KC_F11, KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_F12,
        KC_TRNS, DE_AT,   DE_UNDS, DE_LBRC, DE_RBRC, DE_CIRC, DE_EXLM, DE_LABK, DE_RABK, DE_EQL,  DE_AMPR, KC_TRNS,
        KC_TRNS, DE_BSLS, DE_SLSH, DE_LCBR, DE_RCBR, DE_ASTR, DE_QUES, DE_LPRN, DE_RPRN, DE_MINS, DE_COLN, KC_TRNS,
        KC_TRNS, DE_HASH, DE_DLR,  DE_PIPE, DE_TILD, DE_GRV,  KC_TRNS, KC_TRNS, DE_PLUS, DE_PERC, DE_DQUO, DE_QUOT, DE_SCLN, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, MO(3), KC_TRNS, KC_TRNS),
	[2] = LAYOUT(
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_ESC, KC_7, KC_8, KC_9, KC_TRNS, KC_HOME, KC_PGUP, KC_UP, KC_PGDN, KC_BSPC, KC_TRNS,
        KC_TRNS, KC_TAB, KC_4, KC_5, KC_6, KC_TRNS, KC_END, KC_LEFT, KC_DOWN, KC_RGHT, KC_ENT, KC_TRNS,
        KC_TRNS, KC_CAPS, KC_1, KC_2, KC_3, KC_0, KC_TRNS, KC_TRNS, KC_UNDO, KC_CUT, KC_COPY, KC_PSTE, KC_DEL, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS),
	[3] = LAYOUT(
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, QK_BOOT,
        KC_TRNS, KC_TRNS, DE_UDIA, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, RGB_TOG, RGB_VAI, RGB_MOD, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, DE_EURO, DE_ADIA, DE_ODIA, KC_TRNS, BL_OFF, RGB_VAD, RGB_RMOD, DE_SS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS)
};

// determine the tapdance state to return
int cur_dance (qk_tap_dance_state_t *state) {
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

