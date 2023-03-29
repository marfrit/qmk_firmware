/* Good on you for modifying your layout! if you don't have
 * time to read the QMK docs, a list of keycodes can be found at
 *
 * https://github.com/qmk/qmk_firmware/blob/master/docs/keycodes.md
 *
 * There's also a template for adding new layers at the bottom of this file!
 */

#include "marfrit.h"

#define _BASE   0 // default layer
#define _LOWER  1 // symbols
#define _RAISE  2 // numbers/motion
#define _ADJUST 3
#define _UMLAUT 4

bool override = false;
// Blank template at the bottom

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
/* Keymap 0: Basic layer
 *
 * ,-------------------------------------------.                                       ,-------------------------------------------.
 * |   ESC  |  K   |  U   |  Q   |  .   |  J   |                                       |   V  |  G   |  C   |  L   |  F   |  BSPC  |
 * |--------+------+------+------+------+------|------.                         .------|------+------+------+------+------+--------|
 * |   TAB  |  H   |  I   |  E   |  A   |  O   |      |                         |      |   D  |  T   |  R   |  N   |  S   |  ENT   |
 * |--------+------+------+------+------+------|------|------.           .------|------|------+------+------+------+------+--------|
 * |  LSFT  |  X   |  Y   |  ;   |  ,   |  /   |      |      |           |      |      |   B  |  P   |  W   |  M   |  Z   |  RSFT  |
 * `--------+------+------+------+------+--------------------'           `------+------+------+------+------+------+------+--------'
 *                        .------.              ,-------------.         ,-------------.              .------.
 *                        |  LWR |   .------.   |      |      |         |      |      |   .------.   |  LWR |
 *                        '------'   | RAISE|   | SPC  | ADJ  |         | RAISE| BSPC |   |RAISE |   '------'
 *                                   '------'   |      |      |         |      |      |   '------'
 *                                              `-------------'         `-------------'
 */
[_BASE] = LAYOUT_gergo(
    ALL_T(KC_ESC),   KC_K,         KC_U,         KC_Q,         LT(_UMLAUT,KC_DOT), KC_J,                                                                  KC_V,                LT(_UMLAUT, KC_G), KC_C,   KC_L,  KC_F,         KC_BSPC,
    KC_TAB,          LGUI_T(KC_H), LALT_T(KC_I), LCTL_T(KC_E), LSFT_T(KC_A),       KC_O,                MUTE_TEAMS,                           MUTE_SKYPE, KC_D,                RSFT_T(KC_T),      RCTL_T(KC_R),  LALT_T(KC_N), KC_S, KC_ENT,
    KC_LSFT,         KC_X,         RALT_T(KC_Y), KC_SCLN,      LT(_LOWER,KC_COMM), LT(_RAISE, KC_SLSH), KC_BTN1, KC_BTN3,  KC_PGDN,           KC_BSPC,    LT(_RAISE,KC_B),     LT(_LOWER,KC_P),   KC_W,          RALT_T(KC_M), KC_Z, KC_RSFT,

                                                 MO(_ADJUST),                       MO(_LOWER),         LT(_RAISE, KC_SPC),MO(_ADJUST),       KC_ENT,     LT(_RAISE, KC_BSPC), MO(_LOWER),        MO(_ADJUST)
    ),
/* Keymap 1: Lower layer
 *
 * ,-------------------------------------------.                                       ,-------------------------------------------.
 * |        |  @   |  _   |  [   |  ]   |  ^   |                                       |   !  |  <   |  >   |  =   |  &   |  BSPC  |
 * |--------+------+------+------+------+------|------.                         .------|------+------+------+------+------+--------|
 * |        |  \   |  /   |  {   |  }   |  *   |      |                         |      |   ?  |  (   |  )   |  -   |  :   |        |
 * |--------+------+------+------+------+------|------|------.           .------|------|------+------+------+------+------+--------|
 * |        |  #   |  $   |  |   |  ~   |  `   |      |      |           |      |      |   +  |  %   |  "   |  '   |  ;   |        |
 * `--------+------+------+------+------+--------------------'           `------+-------------+------+------+------+------+--------'
 *                        .------.              ,-------------.         ,-------------.              .------.
 *                        |      |   .------.   |      |      |         |      |      |   .------.   |      |
 *                        '------'   |      |   |      |      |         |      |      |   |      |   '------'
 *                                   '------'   |      |      |         |      |      |   '------'
 *                                              `-------------'         `-------------'
 */

