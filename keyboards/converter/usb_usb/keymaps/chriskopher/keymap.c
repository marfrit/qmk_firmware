/* Copyright 2020 Christopher Ko <chriskopher@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "shared_enum.h"

#include QMK_KEYBOARD_H

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /* Modified Qwerty for chriskopher: base default layer
     * ,---.   ,---------------. ,---------------. ,---------------. ,-----------.
     * |Esc|   |F1 |F2 |F3 |F4 | |F5 |F6 |F7 |F8 | |F9 |F10|F11|F12| |PrS|ScL|Pau|
     * `---'   `---------------' `---------------' `---------------' `-----------'
     * ,-----------------------------------------------------------. ,-----------. ,---------------.
     * |  `|  1|  2|  3|  4|  5|  6|  7|  8|  9|  0|  -|  =|   \   | |Ins|Hom|PgU| |NmL|  /|  *|  -|
     * |-----------------------------------------------------------| |-----------| |---------------|
     * |Tab  |  Q|  W|  E|  R|  T|  Y|  U|  I|  O|  P|  [|  ]|  Bsp| |Del|End|PgD| |  7|  8|  9|   |
     * |-----------------------------------------------------------| `-----------' |-----------|   |
     * |Ctl/Esc|  A|  S|  D|  F|  G|  H|  J|  K| L|;/:| ' |  Return|               |  4|  5|  6|  +|
     * |-----------------------------------------------------------|     ,---.     |---------------|
     * |Shift/( |Dev/Z| X|  C|  V|  B|  N|  M|  ,|  .|  /|  Shift/)|     | ↑ |     |  1|  2|  3|   |
     * |-----------------------------------------------------------| ,-----------. |-----------|   |
     * |Meh/CapsL|Gui|Alt|     Space       |Alt|Adjust/Gui|App| Ctl| | ← | ↓ | → | |      0|  .|Ent|
     * `-----------------------------------------------------------' `-----------' `---------------'
    */
    [_CKO] = LAYOUT_ansi(
        KC_ESC,                           KC_F1,   KC_F2,   KC_F3,   KC_F4,  KC_F5,  KC_F6,  KC_F7,   KC_F8,   KC_F9,        KC_F10,   KC_F11,    KC_F12,              KC_PSCR, KC_SLCK, KC_PAUS,
        KC_GRV,           KC_1,           KC_2,    KC_3,    KC_4,    KC_5,   KC_6,   KC_7,   KC_8,    KC_9,    KC_0,         KC_MINS,  KC_EQL,    KC_BSLS,    KC_INS,  KC_HOME, KC_PGUP,            KC_NLCK, KC_PSLS, KC_PAST, KC_PMNS,
        KC_TAB,           KC_Q,           KC_W,    KC_E,    KC_R,    KC_T,   KC_Y,   KC_U,   KC_I,    KC_O,    KC_P,         KC_LBRC,  KC_RBRC,   KC_BSPC,    KC_DEL,  KC_END,  KC_PGDN,            KC_P7,   KC_P8,   KC_P9,   KC_PPLS,
        LCTL_T(KC_ESC),   KC_A,           KC_S,    KC_D,    KC_F,    KC_G,   KC_H,   KC_J,   KC_K,    KC_L,    TD(SCLN_CLN), KC_QUOT,             KC_ENT,                                           KC_P4,   KC_P5,   KC_P6,
        TD(ESPC_L),       LT(_DEV,KC_Z),  KC_X,    KC_C,    KC_V,    KC_B,   KC_N,   KC_M,   KC_COMM, KC_DOT,  KC_SLSH,                           TD(ESPC_R),          KC_UP,                       KC_P1,   KC_P2,   KC_P3,   KC_PENT,
        MEH_T(KC_CAPS), KC_LGUI, KC_LALT,                   KC_SPC,                                   KC_RALT, LT(_ADJUST,KC_RGUI),      KC_APP,  KC_RCTL,    KC_LEFT, KC_DOWN, KC_RGHT,            KC_P0,            KC_PDOT
    ),

    /* Regular Qwerty: default layer
     * ,---.   ,---------------. ,---------------. ,---------------. ,-----------.
     * |Esc|   |F1 |F2 |F3 |F4 | |F5 |F6 |F7 |F8 | |F9 |F10|F11|F12| |PrS|ScL|Pau|
     * `---'   `---------------' `---------------' `---------------' `-----------'
     * ,-----------------------------------------------------------. ,-----------. ,---------------.
     * |  `|  1|  2|  3|  4|  5|  6|  7|  8|  9|  0|  -|  =|    Bsp| |Ins|Hom|PgU| |NmL|  /|  *|  -|
     * |-----------------------------------------------------------| |-----------| |---------------|
     * |Tab  |  Q|  W|  E|  R|  T|  Y|  U|  I|  O|  P|  [|  ]|  \  | |Del|End|PgD| |  7|  8|  9|   |
     * |-----------------------------------------------------------| `-----------' |-----------|   |
     * |CapsL |  A|  S|  D|  F|  G|  H|  J|  K|  L|  ;|  '|  Return|               |  4|  5|  6|  +|
     * |-----------------------------------------------------------|     ,---.     |---------------|
     * |Shift   |  Z|  X|  C|  V|  B|  N|  M|  ,|  ,|  /|Shift     |     | ↑ |     |  1|  2|  3|   |
     * |-----------------------------------------------------------| ,-----------. |-----------|   |
     * |Ctl|Gui|Alt|           Space        |Alt|Adjust/Gui|App|Ctl| | ← | ↓ | → | |      0|  .|Ent|
     * `-----------------------------------------------------------' `-----------' `---------------'
    */
    [_QWERTY] = LAYOUT_ansi(
        KC_ESC,           KC_F1,   KC_F2,   KC_F3,   KC_F4,  KC_F5,  KC_F6,  KC_F7,   KC_F8,   KC_F9,   KC_F10,    KC_F11,   KC_F12,             KC_PSCR, KC_SLCK, KC_PAUS,
        KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,   KC_6,   KC_7,   KC_8,    KC_9,    KC_0,    KC_MINS,   KC_EQL,   KC_BSPC,   KC_INS,  KC_HOME, KC_PGUP,    KC_NLCK, KC_PSLS, KC_PAST, KC_PMNS,
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,   KC_Y,   KC_U,   KC_I,    KC_O,    KC_P,    KC_LBRC,   KC_RBRC,  KC_BSLS,   KC_DEL,  KC_END,  KC_PGDN,    KC_P7,   KC_P8,   KC_P9,   KC_PPLS,
        KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,   KC_H,   KC_J,   KC_K,    KC_L,    KC_SCLN, KC_QUOT,             KC_ENT,                                  KC_P4,   KC_P5,   KC_P6,
        KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,   KC_N,   KC_M,   KC_COMM, KC_DOT,  KC_SLSH,                      KC_RSFT,            KC_UP,               KC_P1,   KC_P2,   KC_P3,   KC_PENT,
        KC_LCTL, KC_LGUI, KC_LALT,                   KC_SPC,                         KC_RALT,  LT(_ADJUST,KC_RGUI), KC_APP,  KC_RCTL,   KC_LEFT, KC_DOWN, KC_RGHT,    KC_P0,            KC_PDOT
    ),

    /* SuperDuper
     * ,---.   ,---------------. ,---------------. ,---------------. ,-----------.
     * |   |   |   |   |   |   | |   |   |   |   | |   |   |   |   | |   |   |   |
     * `---'   `---------------' `---------------' `---------------' `-----------'
     * ,-----------------------------------------------------------. ,-----------. ,---------------.
     * |   |   |   |   |   |   |   |   |   |   |   |   |   |       | |   |   |   | |   |   |   |   |
     * |-----------------------------------------------------------| |-----------| |---------------|
     * |     |   |   |   |   |   |   |1T |T← |T→ |9T |   |    |    | |   |   |   | |   |   |   |   |
     * |-----------------------------------------------------------| `-----------' |-----------|   |
     * |      |Alt|[SuperDuper]|Bksp|Ctl| ← | ↓ | ↑ | → |Del|   |  |               |   |   |   |   |
     * |-----------------------------------------------------------|     ,---.     |---------------|
     * |        |   |   |   |   |   |   |   |   |   |ToggleSD|     |     |   |     |   |   |   |   |
     * |-----------------------------------------------------------| ,-----------. |-----------|   |
     * |   |   |   |             Shift             |   |   |   |   | |   |   |   | |       |   |   |
     * `-----------------------------------------------------------' `-----------' `---------------'
    */
    [_SUPERDUPER] = LAYOUT_ansi(
        ______,           ______,  ______,  ______,  ______,  ______,  ______,  ______,       ______,     ______,    ______,  ______,  ______,               ______,  ______,  ______,
        ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,       ______,     ______,    ______,  ______,           ______,      ______,  ______,  ______,     ______,  ______,  ______,  ______,
        ______,  ______,  ______,  ______,  ______,  ______,  ______,  C(KC_1), C(S(KC_TAB)), C(KC_TAB),  C(KC_9),   ______,  ______,           ______,      ______,  ______,  ______,     ______,  ______,  ______,  ______,
        ______,  KC_LALT, ______,  ______,  KC_BSPC, KC_LCTL, KC_LEFT, KC_DOWN, KC_UP,        KC_RIGHT,   KC_DEL,    ______,                    ______,                                    ______,  ______,  ______,
        ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,       ______,     TG(_SUPERDUPER),                      ______,               ______,              ______,  ______,  ______,  ______,
        ______,  ______,  ______,                    KC_LSFT,                                                        ______,  ______,  ______,  ______,      ______,  ______,  ______,     ______,           ______
    ),

    /* Dev
     * ,---.   ,---------------. ,---------------. ,---------------. ,-----------.
     * |   |   |   |   |   |   | |   |   |   |   | |   |   |   |   | |   |   |   |
     * `---'   `---------------' `---------------' `---------------' `-----------'
     * ,-----------------------------------------------------------. ,-----------. ,---------------.
     * |   |   |   |   |   |   |   |   |   |   |   |   |   |       | |   |   |   | |   |   |   |   |
     * |-----------------------------------------------------------| |-----------| |---------------|
     * |     |   |   |   |   |   |   | - | + | ( | ) |   |   |     | |   |   |   | |   |   |   |   |
     * |-----------------------------------------------------------| `-----------' |-----------|   |
     * |      |   |   |   |   |   | _ | [ | ] | { | } |   |        |               |   |   |   |   |
     * |-----------------------------------------------------------|     ,---.     |---------------|
     * |        |   |   |   |   |   | = | | | < | > | ? |          |     |   |     |   |   |   |   |
     * |-----------------------------------------------------------| ,-----------. |-----------|   |
     * |   |   |   |                               |   |   |   |   | |   |   |   | |       |   |   |
     * `-----------------------------------------------------------' `-----------' `---------------'
    */
    [_DEV] = LAYOUT_ansi(
        ______,           ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,                            ______,  ______,  ______,
        ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,                        ______,      ______,  ______,  ______,     ______,  ______,  ______,  ______,
        ______,  ______,  ______,  ______,  ______,  ______,  ______,  KC_MINS, S(KC_EQL), S(KC_9), S(KC_0),  ______,  ______,                     ______,      ______,  ______,  ______,     ______,  ______,  ______,  ______,
        ______,  ______,  ______,  ______,  ______,  ______,  S(KC_MINS), KC_LBRC, KC_RBRC, S(KC_LBRC), S(KC_RBRC),  ______,                       ______,                                    ______,  ______,  ______,
        ______,  ______,  ______,  ______,  ______,  ______,  KC_EQL, S(KC_BSLASH), S(KC_COMM), S(KC_DOT), S(KC_SLSH),                             ______,               ______,              ______,  ______,  ______,  ______,
        ______,  ______,  ______,                    ______,                                                            ______,  ______,  ______,  ______,      ______,  ______,  ______,     ______,           ______
    ),

    /* Adjust
     * ,---.   ,---------------. ,---------------. ,---------------. ,-----------.
     * |   |   |   |   |   |   | |   |   |   |   | |   |   |   |   | |   |   |   |
     * `---'   `---------------' `---------------' `---------------' `-----------'
     * ,-----------------------------------------------------------. ,--------------. ,---------------.
     * |   |   |   |   |   |   |   |   |   |   |   |   |   |       | |Play|Next|VolU| |   |   |   |   |
     * |-----------------------------------------------------------| |--------------| |---------------|
     * |     |QWERTY|   |   |   |   |   |   |   |   |   |   |  |   | |Stop|Prev|VolD| |   |   |   |   |
     * |-----------------------------------------------------------| `--------------' |-----------|   |
     * |      |   |   |   |   |   |   |   |   |   |   |   |        |                  |   |   |   |   |
     * |-----------------------------------------------------------|     ,---.        |---------------|
     * |        |   |   |CKO|   |   |  |Play|Mute|VolD|VolU| SD|   |     |   |        |   |   |   |   |
     * |-----------------------------------------------------------| ,-----------.    |-----------|   |
     * |   |   |   |                               |   |   |   |   | |   |   |   |    |       |   |   |
     * `-----------------------------------------------------------' `-----------'    `---------------'
    */
    [_ADJUST] = LAYOUT_ansi(
        ______,                ______,  ______,    ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,               ______,  ______,  ______,
        ______,  ______,       ______,  ______,    ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,           ______,      KC_MPLY, KC_MNXT, KC_VOLU,    ______,  ______,  ______,  ______,
        ______,  DF(_QWERTY),  ______,  ______,    ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,           ______,      KC_MSTP, KC_MPRV, KC_VOLD,    ______,  ______,  ______,  ______,
        ______,  ______,       ______,  ______,    ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,                    ______,                                    ______,  ______,  ______,
        ______,  ______,       ______,  DF(_CKO),  ______,  ______,  KC_MPLY,  KC_MUTE, KC_VOLD, KC_VOLU, TG(_SUPERDUPER),                   ______,               ______,              ______,  ______,  ______,  ______,
        ______,  ______,       ______,                      ______,                                               ______,  ______,  ______,  ______,      ______,  ______,  ______,     ______,           ______
    )

