// SPDX-License-Identifier: GPL-2.0-or-later
// CFW — IBM PC Keyboard Converter
//
// This is the flagship converter: AT, PS/2, Terminal, and clones.
// Ported from pio_ps2_converter (Markus Fritsche) into the CFW
// converter interface. Runs on any CFW platform (RP2040, AVR, STM32).
//
// The original code had deep coupling to QMK's matrix and ps2_host
// APIs. This version uses cfw_wire_t for I/O, making the protocol
// logic platform-independent.

#include "ibmpc.h"
#include <string.h>

// ============================================================================
// SCAN CODE TRANSLATION TABLES
// ============================================================================
// These map native scan codes to the unified 128-key UNIMAP positions.
// Each scan code set has its own table because the byte values are
// completely different (Set 2: A=0x1C, Set 3: A=0x1C — same by
// coincidence, but most keys differ).
//
// The tables are from pio_ps2_converter.c (Markus Fritsche), which
// derived them from TMK (Jun Wako). They're the definitive reference
// for IBM keyboard scan code mapping.

// UNIMAP position constants (row*16 + col in the 8x16 matrix)
// These match the LAYOUT macro positions.
#define U_NO   0xFF
#define U(row,col) ((row)*16 + (col))

// Forward declarations
static uint8_t cs2_e0code(uint8_t code);
static uint8_t translate_5576_cs2(uint8_t code, uint16_t kb_id);
static uint8_t translate_5576_cs2_e0(uint8_t code, uint16_t kb_id);

// --- Scan Code Set 2 → UNIMAP ---
// Index: raw Set 2 scan code (0x00-0x7F)
// Value: UNIMAP position (row*16 + col) or U_NO
static const uint8_t unimap_cs2[128] = {
    // Directly from pio_ps2_converter.c unimap_cs2 table
    // Flattened from 8x16 to linear: unimap_cs2[row][col] → unimap_cs2_flat[row*16+col]
    // Row 0 (codes 0x00-0x0F)
    0x48, 0x42, 0x40, 0x3E, 0x3C, 0x3A, 0x3B, 0x45,
    0x68, 0x43, 0x41, 0x3F, 0x3D, 0x2B, 0x35, 0x0F,
    // Row 1 (codes 0x10-0x1F)
    0x69, 0x7A, 0x79, 0x00, 0x78, 0x14, 0x1E, 0x7B,
    0x6A, 0x7C, 0x1D, 0x16, 0x04, 0x1A, 0x1F, 0x7F,
    // Row 2 (codes 0x20-0x2F)
    0x6B, 0x06, 0x1B, 0x07, 0x08, 0x21, 0x20, 0x65,
    0x6C, 0x2C, 0x19, 0x09, 0x17, 0x15, 0x22, 0x4A,
    // Row 3 (codes 0x30-0x3F)
    0x6D, 0x11, 0x05, 0x0B, 0x0A, 0x1C, 0x23, 0x4C,
    0x6E, 0x49, 0x10, 0x0D, 0x18, 0x24, 0x25, 0x51,
    // Row 4 (codes 0x40-0x4F)
    0x6F, 0x36, 0x0E, 0x0C, 0x12, 0x27, 0x26, 0x4F,
    0x70, 0x37, 0x38, 0x0F, 0x33, 0x13, 0x2D, 0x52,
    // Row 5 (codes 0x50-0x5F)
    0x71, 0x75, 0x34, 0x50, 0x2F, 0x2E, 0x4E, 0x72,
    0x39, 0x7D, 0x28, 0x30, 0x4D, 0x31, 0x4B, 0x73,
    // Row 6 (codes 0x60-0x6F)
    0x54, 0x64, 0x58, 0x67, 0x76, 0x01, 0x2A, 0x77,
    0x32, 0x59, 0x74, 0x5C, 0x5F, 0x66, 0x02, 0x03,
    // Row 7 (codes 0x70-0x7F)
    0x62, 0x63, 0x5A, 0x5D, 0x5E, 0x60, 0x29, 0x53,
    0x44, 0x57, 0x5B, 0x56, 0x55, 0x61, 0x47, 0x46,
};