[_LOWER] = LAYOUT_gergo(
    KC_TRNS, KC_AT,   KC_UNDS, KC_LBRC, KC_RBRC, KC_CIRC,                                            KC_EXLM, KC_LT,   KC_GT,   KC_EQL,  KC_AMPR, KC_BSPC,
    KC_TRNS, KC_BSLS, KC_SLSH, KC_LCBR, KC_RCBR, KC_ASTR, KC_TRNS,                          KC_TRNS, KC_QUES, KC_LPRN, KC_RPRN, KC_MINS, KC_COLN, KC_TRNS,
    KC_TRNS, KC_HASH, KC_DLR,  KC_PIPE, KC_TILD, KC_GRV,  KC_TRNS, KC_TRNS,        KC_TRNS, KC_TRNS, KC_PLUS, KC_PERC, KC_DQUO, KC_QUOT, KC_SCLN, KC_TRNS,
                                        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
    ),
/* Keymap 2: Pad/Function layer
 *
 * ,-------------------------------------------.                         ,-------------------------------------------.
 * |        |   1  |  2   |  3   |  4   |  5   |                         |  6   |  7   |  8   |  9   |  0   |        |
 * |--------+------+------+------+------+------|------.           .------|------+------+------+------+------+--------|
 * |  F1    |  F2  | F3   | F4   | F5   | F6   | BTN1 |           |      | LEFT | DOWN |  UP  | RIGHT|VolDn | VolUp  |
 * |--------+------+------+------+------+------|------|           |------|------+------+------+------+------+--------|
 * |  F7    |  F8  | F9   | F10  | F11  | F12  | BTN2 |           |      | MLFT | MDWN | MUP  | MRGHT|Ply/Pa|  Skip  |
 * `--------+------+------+------+------+-------------'           `-------------+------+------+------+------+--------'
 *                        .------.   .------.                                 .------.   .-----.
 *                        |      |   |      |                                 |      |   |     |
 *                        '------'   '------'                                 `------.   '-----'
 *                                        ,-------.                     ,-------.
 *                                        |       |                     | PgUp  |
 *                                 ,------|-------|                     |-------|------.
 *                                 |      |       |                     |       |      |
 *                                 |      |       |                     |       |      |
 *                                 |      |       |                     |       |      |
 *                                 `--------------'                     `--------------'
 */
[_RAISE] = LAYOUT_gergo(
    KC_TRNS,  KC_ESC,  KC_7, KC_8, KC_9, KC_TRNS,                                          KC_HOME, KC_PGUP, KC_UP,   KC_PGDN, KC_BSPC, KC_DEL,
    KC_TRNS,  KC_TAB,  KC_4, KC_5, KC_6, KC_TRNS, KC_TRNS,                       KC_TRNS,  KC_END,  KC_LEFT, KC_DOWN, KC_RGHT, KC_ENT,  KC_TRNS,
    KC_TRNS,  KC_CAPS, KC_1, KC_2, KC_3, KC_0,    KC_TRNS, KC_TRNS,     KC_TRNS, KC_TRNS,  KC_UNDO, KC_CUT, KC_COPY,  KC_PSTE, KC_DEL,  KC_TRNS,
                              KC_TRNS,   KC_TRNS, KC_TRNS, KC_TRNS,     KC_TRNS, KC_TRNS,  KC_TRNS,     KC_TRNS
    ),
