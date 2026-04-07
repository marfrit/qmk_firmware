// SPDX-License-Identifier: GPL-2.0-or-later
// CFW — IBM 5291 "Bigfoot" Terminal Keyboard Converter
//
// Capacitive buckling spring keyboard from IBM 5291 Model 2 display terminal.
// No CPU on the keyboard — the host (converter) drives everything.
//
// HARDWARE:
//   - DA-15 connector
//   - Pair of decoder ICs (address lines → 96 strobe lines)
//   - IBM 4-channel capsense chip (P/N 5119699)
//   - No keyboard controller, no firmware, no protocol negotiation
//
// SCANNING:
//   Uses CFW_WIRE_MUXSTROBE: the converter sets a 7-bit address on the
//   decoder, pulses the gate signal, waits for the capsense chip to settle,
//   and reads the sense pin. Repeated for all 96 key positions.
//
// WIRING (from Soarer's bigfoot.sc, confirmed by RE):
//   DA-15 D6:D0 → 7 mux address pins (directly to decoder address inputs)
//   DA-15 Strobe → gate pin (active-low, directly to decoder enable)
//   DA-15 Data → sense pin (capsense output, active-high)
//   DA-15 GND → ground
//   PCB bolt → chassis ground (essential for capsense reference)
//
// MATRIX: 12 rows × 8 cols (96 positions mapped from muxstrobe addresses)
//   Row = muxstrobe_addr / 8, Col = muxstrobe_addr % 8
//
// KEYMAP: Derived from Soarer's bigfoot.sc configuration and cross-referenced
//   with the physical XT-style layout of the 5291 keyboard.

#pragma once

#include "cfw.h"

// The IBM 5291 converter instance
extern const cfw_converter_t cfw_ibm5291_converter;

// Matrix dimensions (96 keys = 12×8)
#define CFW_5291_MATRIX_ROWS 12
#define CFW_5291_MATRIX_COLS  8

// Number of muxstrobe addresses
#define CFW_5291_NUM_KEYS    96