// --- Scan Code Set 3 → UNIMAP ---
static const uint8_t unimap_cs3[128] = {
    // Row 0 (codes 0x00-0x0F)
    0x00, 0x7B, 0x46, 0x01, 0x02, 0x03, 0x76, 0x3A,
    0x68, 0x7F, 0x65, 0x77, 0x48, 0x2B, 0x35, 0x3B,
    // Row 1 (codes 0x10-0x1F)
    0x69, 0x78, 0x79, 0x64, 0x39, 0x14, 0x1E, 0x3C,
    0x6A, 0x7A, 0x1D, 0x16, 0x04, 0x1A, 0x1F, 0x3D,
    // Row 2 (codes 0x20-0x2F)
    0x6B, 0x06, 0x1B, 0x07, 0x08, 0x21, 0x20, 0x3E,
    0x6C, 0x2C, 0x19, 0x09, 0x17, 0x15, 0x22, 0x3F,
    // Row 3 (codes 0x30-0x3F)
    0x6D, 0x11, 0x05, 0x0B, 0x0A, 0x1C, 0x23, 0x40,
    0x6E, 0x7E, 0x10, 0x0D, 0x18, 0x24, 0x25, 0x41,
    // Row 4 (codes 0x40-0x4F)
    0x6F, 0x36, 0x0E, 0x0C, 0x12, 0x27, 0x26, 0x42,
    0x70, 0x37, 0x38, 0x0F, 0x33, 0x13, 0x2D, 0x43,
    // Row 5 (codes 0x50-0x5F)
    0x71, 0x75, 0x34, 0x32, 0x2F, 0x2E, 0x44, 0x72,
    0x7C, 0x7D, 0x28, 0x30, 0x31, 0x74, 0x45, 0x73,
    // Row 6 (codes 0x60-0x6F)
    0x51, 0x50, 0x4A, 0x52, 0x4D, 0x49, 0x2A, 0x54,
    0x66, 0x59, 0x4F, 0x5C, 0x5F, 0x4C, 0x4B, 0x4E,
    // Row 7 (codes 0x70-0x7F)
    0x62, 0x63, 0x5A, 0x5D, 0x5E, 0x60, 0x29, 0x53,
    0x67, 0x58, 0x5B, 0x56, 0x57, 0x61, 0x47, 0x55,
};


// ============================================================================
// E0-PREFIXED CODE REMAPPING (Set 2)
// ============================================================================

static uint8_t cs2_e0code(uint8_t code) {
    switch (code) {
        case 0x11: return 0x0F; // RALT
        case 0x14: return 0x17; // RCTRL
        case 0x1F: return 0x19; // LGUI
        case 0x27: return 0x1F; // RGUI
        case 0x2F: return 0x5C; // APP
        case 0x4A: return 0x60; // KP /
        case 0x5A: return 0x62; // KP Enter
        case 0x69: return 0x27; // End
        case 0x6B: return 0x53; // Left
        case 0x6C: return 0x2F; // Home
        case 0x70: return 0x39; // Insert
        case 0x71: return 0x37; // Delete
        case 0x72: return 0x3F; // Down
        case 0x74: return 0x47; // Right
        case 0x75: return 0x4F; // Up
        case 0x7A: return 0x56; // PageDown
        case 0x7D: return 0x5E; // PageUp
        case 0x7C: return 0x7F; // PrintScreen
        case 0x7E: return 0x00; // Ctrl+Pause
        case 0x21: return 0x65; // Volume Down
        case 0x32: return 0x6E; // Volume Up
        case 0x23: return 0x6F; // Mute
        default:   return (code & 0x7F);
    }
}


// ============================================================================
// PROTOCOL STATE MACHINES
// ============================================================================

// --- Internal state ---
typedef struct {
    cfw_keyboard_info_t *info;
    uint16_t keyboard_id;

    // Set 2 state machine
    enum { CS2_INIT, CS2_F0, CS2_E0, CS2_E0_F0,
           CS2_E1, CS2_E1_14, CS2_E1_F0, CS2_E1_F0_14, CS2_E1_F0_14_F0 } cs2_state;

    // Set 3 state machine
    enum { CS3_READY, CS3_F0 } cs3_state;

} ibmpc_state_t;