[_ADJUST] = LAYOUT_gergo(
    QK_BOOT, KC_1, 	  KC_2,    KC_3,    KC_4,    KC_5,                                             KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_DEL,
    EE_CLR,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,                         KC_TRNS,  KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, KC_VOLD, KC_VOLU,
    KC_TRNS, KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  KC_TRNS,     KC_TRNS, KC_TRNS,  KC_MS_L, KC_MS_D, KC_MS_U, KC_MS_R, KC_MPLY, KC_MNXT,
                                        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,         KC_TRNS,  KC_TRNS, KC_TRNS, KC_TRNS
    ),
[_UMLAUT] = LAYOUT_gergo(
    KC_TRNS, KC_TRNS, RALT(KC_Y), KC_TRNS, KC_TRNS, KC_TRNS,                                             KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
    KC_TRNS, KC_TRNS, KC_TRNS, RALT(KC_5), RALT(KC_Q), RALT(KC_P), KC_TRNS,                           KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, RALT(KC_S), KC_TRNS,
    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,         KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                                        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,         KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
    )
};

/* Keymap template
 *
 * ,-------------------------------------------.                         ,-------------------------------------------.
 * |        |      |      |      |      |      |                         |      |      |      |      |      |        |
 * |--------+------+------+------+------+------|------.           .------|------+------+------+------+------+--------|
 * |        |      |      |      |      |      |      |           |      |      |      |      |      |      |        |
 * |--------+------+------+------+------+------|------|           |------|------+------+------+------+------+--------|
 * |        |      |      |      |      |      |      |           |      |      |      |      |      |      |        |
 * `--------+------+------+------+------+-------------'           `-------------+------+------+------+------+--------'
 *                        .------.   .------.                                 .------.   .-----.
 *                        |      |   |      |                                 |      |   |     |
 *                        '------'   '------'                                 `------.   '-----'
 *                                        ,-------.       ,-------.
 *                                        |       |       |       |
 *                                 ,------|-------|       |-------|------.
 *                                 |      |       |       |       |      |
 *                                 |      |       |       |       |      |
 *                                 |      |       |       |       |      |
 *                                 `--------------'       `--------------'
[SYMB] = LAYOUT_gergo(
    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,                                             KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,                           KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,         KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                                        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,         KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
    )
 */


/*
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  switch (keycode) {
    case KC_A:
      if (record->event.pressed){
        if (get_mods() & MOD_BIT(KC_RALT)){
          register_code(KC_RALT);
          register_code(KC_Q);
          override = true;
        } else {
          register_code(KC_A);
          override = false;
        }
      } else {
        if(override){
            unregister_code(KC_Q);
            unregister_code(KC_RALT);
        } else {
            unregister_code(KC_A);
        }
        override = false;
      }
      return false;
    case KC_O:
      if (record->event.pressed){
        if (get_mods() & MOD_BIT(KC_RALT)){
          register_code(KC_RALT);
          register_code(KC_Y);
          override = true;
        } else {
          register_code(KC_O);
          override = false;
        }
       } else {
        if(override){
          unregister_code(KC_Y);
          unregister_code(KC_RALT);
        } else {
            unregister_code(KC_O);
        }
        override = false;
      }
      return false;
    case KC_U:
      if (record->event.pressed){
        if (get_mods() & MOD_BIT(KC_RALT)){
          register_code(KC_RALT);
          register_code(KC_P);
          override = true;
        } else {
          register_code(KC_U);
          override = false;
        }
       } else {
        if(override){
          unregister_code(KC_P);
          unregister_code(KC_RALT);
        } else {
            unregister_code(KC_U);
        }
        override = false;
      }
      return false;
    case KC_E:
      if (record->event.pressed){
        if (get_mods() & MOD_BIT(KC_RALT)){
          register_code(KC_RALT);
          register_code(KC_5);
          override = true;
        } else {
          register_code(KC_E);
          override = false;
        }
       } else {
        if(override){
          unregister_code(KC_5);
          unregister_code(KC_RALT);
        } else {
            unregister_code(KC_E);
        }
        override = false;
      }
      return false;
    default:
      return true;
  }
}
*/
