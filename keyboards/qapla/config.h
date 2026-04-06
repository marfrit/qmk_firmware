// SPDX-License-Identifier: GPL-2.0-or-later
// QAP'LA keyboard definition
// Maps a standard 104-key keyboard via evdev
#pragma once

// Matrix dimensions: we use a 16x16 matrix = 256 positions
// Each evdev keycode maps to (row, col) = (code/16, code%16)
#define MATRIX_ROWS 16
#define MATRIX_COLS 16

// EEPROM: custom file-backed driver (generous — it's just a file on disk)
#define EEPROM_SIZE 16384

// VIA/Vial dynamic keymap config
#define DYNAMIC_KEYMAP_LAYER_COUNT 4