static ibmpc_state_t state;

// Process a Set 2 byte. Returns true if a key event was produced.
static bool process_cs2(uint8_t code, uint8_t *out_unimap, bool *out_pressed) {
    switch (state.cs2_state) {
        case CS2_INIT:
            switch (code) {
                case 0xE0: state.cs2_state = CS2_E0; return false;
                case 0xF0: state.cs2_state = CS2_F0; return false;
                case 0xE1: state.cs2_state = CS2_E1; return false;
                case 0xAA: case 0xFC: return false; // BAT codes, ignore in scan loop
                case 0x83: *out_unimap = unimap_cs2[0x02]; *out_pressed = true; return true; // F7 quirk
                case 0x84: *out_unimap = unimap_cs2[0x7F]; *out_pressed = true; return true; // Alt+PrtSc
                default:
                    if (code < 0x80) {
                        *out_unimap = unimap_cs2[code];
                        *out_pressed = true;
                        return true;
                    }
                    return false;
            }

        case CS2_E0:
            state.cs2_state = CS2_INIT;
            switch (code) {
                case 0x12: case 0x59: return false; // fake shift, ignore
                case 0xF0: state.cs2_state = CS2_E0_F0; return false;
                default:
                    if (code < 0x80) {
                        *out_unimap = unimap_cs2[cs2_e0code(code)];
                        *out_pressed = true;
                        return true;
                    }
                    return false;
            }

        case CS2_F0:
            state.cs2_state = CS2_INIT;
            if (code == 0x83) { *out_unimap = unimap_cs2[0x02]; *out_pressed = false; return true; }
            if (code == 0x84) { *out_unimap = unimap_cs2[0x7F]; *out_pressed = false; return true; }
            if (code < 0x80) {
                *out_unimap = unimap_cs2[code];
                *out_pressed = false;
                return true;
            }
            return false;

        case CS2_E0_F0:
            state.cs2_state = CS2_INIT;
            if (code == 0x12 || code == 0x59) return false;
            if (code < 0x80) {
                *out_unimap = unimap_cs2[cs2_e0code(code)];
                *out_pressed = false;
                return true;
            }
            return false;

        // Pause: E1 14 77 (make), E1 F0 14 F0 77 (break)
        case CS2_E1:
            if (code == 0x14) { state.cs2_state = CS2_E1_14; return false; }
            if (code == 0xF0) { state.cs2_state = CS2_E1_F0; return false; }
            state.cs2_state = CS2_INIT; return false;
        case CS2_E1_14:
            state.cs2_state = CS2_INIT;
            if (code == 0x77) { *out_unimap = unimap_cs2[0x00]; *out_pressed = true; return true; }
            return false;
        case CS2_E1_F0:
            if (code == 0x14) { state.cs2_state = CS2_E1_F0_14; return false; }
            state.cs2_state = CS2_INIT; return false;
        case CS2_E1_F0_14:
            if (code == 0xF0) { state.cs2_state = CS2_E1_F0_14_F0; return false; }
            state.cs2_state = CS2_INIT; return false;
        case CS2_E1_F0_14_F0:
            state.cs2_state = CS2_INIT;
            if (code == 0x77) { *out_unimap = unimap_cs2[0x00]; *out_pressed = false; return true; }
            return false;

        default:
            state.cs2_state = CS2_INIT;
            return false;
    }
}

// Process a Set 3 byte
static bool process_cs3(uint8_t code, uint8_t *out_unimap, bool *out_pressed) {
    // Filter protocol codes
    if (code == 0xAA || code == 0xFC || code == 0xBF || code == 0xAB) return false;

    switch (state.cs3_state) {
        case CS3_READY:
            switch (code) {
                case 0xF0:
                    state.cs3_state = CS3_F0;
                    return false;
                case 0x83: *out_unimap = unimap_cs3[0x02]; *out_pressed = true; return true; // PrtSc
                case 0x84: *out_unimap = unimap_cs3[0x7F]; *out_pressed = true; return true; // KP *
                default:
                    if (code < 0x80) {
                        *out_unimap = unimap_cs3[code];
                        *out_pressed = true;
                        return true;
                    }
                    return false;
            }

        case CS3_F0:
            state.cs3_state = CS3_READY;
            if (code == 0x83) { *out_unimap = unimap_cs3[0x02]; *out_pressed = false; return true; }
            if (code == 0x84) { *out_unimap = unimap_cs3[0x7F]; *out_pressed = false; return true; }
            if (code < 0x80) {
                *out_unimap = unimap_cs3[code];
                *out_pressed = false;
                return true;
            }
            return false;

        default:
            state.cs3_state = CS3_READY;
            return false;
    }
}