/* Empty layout for future reference
  * ,---.   ,---------------. ,---------------. ,---------------. ,-----------.
  * |   |   |   |   |   |   | |   |   |   |   | |   |   |   |   | |   |   |   |
  * `---'   `---------------' `---------------' `---------------' `-----------'
  * ,-----------------------------------------------------------. ,-----------. ,---------------.
  * |   |   |   |   |   |   |   |   |   |   |   |   |   |       | |   |   |   | |   |   |   |   |
  * |-----------------------------------------------------------| |-----------| |---------------|
  * |     |   |   |   |   |   |   |   |   |   |   |   |   |     | |   |   |   | |   |   |   |   |
  * |-----------------------------------------------------------| `-----------' |-----------|   |
  * |      |   |   |   |   |   |   |   |   |   |   |   |        |               |   |   |   |   |
  * |-----------------------------------------------------------|     ,---.     |---------------|
  * |        |   |   |   |   |   |   |   |   |   |   |          |     |   |     |   |   |   |   |
  * |-----------------------------------------------------------| ,-----------. |-----------|   |
  * |   |   |   |                               |   |   |   |   | |   |   |   | |       |   |   |
  * `-----------------------------------------------------------' `-----------' `---------------'
 */
 /*
  *  [_EMPTY] = LAYOUT_ansi(
  *      ______,           ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,               ______,  ______,  ______,
  *      ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,           ______,      ______,  ______,  ______,     ______,  ______,  ______,  ______,
  *      ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,           ______,      ______,  ______,  ______,     ______,  ______,  ______,  ______,
  *      ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,                    ______,                                    ______,  ______,  ______,
  *      ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,  ______,                             ______,               ______,              ______,  ______,  ______,  ______,
  *      ______,  ______,  ______,                    ______,                                               ______,  ______,  ______,  ______,      ______,  ______,  ______,     ______,           ______
  *  ),
 */
};
// clang-format on

// Configure ignore mod tap interrupt per key
bool get_ignore_mod_tap_interrupt(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        // I don't like how mod tap interrupt feels with these keys specifically when I'm typing
        case LCTL_T(KC_ESC):
            return false;
        default:
            return true;
    }
}
