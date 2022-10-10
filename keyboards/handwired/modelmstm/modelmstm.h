/* Copyright 2019 iw0rm3r
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
#pragma once

#include "quantum.h"

/* This a shortcut to help you visually see your layout.
 * The first section contains "names" for physical keys of the keyboard
 * and defines their position on the board.
 * The second section defines position of the keys on the switch matrix
 * (where COLUMNS and ROWS crosses). */

#define XXX KC_NO

#define LAYOUT_terminal122( \
                K09, K19, K1A, K29, K39, K3A, K49, K59, K5A, K69, K79, K7A, \
                K0A, K0B, K1B, K2A, K2B, K3B, K4A, K4B, K5B, K6A, K6B, K7B, \
    K31, K32,   K34, K24, K25, K26, K27, K37, K38, K28, K2C, K2D, K2E, K3E, K3C, K3F,   K3G, K3H, K2G,   K2F, K2H, K2I, K20, \
    K21, K41,   K42, K44, K45, K46, K47, K57, K58, K48, K4C, K4D, K4E, K5E, K5C,        K1G, K5G, K4G,   K4F, K4H, K4I, K40, \
    K51, K52,   K62, K14, K15, K16, K17, K07, K08, K18, K1C, K1D, K1E, K0E, K6E, K6F,        K0G,        K1F, K1H, K1I, K10, \
    K11, K12,   K73, K74, K64, K65, K66, K67, K77, K78, K68, K6C, K6D, K7E,      K63,   K60, K0J, K1J,   K61, K6H, K6I, K7J, \
    K02, K01,   K00,    K70,                K71,             K03,      K72,                  K0F,        K7H,      K7I \
) { \
    { K00, K01, K02, K03, XXX, XXX, XXX, K07, K08, K09, K0A, K0B, XXX, XXX, K0E, K0F, K0G, XXX, XXX, K0J }, \
    { K10, K11, K12, XXX, K14, K15, K16, K17, K18, K19, K1A, K1B, K1C, K1D, K1E, K1F, K1G, K1H, K1I, K1J }, \
    { K20, K21, XXX, XXX, K24, K25, K26, K27, K28, K29, K2A, K2B, K2C, K2D, K2E, K2F, K2G, K2H, K2I, XXX }, \
    { XXX, K31, K32, XXX, K34, XXX, XXX, K37, K38, K39, K3A, K3B, K3C, XXX, K3E, K3F, K3G, K3H, XXX, XXX }, \
    { K40, K41, K42, XXX, K44, K45, K46, K47, K48, K49, K4A, K4B, K4C, K4D, K4E, K4F, K4G, K4H, K4I, XXX }, \
    { XXX, K51, K52, XXX, XXX, XXX, XXX, K57, K58, K59, K5A, K5B, K5C, XXX, K5E, XXX, K5G, XXX, XXX, XXX }, \
    { K60, K61, K62, K63, K64, K65, K66, K67, K68, K69, K6A, K6B, K6C, K6D, K6E, K6F, XXX, K6H, K6I, XXX }, \
    { K70, K71, K72, K73, K74, XXX, XXX, K77, K78, K79, K7A, K7B, XXX, XXX, K7E, XXX, XXX, K7H, K7I, K7J } \
}

