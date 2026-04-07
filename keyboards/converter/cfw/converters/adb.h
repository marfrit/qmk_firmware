// SPDX-License-Identifier: GPL-2.0-or-later
// CFW — Apple Desktop Bus Converter
//
// Single-wire, host-initiated command/response protocol used by Apple
// keyboards from 1986 (IIGS) through 1999 (last beige PowerMacs).
//
// SUPPORTED KEYBOARDS:
//   - Apple Standard Keyboard M0116 (handler 0x01)
//   - Apple Extended Keyboard M0115 (handler 0x02)
//   - Apple Extended Keyboard II M3501 (handler 0x02)
//   - Apple Adjustable Keyboard M1242 (handler 0x10)
//   - ISO variants of the above (handlers 0x04, 0x05)
//
// WIRE PROTOCOL:
//   Uses cfw_wire_clock_data_t but only the DATA pin — ADB is single-wire.
//   The clock_pin field is unused. All framing is done via send_bit/recv_bit
//   with custom timing (bit0: 65us low + 35us high, bit1: 35us low + 65us high).
//
// IDENTIFICATION FLOW (boot-time, one-shot):
//   1. Wait 2s for keyboard to settle
//   2. Send Talk reg 3 — read handler ID from low byte
//   3. Try switching to handler 0x03 (right-modifier support) via Listen reg 3
//   4. Classify as standard/extended/ISO
//
// KEY DATA FORMAT (Talk reg 0, 16 bits):
//   Bit 15:    key0 released
//   Bits 14-8: key0 scancode (7-bit)
//   Bit 7:     key1 released
//   Bits 6-0:  key1 scancode (7-bit), 0x7F = no second key
//
// MATRIX: 16 rows × 8 cols
//   Row = scancode bits 6-3, col = scancode bits 2-0

#pragma once

#include "cfw.h"

// The ADB converter instance
extern const cfw_converter_t cfw_adb_converter;

// Matrix dimensions
#define CFW_ADB_MATRIX_ROWS 16
#define CFW_ADB_MATRIX_COLS 8

// ADB addresses
#define ADB_ADDR_KEYBOARD  2
#define ADB_ADDR_MOUSE     3

// ADB commands (shifted into command byte position)
#define ADB_CMD_RESET      0
#define ADB_CMD_FLUSH      1
#define ADB_CMD_LISTEN     8
#define ADB_CMD_TALK       12

// ADB registers
#define ADB_REG_0          0
#define ADB_REG_1          1
#define ADB_REG_2          2
#define ADB_REG_3          3

// Handler IDs
#define ADB_HANDLER_STD          0x01  // Apple Standard Keyboard (IIGS, M0116)
#define ADB_HANDLER_AEK          0x02  // Apple Extended Keyboard (M0115, M3501)
#define ADB_HANDLER_AEK_RMOD     0x03  // AEK with right modifier differentiation
#define ADB_HANDLER_STD_ISO      0x04  // Standard, ISO layout
#define ADB_HANDLER_AEK_ISO      0x05  // Extended, ISO layout
#define ADB_HANDLER_ADJUSTABLE   0x10  // Apple Adjustable Keyboard M1242

// Special keycodes
#define ADB_KEY_POWER      0x7F