// ============================================================================
// CONVERTER INTERFACE IMPLEMENTATION
// ============================================================================

static int16_t wire_recv(cfw_wire_t *wire, uint32_t timeout_us) {
    return wire->clock_data.recv_byte(&wire->clock_data, timeout_us);
}

static int16_t wire_send(cfw_wire_t *wire, uint8_t byte) {
    return wire->clock_data.send_byte(&wire->clock_data, byte);
}

static int16_t wire_recv_wait(cfw_wire_t *wire, uint16_t timeout_ms) {
    // Poll for data with millisecond granularity
    for (uint16_t i = 0; i < timeout_ms; i++) {
        int16_t code = wire_recv(wire, 1000);  // 1ms timeout per attempt
        if (code >= 0) return code;
    }
    return CFW_ERR_TIMEOUT;
}

// --- Identify ---
static bool ibmpc_identify(cfw_wire_t *wire, cfw_keyboard_info_t *info) {
    memset(&state, 0, sizeof(state));
    state.info = info;

    // Drain any pending data
    while (wire_recv(wire, 1000) >= 0) {}

    // Send reset (0xFF) with generous timeout for elderly keyboards
    int16_t ack = wire_send(wire, 0xFF);
    if (ack != 0xFA) {
        // No ACK on reset — could be XT or very old clone
        // Wait for BAT anyway
    }

    // Wait for BAT (0xAA) — AT 84-key can take up to 9.9s per TechRef.
    // Use 11s to be safe with old/slow hardware.
    int16_t bat = wire_recv_wait(wire, 11000);

    // Wait for optional BF BF (terminal BAT)
    int16_t bf1 = wire_recv_wait(wire, 500);
    if (bf1 >= 0) {
        wire_recv_wait(wire, 500);  // second BF
    }

    // Read keyboard ID (0xF2)
    uint16_t kb_id = 0;
    ack = wire_send(wire, 0xF2);
    if (ack == 0xFA) {
        int16_t id_hi = wire_recv_wait(wire, 500);
        if (id_hi < 0) {
            kb_id = 0x0000;  // AT 84-key: no ID response
        } else {
            kb_id = ((uint16_t)(id_hi & 0xFF)) << 8;
            int16_t id_lo = wire_recv_wait(wire, 500);
            if (id_lo >= 0) kb_id |= (id_lo & 0xFF);
        }
    } else if (ack < 0) {
        kb_id = 0xFFFF;  // No keyboard
    } else {
        kb_id = 0xFFFE;  // Broken response
    }

    state.keyboard_id = kb_id;
    info->id = kb_id;

    // Classify by ID
    if (kb_id == 0x0000 || kb_id == 0xFFFE || kb_id == 0xFFFD) {
        info->type = CFW_KB_IBM_AT;
        info->scan_set = 2;
        info->bidirectional = true;
        info->has_ack = (kb_id != 0xFFFE);
        info->description = "IBM AT (Set 2)";
    } else if (kb_id == 0xAB85 || kb_id == 0xAB86 || kb_id == 0xAB92) {
        // Try Set 3
        if (wire_send(wire, 0xF0) == 0xFA && wire_send(wire, 0x03) == 0xFA) {
            info->type = CFW_KB_IBM_PS2_SET3;
            info->scan_set = 3;
            info->description = "IBM Terminal (Set 3)";
        } else {
            info->type = CFW_KB_IBM_AT;
            info->scan_set = 2;
            info->description = "IBM AT-compat (Set 2)";
        }
        info->bidirectional = true;
        info->has_ack = true;
    } else if (kb_id == 0xBFB0) {
        info->type = CFW_KB_IBM_PS2_SET3;
        info->scan_set = 3;
        info->bidirectional = true;
        info->has_ack = true;
        info->description = "IBM RT (Set 3)";
    } else if ((kb_id & 0xFF00) == 0xAB00) {
        info->type = CFW_KB_IBM_PS2;
        info->scan_set = 2;
        info->bidirectional = true;
        info->has_ack = true;
        info->description = "IBM PS/2 (Set 2)";
    } else if ((kb_id & 0xFF00) == 0xBF00 || (kb_id & 0xFF00) == 0x7F00) {
        info->type = CFW_KB_IBM_PS2_SET3;
        info->scan_set = 3;
        info->bidirectional = true;
        info->has_ack = true;
        info->description = "Terminal (Set 3)";
    } else if (kb_id == 0xFFFF) {
        info->type = CFW_KB_UNKNOWN;
        info->description = "No keyboard detected";
        return false;
    } else {
        // Unknown ID — try Set 2 first, fallback to Set 3
        if (wire_send(wire, 0xF0) == 0xFA && wire_send(wire, 0x02) == 0xFA) {
            info->type = CFW_KB_IBM_AT;
            info->scan_set = 2;
        } else if (wire_send(wire, 0xF0) == 0xFA && wire_send(wire, 0x03) == 0xFA) {
            info->type = CFW_KB_IBM_PS2_SET3;
            info->scan_set = 3;
        } else {
            info->type = CFW_KB_IBM_AT;
            info->scan_set = 2;
        }
        info->bidirectional = true;
        info->has_ack = true;
        info->description = "Clone keyboard";
    }

    return true;
}