#define LAYOUT_terminal102( \
    K5A,      K5B, K5C, K5D, K5E, K5F, K5G, K5H, K5I, K5J, K5K, K5L, K5M,   K5N, K5O, K5P, \
    \
    K4A, K4B, K4C, K4D, K4E, K4F, K4G, K4H, K4I, K4J, K4K, K4L, K4M, K4N,   K4O, K4P, K4Q,   K4R, K4S, K4T, K4U, \
    K3A, K3B, K3C, K3D, K3E, K3F, K3G, K3H, K3I, K3J, K3K, K3L, K3M, K3N,   K3O, K3P, K3Q,   K3R, K3S, K3T, K3U, \
    K2A, K2B, K2C, K2D, K2E, K2F, K2G, K2H, K2I, K2J, K2K, K2L,      K2N,                    K2O, K2P, K2Q, K2R, \
    K1A,      K1C, K1D, K1E, K1F, K1G, K1H, K1I, K1J, K1K, K1L,      K1M,        K1N,        K1O, K1P, K1Q, K1R, \
    K0A,      K0B,                K0C,                     K0D,      K0E,   K0F, K0G, K0H,   K0I,      K0J \
) \
{ \
/* 00 */ { KC_NO, KC_NO, K5A,   KC_NO, K5E,   K2F, K5F,   K2G, K5G,   KC_NO, K2L,   KC_NO, K0I, K0J, K1N,   K0B   }, \
/* 01 */ { KC_NO, K1A,   K3A,   K2A,   K5D,   K3F, K4N,   K3G, K3M,   K5H,   K3L,   K2O,   K2P, K2Q, K2R,   KC_NO }, \
/* 02 */ { K0A,   KC_NO, K4A,   K5B,   K5C,   K4F, K5J,   K4G, K4M,   K5I,   K4L,   K3O,   K4O, K4Q, K4P,   KC_NO }, \
/* 03 */ { KC_NO, KC_NO, K4B,   K4C,   K4D,   K4E, K5K,   K4H, K4I,   K4J,   K4K,   K5L,   K5M, K3Q, K3P,   K5N   }, \
/* 04 */ { KC_NO, KC_NO, K3B,   K3C,   K3D,   K3E, KC_NO, K3H, K3I,   K3J,   K3K,   K3R,   K3S, K3T, K3U,   K5O   }, \
/* 05 */ { KC_NO, KC_NO, K2B,   K2C,   K2D,   K2E, K3N,   K2H, K2I,   K2J,   K2K,   K1O,   K1P, K1Q, K1R,   KC_NO }, \
/* 06 */ { K0E,   K1M,   K1C,   K1D,   K1E,   K1F, K2N,   K1I, K1J,   K1K,   KC_NO, K4R,   K4S, K4T, K5P,   KC_NO }, \
/* 07 */ { KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, K1G, K0C,   K1H, KC_NO, KC_NO, K1L,   K0G,   K0H, K4U, K0F,   K0D   }, \
}
/*         0      1      2      3      4      5    6      7    8      9      A      B      C    D    E      F       */

#define LAYOUT_iso102( \
    K5A,      K5B, K5C, K5D, K5E, K5F, K5G, K5H, K5I, K5J, K5K, K5L, K5M,   K5N, K5O, K5P, \
    \
    K4A, K4B, K4C, K4D, K4E, K4F, K4G, K4H, K4I, K4J, K4K, K4L, K4M, K4N,   K4O, K4P, K4Q,   K4R, K4S, K4T, K4U, \
    K3A, K3B, K3C, K3D, K3E, K3F, K3G, K3H, K3I, K3J, K3K, K3L, K3M,        K3O, K3P, K3Q,   K3R, K3S, K3T, K3U, \
    K2A, K2B, K2C, K2D, K2E, K2F, K2G, K2H, K2I, K2J, K2K, K2L, K2M, K2N,                    K2O, K2P, K2Q,      \
    K1A, K1B, K1C, K1D, K1E, K1F, K1G, K1H, K1I, K1J, K1K, K1L,      K1M,        K1N,        K1O, K1P, K1Q, K1R, \
    K0A,      K0B,                K0C,                     K0D,      K0E,   K0F, K0G, K0H,   K0I,      K0J \
) \
{ \
/* 00 */ { KC_NO, KC_NO, K5A,   K1B,   K5E,   K2F, K5F,   K2G, K5G,   KC_NO, K2L,   KC_NO, K0I, K0J, K1N,   K0B   }, \
/* 01 */ { KC_NO, K1A,   K3A,   K2A,   K5D,   K3F, K4N,   K3G, K3M,   K5H,   K3L,   K2O,   K2P, K2Q, KC_NO, KC_NO }, \
/* 02 */ { K0A,   KC_NO, K4A,   K5B,   K5C,   K4F, K5J,   K4G, K4M,   K5I,   K4L,   K3O,   K4O, K4Q, K4P,   KC_NO }, \
/* 03 */ { KC_NO, KC_NO, K4B,   K4C,   K4D,   K4E, K5K,   K4H, K4I,   K4J,   K4K,   K5L,   K5M, K3Q, K3P,   K5N   }, \
/* 04 */ { KC_NO, KC_NO, K3B,   K3C,   K3D,   K3E, KC_NO, K3H, K3I,   K3J,   K3K,   K3R,   K3S, K3T, K3U,   K5O   }, \
/* 05 */ { KC_NO, KC_NO, K2B,   K2C,   K2D,   K2E, KC_NO, K2H, K2I,   K2J,   K2K,   K1O,   K1P, K1Q, K1R,   KC_NO }, \
/* 06 */ { K0E,   K1M,   K1C,   K1D,   K1E,   K1F, K2N,   K1I, K1J,   K1K,   K2M,   K4R,   K4S, K4T, K5P,   KC_NO }, \
/* 07 */ { KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, K1G, K0C,   K1H, KC_NO, KC_NO, K1L,   K0G,   K0H, K4U, K0F,   K0D   }, \
}
/*         0      1      2      3      4      5    6      7    8      9      A      B      C    D    E      F       */
