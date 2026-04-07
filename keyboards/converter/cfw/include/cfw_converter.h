// SPDX-License-Identifier: GPL-2.0-or-later
// CFW — Converter interface
// Top layer: keyboard protocol handling, identification, scancode translation
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "cfw_wire.h"
#include "cfw_feedback.h"
#include "matrix.h"    // QMK's matrix_row_t

// --- Keyboard identification (populated once at boot) ---
typedef enum {
    CFW_KB_UNKNOWN = 0,
    // IBM family
    CFW_KB_IBM_XT,
    CFW_KB_IBM_XT_CLONE,
    CFW_KB_IBM_AT,
    CFW_KB_IBM_PS2,
    CFW_KB_IBM_PS2_SET3,
    CFW_KB_IBM_CLONE_NO_ACK,
    CFW_KB_IBM_CLONE_NO_ID,
    // Apple
    CFW_KB_ADB_STANDARD,
    CFW_KB_ADB_EXTENDED,
    CFW_KB_ADB_ISO,
    CFW_KB_M0110,
    CFW_KB_M0110A,
    // Sun
    CFW_KB_SUN_TYPE3,
    CFW_KB_SUN_TYPE4,
    CFW_KB_SUN_TYPE5,
    CFW_KB_SUN_TYPE6,
    // Serial
    CFW_KB_PALM_STOWAWAY,
    CFW_KB_PALM_HANDSPRING,
    // Amiga
    CFW_KB_AMIGA_1200,
    // IBM terminal (muxstrobe / capsense)
    CFW_KB_IBM_5291,
    // Other
    CFW_KB_SIEMENS,
    CFW_KB_HP_46010A,
} cfw_keyboard_type_t;

typedef struct {
    cfw_keyboard_type_t type;
    uint16_t id;              // device ID from identify command (0xFFFF = unknown)
    uint8_t scan_set;         // active scan code set (1, 2, 3, or 0 = N/A)
    bool bidirectional;       // can we send commands?
    bool has_ack;             // does it ACK commands?
    bool needs_slow_init;     // clone that needs generous delays
    bool inverted_logic;      // negative logic (Sun)
    const char *description;  // human-readable, e.g. "IBM Model M (PS/2, Set 2)"
} cfw_keyboard_info_t;

// --- Converter interface ---
typedef struct {
    const char *name;         // "IBM PC Keyboard", "Apple Desktop Bus", etc.

    // Identify keyboard at boot (one-shot, generous timeouts)
    // Populates info struct. Returns true on success.
    bool (*identify)(cfw_wire_t *wire, cfw_keyboard_info_t *info);

    // Initialize keyboard after identification
    // Send reset, set scan set, enable scanning, etc.
    bool (*init)(cfw_wire_t *wire, cfw_keyboard_info_t *info);

    // --- Input (pick one per converter) ---

    // Event-driven: return next key event
    // Called repeatedly from matrix_scan() until it returns false (no more pending)
    // row/col are matrix coordinates, pressed is make/break
    bool (*next_event)(cfw_wire_t *wire, cfw_keyboard_info_t *info,
                       uint8_t *row, uint8_t *col, bool *pressed);

    // Polled: fill matrix directly (for shift-register / GPIO matrix keyboards)
    // Returns true if matrix changed
    bool (*scan_matrix)(cfw_wire_t *wire, cfw_keyboard_info_t *info,
                        matrix_row_t matrix[]);

    // --- Output ---

    // Set keyboard LEDs (CFW_LED_* mask)
    void (*set_leds)(cfw_wire_t *wire, cfw_keyboard_info_t *info,
                     uint8_t led_mask);

    // Optional extended feedback (OLED, etc.)
    const cfw_feedback_t *feedback;

} cfw_converter_t;

// LED mask bits (match USB HID)
#define CFW_LED_NUM_LOCK    0x01
#define CFW_LED_CAPS_LOCK   0x02
#define CFW_LED_SCROLL_LOCK 0x04

// --- QMK integration ---
// Called from keyboard's matrix.c to drive the converter
void cfw_matrix_init(const cfw_converter_t *conv, cfw_wire_t *wire);
uint8_t cfw_matrix_scan(matrix_row_t matrix[]);
void cfw_set_leds(uint8_t led_mask);
const cfw_keyboard_info_t *cfw_get_keyboard_info(void);
