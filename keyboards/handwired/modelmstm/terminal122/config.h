/*
Copyright 2015 Jun Wako <wakojun@gmail.com>

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

/* USB Device descriptor parameter */
#define VENDOR_ID       0xFEED
#define PRODUCT_ID      0x6465
#define DEVICE_VER      0x0122
#define MANUFACTURER    marfrit
#define PRODUCT         modelmstm

/* key matrix size */
#define MATRIX_ROWS 8
#define MATRIX_COLS 20

// #define SD1_CTS_PIN B9

#define MATRIX_ROW_PINS { A7,  A6,  A5,  A4, A3, A2, A1, A0 }
//#define MATRIX_COL_PINS { C13, C14, C15, B9, B8, B7, B6, B5, B4, B3, A15, B0, B1, A10, B10, A8, B15, B14, B13, B12 }
#define MATRIX_COL_PINS { B12, B13, B14, B15, A8, B10, A10, B1, B0, A15, B3, B4, B5, B6, B7, B8, B9, C15, C14, C13 }
#define UNUSED_PINS

/* COL2ROW, ROW2COL*/
#define DIODE_DIRECTION ROW2COL

/* define if matrix has ghost */
#define MATRIX_HAS_GHOST

/* Set 0 if debouncing isn't needed */
#define DEBOUNCE    5

/* Mechanical locking support. Use KC_LCAP, KC_LNUM or KC_LSCR instead in keymap */
//#define LOCKING_SUPPORT_ENABLE
/* Locking resynchronize hack */
//#define LOCKING_RESYNC_ENABLE

/*
 * Feature disable options
 *  These options are also useful to firmware size reduction.
 */

/* disable debug print */
//#define NO_DEBUG

/* disable print */
//#define NO_PRINT

/* disable action features */
//#define NO_ACTION_LAYER
//#define NO_ACTION_TAPPING
//#define NO_ACTION_ONESHOT
//#define NO_ACTION_MACRO
//#define NO_ACTION_FUNCTION

#define EARLY_INIT_PERFORM_BOOTLOADER_JUMP TRUE
//#define STM32_HSECLK 8000000
