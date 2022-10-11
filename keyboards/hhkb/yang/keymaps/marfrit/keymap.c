/* marfrit HHKB Layout
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
#include "marfrit.h"

#define LAYOUT_60_hhkb_wrapper(...) LAYOUT_60_hhkb(__VA_ARGS__)

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

    [_HIEAO] = LAYOUT_60_hhkb_wrapper( //  default layer
        KC_ESC,  ___NUMBER_L1____, ___NUMBER_R1____, KC_MINS, KC_EQL, KC_BSLS, KC_GRV,
        KC_TAB,  ____HIEAO_L1____, ____HIEAO_R1____, KC_LBRC, KC_RBRC, KC_BSPC,
        KC_LCTL, ____HIEAO_L2____, ____HIEAO_R2____, KC_QUOT, KC_ENT,
        KC_LSFT, ____HIEAO_L3____, ____HIEAO_R3____, KC_RSFT, MO(_HHKB),
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

    [_HHKB] = LAYOUT_60_hhkb_wrapper(
        KC_PWR, KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_F11, KC_F12, KC_INS, KC_DEL,
        KC_CAPS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_PSCR, KC_SLCK, KC_PAUS, KC_UP, KC_TRNS, KC_BSPC,
        KC_TRNS, KC_VOLD, KC_VOLU, KC_MUTE, KC_TRNS, KC_TRNS, KC_PAST, KC_PSLS, KC_HOME, KC_PGUP, KC_LEFT, KC_RGHT, KC_PENT,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, RESET,   KC_PPLS, KC_PMNS, KC_END, KC_PGDN, KC_DOWN, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS),

    [_SYMBOLS] = LAYOUT_60_hhkb_wrapper(
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, ___SYMBOL_L1____, ___SYMBOL_R1____, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, ___SYMBOL_L2____, ___SYMBOL_R2____, KC_TRNS, KC_TRNS,
        KC_TRNS, ___SYMBOL_L3____, ___SYMBOL_R3____, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS),

    [_NAVIGATION] = LAYOUT_60_hhkb_wrapper(
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, _NAVIGATION_L1__, _NAVIGATION_R1__, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, _NAVIGATION_L2__, _NAVIGATION_R2__, KC_TRNS, KC_TRNS,
        KC_TRNS, _NAVIGATION_L3__, _NAVIGATION_R3__, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS),

    [_DIACRITICS] = LAYOUT_60_hhkb_wrapper(
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, _DIACRITICS_L1__, _DIACRITICS_R1__, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, _DIACRITICS_L2__, _DIACRITICS_R2__, KC_TRNS, KC_TRNS,
        KC_TRNS, _DIACRITICS_L3__, _DIACRITICS_R3__, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS),

    [_FUNCTION] = LAYOUT_60_hhkb_wrapper(
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, __FUNCTION_L1___, __FUNCTION_R1___, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, __FUNCTION_L2___, __FUNCTION_R2___, KC_TRNS, KC_TRNS,
        KC_TRNS, __FUNCTION_L3___, __FUNCTION_R3___, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS),

    };
