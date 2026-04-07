// SPDX-License-Identifier: GPL-2.0-or-later
// CFW — IBM PC Keyboard Converter
// Handles AT (Scan Code Set 2), Terminal (Set 3), and clones
//
// Based on pio_ps2_converter by Markus Fritsche and ibmpc_usb by Jun Wako.
// Refactored into the CFW converter interface for multi-platform support.
//
// SUPPORTED KEYBOARDS:
//   - IBM PC AT 84-key (Set 2)
//   - IBM Model M 101/102-key (Set 2)
//   - IBM Model M4/M13 with TrackPoint (Set 2 + PS/2 mouse)
//   - IBM 122-key Terminal (Set 3: AB85, AB86)
//   - IBM 5576-001/002/003 Japanese (Set 3 / Set 82h)
//   - IBM RT Keyboard (Set 3: BFB0)
//   - Cherry G80-2551 Terminal (Set 3)
//   - Televideo DEC (Set 3 via AB91)
//   - NCD N-97 (Set 3)
//   - Generic AT/PS/2 clones (Set 2, with or without ID response)
//   - Keyboards that don't ACK reset (Zenith Z-150 etc.)
//
// IDENTIFICATION FLOW (boot-time, one-shot):
//   1. Wait 3s for keyboard to settle after power-on
//   2. Send 0xFF (reset), wait for 0xAA (BAT passed)
//   3. Optionally receive BF BF (terminal keyboard BAT)
//   4. Send 0xF2 (read ID)
//   5. Classify by ID → select scan code set
//   6. Configure keyboard (set make/break for terminal, LEDs)

#pragma once

#include "cfw.h"

// The IBMPC converter instance
extern const cfw_converter_t cfw_ibmpc_converter;

// Unified 128-key matrix layout (8 rows × 16 cols)
// All scan code sets map into this via unimap_csN[] tables.
// The LAYOUT macro and keymap.c reference these positions.
#define CFW_IBMPC_MATRIX_ROWS 8
#define CFW_IBMPC_MATRIX_COLS 16
