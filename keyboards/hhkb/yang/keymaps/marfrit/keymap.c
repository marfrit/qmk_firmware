/*  -*-  eval: (turn-on-orgtbl); -*-
 * default HHKB Layout
 *
 * Copyright 2022 Markus Fritsche <fritsche.markus@gmail.com>
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
#include QMK_KEYBOARD_H

enum unicode_names {
    KC_AU,
    KC_aU,
    KC_OU,
    KC_oU,
    KC_UU,
    KC_uU,
    KC_SS,
    KC_sS,
    KC_EUR
};

const uint32_t PROGMEM unicode_map[] = {
    [KC_AU]  = 0xC4,  // Ä
    [KC_aU]  = 0xE4,  // ä
    [KC_OU]  = 0xD6,
    [KC_oU]  = 0xF6,
    [KC_UU]  = 0xDC,
    [KC_uU]  = 0xFC,
    [KC_SS]  = 0x1E9E,
    [KC_sS]  = 0xDF,
    [KC_EUR] = 0x20AC
};

enum custom_layers {
    BASE,
    HHKB,
    LOWER,
    RAISE,
    ADJUST,
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    /* BASE Level: Default Layer
     |-------+---+---+---+---+---+---+---+---+---+---+-------+-----+-------+---|
     | Esc   | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 0 | -     | =   | \     | ` |
     |-------+---+---+---+---+---+---+---+---+---+---+-------+-----+-------+---|
     | Tab   | Q | W | E | R | T | Y | U | I | O | P | [     | ]   | Backs |   |
     |-------+---+---+---+---+---+---+---+---+---+---+-------+-----+-------+---|
     | Cont  | A | S | D | F | G | H | J | K | L | ; | '     | Ent |       |   |
     |-------+---+---+---+---+---+---+---+---+---+---+-------+-----+-------+---|
     | Shift | Z | X | C | V | B | N | M | , | . | / | Shift | Fn0 |       |   |
     |-------+---+---+---+---+---+---+---+---+---+---+-------+-----+-------+---|

            |------+------+-----------------------+------+------|
            | LAlt | LGUI | ******* Space ******* | RGUI | RAlt |
            |------+------+-----------------------+------+------|
    */

    [BASE] = LAYOUT_60_hhkb( //  default layer
        KC_ESC, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_MINS, KC_EQL, KC_BSLS, KC_GRV,
        KC_TAB, KC_K, KC_U, KC_Q, LT(ADJUST, KC_DOT), KC_J, KC_V, LT(ADJUST, KC_G), KC_C, KC_L, KC_F, KC_LBRC, KC_RBRC, KC_BSPC,
        KC_LCTL, KC_H, LALT_T(KC_I), LCTL_T(KC_E), LSFT_T(KC_A), KC_O, KC_D, RSFT_T(KC_T), RCTL_T(KC_R), LALT_T(KC_N), KC_S, KC_QUOT, KC_ENT,
        KC_LSFT, KC_X, RALT_T(KC_Y), KC_SCLN, LT(LOWER,KC_COMM), LT(RAISE,KC_SLSH), LT(RAISE,KC_B), LT(LOWER,KC_P), KC_W, RALT_T(KC_M), KC_Z, KC_RSFT, MO(HHKB),
        KC_LALT, KC_LGUI, /*        */ KC_SPC, KC_RGUI, KC_RALT),

    /* Layer HHKB: HHKB mode (HHKB Fn)
      |------+-----+-----+-----+----+----+----+----+-----+-----+-----+-----+-------+-------+-----|
      | Pwr  | F1  | F2  | F3  | F4 | F5 | F6 | F7 | F8  | F9  | F10 | F11 | F12   | Ins   | Del |
      |------+-----+-----+-----+----+----+----+----+-----+-----+-----+-----+-------+-------+-----|
      | Caps |     |     |     |    |    |    |    | Psc | Slk | Pus | Up  |       | Backs |     |
      |------+-----+-----+-----+----+----+----+----+-----+-----+-----+-----+-------+-------+-----|
      |      | VoD | VoU | Mut |    |    | *  | /  | Hom | PgU | Lef | Rig | Enter |       |     |
      |------+-----+-----+-----+----+----+----+----+-----+-----+-----+-----+-------+-------+-----|
      |      |     |     |     |    |    | +  | -  | End | PgD | Dow |     |       |       |     |
      |------+-----+-----+-----+----+----+----+----+-----+-----+-----+-----+-------+-------+-----|

                 |------+------+----------------------+------+------+
                 | **** | **** | ******************** | **** | **** |
                 |------+------+----------------------+------+------+

     */

    [HHKB] = LAYOUT_60_hhkb(
        KC_PWR, KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_F11, KC_F12, KC_INS, KC_DEL,
        KC_CAPS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_PSCR, KC_SLCK, KC_PAUS, KC_UP, KC_TRNS, KC_BSPC,
        KC_TRNS, KC_VOLD, KC_VOLU, KC_MUTE, KC_TRNS, KC_TRNS, KC_PAST, KC_PSLS, KC_HOME, KC_PGUP, KC_LEFT, KC_RGHT, KC_PENT,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, RESET,   KC_PPLS, KC_PMNS, KC_END, KC_PGDN, KC_DOWN, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS),

    [LOWER] = LAYOUT_60_hhkb(
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_AT, KC_UNDS, KC_LBRC, KC_RBRC, KC_CIRC, KC_EXLM, KC_LT, KC_GT, KC_EQL, KC_AMPR, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_BSLS, KC_SLSH, KC_LCBR, KC_RCBR, KC_ASTR, KC_QUES, KC_LPRN, KC_RPRN, KC_MINS, KC_COLN, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_HASH, KC_DLR, KC_PIPE, KC_TILD, KC_GRV, KC_PLUS, KC_PERC, KC_DQUO, KC_QUOT, KC_SCLN, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS),

    [RAISE] = LAYOUT_60_hhkb(
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_ESC, KC_7, KC_8, KC_9, KC_TRNS, KC_HOME, KC_PGUP, KC_UP, KC_PGDN, KC_BSPC, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TAB, KC_4, KC_5, KC_6, KC_TRNS, KC_END, KC_LEFT, KC_DOWN, KC_RGHT, KC_ENT, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_CAPS, KC_1, KC_2, KC_3, KC_0, KC_UNDO, KC_CUT, KC_COPY, KC_PSTE, KC_DEL, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS),

    [ADJUST] = LAYOUT_60_hhkb(
        KC_TRNS, UC_M_LN, UC_M_MA, UC_M_WI, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, XP(KC_uU, KC_UU), KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, X(KC_EUR), XP(KC_aU, KC_AU), XP(KC_oU, KC_OU), KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, XP(KC_sS, KC_SS), KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS),

    };