// --- Init ---
static bool ibmpc_init(cfw_wire_t *wire, cfw_keyboard_info_t *info) {
    if (info->scan_set == 3) {
        // Terminal keyboards: set all keys to make/break
        wire_send(wire, 0xF8);
    }
    return true;
}

// --- Next Event ---
static bool ibmpc_next_event(cfw_wire_t *wire, cfw_keyboard_info_t *info,
                              uint8_t *row, uint8_t *col, bool *pressed) {
    int16_t code = wire_recv(wire, 0);  // non-blocking
    if (code < 0) return false;

    // Error/overrun
    if (code == 0xFF || code == 0x00) return false;

    uint8_t unimap_pos = U_NO;
    bool is_pressed = false;

    bool got_event = false;
    if (info->scan_set == 2) {
        got_event = process_cs2((uint8_t)code, &unimap_pos, &is_pressed);
    } else if (info->scan_set == 3) {
        got_event = process_cs3((uint8_t)code, &unimap_pos, &is_pressed);
    }

    if (got_event && unimap_pos != U_NO) {
        *row = (unimap_pos >> 4) & 0x07;
        *col = unimap_pos & 0x0F;
        *pressed = is_pressed;
        return true;
    }

    return false;
}

// --- Set LEDs ---
static void ibmpc_set_leds(cfw_wire_t *wire, cfw_keyboard_info_t *info,
                            uint8_t led_mask) {
    if (info->type == CFW_KB_UNKNOWN) return;

    // Convert CFW LED mask to PS/2 LED byte
    // CFW: bit0=NUM, bit1=CAPS, bit2=SCROLL
    // PS/2: bit0=SCROLL, bit1=NUM, bit2=CAPS
    uint8_t ps2_led = 0;
    if (led_mask & CFW_LED_SCROLL_LOCK) ps2_led |= (1 << 0);
    if (led_mask & CFW_LED_NUM_LOCK)    ps2_led |= (1 << 1);
    if (led_mask & CFW_LED_CAPS_LOCK)   ps2_led |= (1 << 2);

    if (wire_send(wire, 0xED) == 0xFA) {
        wire_send(wire, ps2_led);
    }
}

// ============================================================================
// CONVERTER INSTANCE
// ============================================================================

const cfw_converter_t cfw_ibmpc_converter = {
    .name        = "IBM PC Keyboard",
    .identify    = ibmpc_identify,
    .init        = ibmpc_init,
    .next_event  = ibmpc_next_event,
    .scan_matrix = NULL,  // event-driven
    .set_leds    = ibmpc_set_leds,
    .feedback    = NULL,  // no OLED yet
};
