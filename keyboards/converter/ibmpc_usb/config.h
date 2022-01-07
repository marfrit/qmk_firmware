/*
Copyright 2012 Jun Wako <wakojun@gmail.com>
Copyright 2016 Priyadi Iman Nurcahyo <priyadi@priyadi.net>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#define VENDOR_ID       0xFEED
#define PRODUCT_ID      0x6535
#define DEVICE_VER      0x0101
#define MANUFACTURER    QMK
#define PRODUCT         IBM Terminal Keyboard

/* matrix size */
#define MATRIX_ROWS 8  // keycode bit: 3-0
#define MATRIX_COLS 16   // keycode bit: 6-4

#define DIODE_DIRECTION COL2ROW

/* legacy keymap support */
#define USE_LEGACY_KEYMAP

/* key combination for command */
#define IS_COMMAND() ( \
    get_mods() == (MOD_BIT(KC_LSHIFT) | MOD_BIT(KC_RSHIFT) | MOD_BIT(KC_RALT) | MOD_BIT(KC_RCTL)) \
)

/*
 * PS/2 Interrupt configuration
 */
#ifdef PS2_USE_INT
/* uses INT1 for clock line(ATMega32U4) */
#define PS2_CLOCK_PIN   D1
#define PS2_DATA_PIN    D0

#define XT_RST_PIN1     B6
#define XT_RST_PIN2     B7

#define PS2_INT_INIT()  do {    \
    EICRA |= ((1<<ISC11) |      \
              (0<<ISC10));      \
} while (0)
#define PS2_INT_ON()  do {      \
    EIMSK |= (1<<INT1);         \
} while (0)
#define PS2_INT_OFF() do {      \
    EIMSK &= ~(1<<INT1);        \
} while (0)
#define PS2_INT_VECT    INT1_vect
#endif
