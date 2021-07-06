/*
Copyright 2019 Markus Fritsche <fritsche.markus@gmail.com>

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

/*
   these matrices map the scan codes as produced by matrix_scan to
   the artificial layout as defined by ibmpc_usb.h LAYOUT.
   Each codeset (1, 2, and 3) is mapped to 127 keys of LAYOUT.

   There are some quirks with prefixed scancodes as shown in
   process_cs1, process_cs2, and process_cs3.

   This is done to use XT- (codeset 1), AT- (codeset 2),
   and terminal-keyboards without recompiling the keymap and
   to potentially support VIA in the future.
*/

#include "ibmpc_usb.h"
const uint8_t map_cs1[MATRIX_ROWS][MATRIX_COLS] = {
    { XXX , 0x1a, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21 }, /* 00-07 */
    { 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x29, 0x33 }, /* 08-0F */
    { 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b }, /* 10-17 */
    { 0x3c, 0x3d, 0x3e, 0x3f, 0x57, 0x76, 0x4b, 0x4c }, /* 18-1F */
    { 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53, 0x54 }, /* 20-27 */
    { 0x55, 0x1b, 0x5f, 0x56, 0x61, 0x62, 0x63, 0x64 }, /* 28-2F */
    { 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6c, 0x2f }, /* 30-37 */
    { 0x77, 0x78, 0x4a, 0x0d, 0x0e, 0x0f, 0x10, 0x11 }, /* 38-3F */
    { 0x12, 0x13, 0x14, 0x15, 0x16, 0x2d, 0x31, 0x44 }, /* 40-47 */
    { 0x45, 0x46, 0x30, 0x59, 0x5a, 0x5b, 0x47, 0x70 }, /* 48-4F */
    { 0x71, 0x72, 0x7d, 0x7e, 0x5e, 0x48, 0x60, 0x17 }, /* 50-57 */
    { 0x18, XXX , 0x75, 0x74, 0x5d, XXX , XXX , XXX  }, /* 58-5F */
    { 0x58, 0x6d, 0x7b, 0x6f, 0x01, 0x02, 0x03, 0x04 }, /* 60-67 */
    { 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x73 }, /* 68-6F */
    { XXX , 0x2a, 0x41, XXX , 0x2b, 0x42, 0x0c, 0x2c }, /* 70-77 */
    { 0x43, XXX , 0x7a, XXX , 0x79, XXX,  0x5c, 0x2e }, /* 78-7F */
};

const uint8_t map_cs2[MATRIX_ROWS][MATRIX_COLS] = {
    { 0x48, 0x15, 0x13, 0x11, 0x0F, 0x0D, 0x0E, 0x18 }, /* 00-07 */
    { 0x01, 0x16, 0x14, 0x12, 0x10, 0x33, 0x1B, 0x79 }, /* 08-0F */
    { 0x02, 0x77, 0x5F, XXX , 0x76, 0x34, 0x1C, 0x7A }, /* 10-17 */
    { 0x03, 0x75, 0x61, 0x4c, 0x4b, 0x35, 0x1D, 0x74 }, /* 18-1F */
    { 0x04, 0x63, 0x62, 0x4d, 0x36, 0x1F, 0x1E, 0x42 }, /* 20-27 */
    { 0x05, 0x78, 0x64, 0x4e, 0x38, 0x37, 0x20, 0x2B }, /* 28-2F */
    { 0x06, 0x66, 0x65, 0x50, 0x4f, 0x39, 0x21, 0x41 }, /* 30-37 */
    { 0x07, 0x2A, 0x67, 0x51, 0x3A, 0x22, 0x23, 0x7b }, /* 38-3F */
    { 0x08, 0x68, 0x52, 0x3B, 0x3c, 0x25, 0x24, 0x6f }, /* 40-47 */
    { 0x09, 0x69, 0x6a, 0x53, 0x54, 0x3d, 0x26, 0x58 }, /* 48-4F */
    { 0x0A, XXX , 0x55, 0x6d, 0x3E, 0x27, 0x43, 0x0B }, /* 50-57 */
    { 0x4a, 0x6C, 0x57, 0x3F, 0x5d, 0x28, 0x2C, 0x0C }, /* 58-5F */
    { 0x2e, 0x60, 0x73, 0x7f, XXX , XXX , 0x29, XXX  }, /* 60-67 */
    { 0x56, 0x70, XXX , 0x59, 0x44, 0x5c, XXX , XXX  }, /* 68-6F */
    { 0x7d, 0x7e, 0x71, 0x5a, 0x5b, 0x45, 0x1A, 0x2d }, /* 70-77 */
    { 0x17, 0x47, 0x72, 0x30, 0x2f, 0x46, 0x31, 0x5e }, /* 78-7F */
};

const uint8_t map_cs3[MATRIX_ROWS][MATRIX_COLS] = {
    { XXX , 0x75, 0x5e, 0x31, XXX , XXX , 0x1a, 0x0d }, /* 00-07 */
    { 0x01, 0x74, 0x5d, XXX , 0x48, 0x33, 0x1b, 0x0e }, /* 08-0F */
    { 0x02, 0x76, 0x5f, 0x60, 0x4a, 0x34, 0x1c, 0x0f }, /* 10-17 */
    { 0x03, 0x77, 0x61, 0x4c, 0x4b, 0x35, 0x1d, 0x10 }, /* 18-1F */
    { 0x04, 0x63, 0x62, 0x4d, 0x36, 0x1f, 0x1e, 0x11 }, /* 20-27 */
    { 0x05, 0x78, 0x64, 0x4e, 0x38, 0x37, 0x20, 0x12 }, /* 28-2F */
    { 0x06, 0x66, 0x65, 0x50, 0x4f, 0x39, 0x21, 0x13 }, /* 30-37 */
    { 0x07, 0x79, 0x67, 0x51, 0x3a, 0x22, 0x23, 0x14 }, /* 38-3F */
    { 0x08, 0x68, 0x52, 0x3b, 0x3c, 0x25, 0x24, 0x15 }, /* 40-47 */
    { 0x09, 0x69, 0x6a, 0x53, 0x54, 0x3d, 0x26, 0x16 }, /* 48-4F */
    { 0x0a, XXX , 0x55, 0x28, 0x3e, 0x27, 0x17, 0x0b }, /* 50-57 */
    { 0x7a, 0x6c, 0x57, 0x3f, 0x29, XXX , 0x18, 0x0c }, /* 58-5F */
    { 0x7b, 0x6d, 0x2b, 0x58, 0x41, 0x42, 0x29, 0x2a }, /* 60-67 */
    { 0x5c, 0x70, 0x6f, 0x59, 0x44, 0x43, 0x2b, 0x2c }, /* 68-6F */
    { 0x7d, 0x7e, 0x71, 0x5a, 0x5b, 0x45, 0x2d, 0x2e }, /* 70-77 */
    { XXX , 0x73, 0x72, 0x5c, 0x47, 0x46, 0x2f, 0x30 }, /* 78-7F */
};
