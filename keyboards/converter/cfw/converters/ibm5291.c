// SPDX-License-Identifier: GPL-2.0-or-later
// CFW — IBM 5291 "Bigfoot" Terminal Keyboard Converter
//
// Reverse-engineered from Soarer's Keyboard Controller v1.20 beta4
// (ATmega32U4 firmware, closed-source) using a custom AVR disassembler.
//
// KEY FINDINGS FROM RE:
//   - Matrix scan at 0x0796: outer strobe loop, inner sense read
//   - Pin drivers at 0x075E-0x0794: 4 modes (low/pullup/high/float)
//     manipulating DDR and PORT via Y-pointer indexed I/O
//   - Muxstrobe variant at 0x0854: drives decoder address before gate pulse
//   - Debounce: shift-register accumulator (shift left, OR new bit)
//   - Sense polarity: active-high for capsense (sense_polarity 1)
//   - Gate: active-low pulse (muxstrobe_gate -PD1)
//
// The 5291 keyboard has no CPU. Two decoder ICs convert a 7-bit address
// into 96 strobe lines feeding an IBM 4-channel capsense chip (5119699).
// The converter sets the address, pulses the gate, waits, reads the sense.

#include "ibm5291.h"
#include <string.h>

// ============================================================================
// KEYMAP: MUXSTROBE ADDRESS → UNIMAP POSITION
// ============================================================================
// Derived from Soarer's bigfoot.sc configuration.
// Each entry maps muxstrobe address (0-95) to a UNIMAP position (row*8+col).
// U_NO = unassigned/unused position.

#define U_NO 0xFF
#define U(row, col) ((row) * CFW_5291_MATRIX_COLS + (col))

// HID keycodes for reference in comments (from Soarer's config)
// The actual keycode translation happens in QMK's keymap.c via LAYOUT macro.
// This table just assigns each muxstrobe address a unique matrix position.
//
// Muxstrobe addresses are physically wired on the PCB — the order follows
// the capsense chip's channel layout, not the visual keyboard layout.
// We map them linearly: muxstrobe N → row N/8, col N%8.
//
// The LAYOUT macro in the QMK keyboard definition will handle the
// visual-to-matrix mapping. Here we just need stable positions.

// For the default keymap, we provide the Soarer-derived HID mapping.
// This maps muxstrobe_addr → HID keycode (USB usage page 0x07).
// 0x00 = unassigned.
static const uint8_t muxstrobe_to_hid[CFW_5291_NUM_KEYS] = {
    // Addr  Key         HID code
    0x1D, // 00  Z           KC_Z
    0x16, // 01  S           KC_S
    0x1A, // 02  W           KC_W
    0x20, // 03  3           KC_3
    0x1B, // 04  X           KC_X
    0x07, // 05  D           KC_D
    0x08, // 06  E           KC_E
    0x21, // 07  4           KC_4
    0x06, // 08  C           KC_C
    0x09, // 09  F           KC_F
    0x15, // 10  R           KC_R
    0x22, // 11  5           KC_5
    0x19, // 12  V           KC_V
    0x0A, // 13  G           KC_G
    0x17, // 14  T           KC_T
    0x23, // 15  6           KC_6
    0x05, // 16  B           KC_B
    0x0B, // 17  H           KC_H
    0x1C, // 18  Y           KC_Y
    0x24, // 19  7           KC_7
    0x11, // 20  N           KC_N
    0x0D, // 21  J           KC_J
    0x18, // 22  U           KC_U
    0x25, // 23  8           KC_8
    0x5A, // 24  PAD_2       KC_KP_2
    0x5D, // 25  PAD_5       KC_KP_5
    0x60, // 26  PAD_8       KC_KP_8
    0x53, // 27  NUM_LOCK    KC_NUM_LOCK
    0x10, // 28  M           KC_M
    0x0E, // 29  K           KC_K
    0x0C, // 30  I           KC_I
    0x26, // 31  9           KC_9
    0x36, // 32  COMMA       KC_COMMA
    0x0F, // 33  L           KC_L
    0x12, // 34  O           KC_O
    0x27, // 35  0           KC_0
    0x37, // 36  PERIOD      KC_DOT
    0x33, // 37  SEMICOLON   KC_SEMICOLON
    0x13, // 38  P           KC_P
    0x2D, // 39  MINUS       KC_MINUS
    0x38, // 40  SLASH       KC_SLASH
    0x34, // 41  QUOTE       KC_QUOTE
    0x2F, // 42  LEFT_BRACE  KC_LBRACKET
    0x2E, // 43  EQUAL       KC_EQUAL
    0x2C, // 44  SPACE       KC_SPACE
    0xE5, // 45  RSHIFT      KC_RSHIFT
    0x31, // 46  BACKSLASH   KC_BACKSLASH
    0x30, // 47  RIGHT_BRACE KC_RBRACKET
    0x39, // 48  CAPS_LOCK   KC_CAPS_LOCK
    0x55, // 49  PAD_*       KC_KP_ASTERISK
    0x28, // 50  ENTER       KC_ENTER
    0x2A, // 51  BACKSPACE   KC_BACKSPACE
    0x62, // 52  PAD_0       KC_KP_0
    0x59, // 53  PAD_1       KC_KP_1
    0x5C, // 54  PAD_4       KC_KP_4
    0x5F, // 55  PAD_7       KC_KP_7
    0x57, // 56  PAD_+       KC_KP_PLUS
    0x00, // 57  UNASSIGNED
    0x56, // 58  PAD_-       KC_KP_MINUS
    0x47, // 59  SCROLL_LOCK KC_SCROLL_LOCK
    0x63, // 60  PAD_.       KC_KP_DOT
    0x5B, // 61  PAD_3       KC_KP_3
    0x5E, // 62  PAD_6       KC_KP_6
    0x61, // 63  PAD_9       KC_KP_9
    0x64, // 64  EUROPE_2    KC_NONUS_BACKSLASH (ISO \|)
    0x04, // 65  A           KC_A
    0x14, // 66  Q           KC_Q
    0x1F, // 67  2           KC_2
    0xE2, // 68  LALT        KC_LALT
    0x00, // 69  UNASSIGNED
    0x00, // 70  UNASSIGNED
    0x1E, // 71  1           KC_1
    0x40, // 72  F7          KC_F7
    0x3E, // 73  F5          KC_F5
    0x3C, // 74  F3          KC_F3
    0x3A, // 75  F1          KC_F1
    0x41, // 76  F8          KC_F8
    0x3F, // 77  F6          KC_F6
    0x3D, // 78  F4          KC_F4
    0x3B, // 79  F2          KC_F2
    0x43, // 80  F10         KC_F10
    0x00, // 81  UNASSIGNED
    0x00, // 82  UNASSIGNED
    0x00, // 83  UNASSIGNED
    0x42, // 84  F9          KC_F9
    0x00, // 85  UNASSIGNED
    0x00, // 86  UNASSIGNED
    0x00, // 87  UNASSIGNED
    0xE1, // 88  LSHIFT      KC_LSHIFT
    0xE0, // 89  LCTRL       KC_LCTRL
    0x2B, // 90  TAB         KC_TAB
    0x29, // 91  ESC         KC_ESCAPE
    0x00, // 92  UNASSIGNED
    0x00, // 93  UNASSIGNED
    0x00, // 94  UNASSIGNED
    0x00, // 95  UNASSIGNED
};

// ============================================================================
// DEBOUNCE STATE
// ============================================================================
// Shift-register debounce: each key has an 8-bit accumulator.
// Each scan, shift left and OR in the current state.
// Key is pressed when accumulator reaches the debounce threshold (all 1s).
// Key is released when accumulator drops below threshold (all 0s).
// This matches Soarer's implementation (RE'd from 0x0796 scan loop).

#define CFW_5291_DEBOUNCE_MASK 0x03  // 2 scans (debounce 1 in Soarer config)

static uint8_t debounce_accum[CFW_5291_NUM_KEYS];
static uint8_t key_state[CFW_5291_NUM_KEYS];  // 0=released, 1=pressed
static uint8_t pending_events[CFW_5291_NUM_KEYS];  // ring buffer of changed keys
static uint8_t pending_head, pending_tail, pending_count;

// ============================================================================
// MUXSTROBE SCANNING
// ============================================================================

static cfw_wire_muxstrobe_t *mux(cfw_wire_t *wire) {
    return &wire->muxstrobe;
}

// Set decoder address pins to select muxstrobe line N
static void set_mux_address(cfw_wire_t *wire, uint8_t addr) {
    cfw_wire_muxstrobe_t *m = mux(wire);
    for (uint8_t i = 0; i < m->mux_pin_count; i++) {
        m->hw->pin_write(m->mux_pins[i], (addr >> i) & 1);
    }
}

// Pulse the gate signal to trigger the decoder/capsense
static void pulse_gate(cfw_wire_t *wire) {
    cfw_wire_muxstrobe_t *m = mux(wire);
    uint8_t active = m->gate_active_low ? 0 : 1;
    uint8_t inactive = m->gate_active_low ? 1 : 0;

    m->hw->pin_write(m->gate_pin, active);
    // Minimal pulse width — the decoder latches on the edge
    m->hw->delay_us(1);
    m->hw->pin_write(m->gate_pin, inactive);
}

// Read a single sense pin
static uint8_t read_sense(cfw_wire_t *wire, uint8_t sense_idx) {
    cfw_wire_muxstrobe_t *m = mux(wire);
    uint8_t raw = m->hw->pin_read(m->sense_pins[sense_idx]);
    return m->sense_active_high ? raw : !raw;
}

// Scan all 96 keys, updating debounce accumulators and generating events
static void scan_all_keys(cfw_wire_t *wire) {
    cfw_wire_muxstrobe_t *m = mux(wire);

    for (uint8_t addr = 0; addr < CFW_5291_NUM_KEYS; addr++) {
        // Skip unassigned keys
        if (muxstrobe_to_hid[addr] == 0x00) continue;

        // Set decoder address
        set_mux_address(wire, addr);

        // Pulse gate to trigger capsense sampling
        pulse_gate(wire);

        // Wait for capsense chip to settle
        if (m->sense_delay_us > 0) {
            m->hw->delay_us(m->sense_delay_us);
        }

        // Read sense (5291 has single sense pin)
        uint8_t pressed = read_sense(wire, 0);

        // Shift-register debounce
        debounce_accum[addr] = (debounce_accum[addr] << 1) | pressed;

        uint8_t masked = debounce_accum[addr] & CFW_5291_DEBOUNCE_MASK;
        uint8_t new_state = key_state[addr];

        if (masked == CFW_5291_DEBOUNCE_MASK && !key_state[addr]) {
            // All recent samples high → key pressed
            new_state = 1;
        } else if (masked == 0x00 && key_state[addr]) {
            // All recent samples low → key released
            new_state = 0;
        }

        if (new_state != key_state[addr]) {
            key_state[addr] = new_state;
            // Queue event
            if (pending_count < CFW_5291_NUM_KEYS) {
                pending_events[pending_tail] = addr;
                pending_tail = (pending_tail + 1) % CFW_5291_NUM_KEYS;
                pending_count++;
            }
        }
    }
}

// ============================================================================
// CONVERTER INTERFACE
// ============================================================================

static bool ibm5291_identify(cfw_wire_t *wire, cfw_keyboard_info_t *info) {
    // No identification possible — the 5291 has no CPU and no way to
    // identify itself. If the hardware is wired, it's a 5291.
    info->type = CFW_KB_IBM_5291;
    info->id = 0x5291;
    info->scan_set = 0;     // N/A
    info->bidirectional = false;
    info->has_ack = false;
    info->needs_slow_init = false;
    info->inverted_logic = false;
    info->description = "IBM 5291 Model F (Bigfoot, capacitive buckling spring)";
    return true;
}

static bool ibm5291_init(cfw_wire_t *wire, cfw_keyboard_info_t *info) {
    cfw_wire_muxstrobe_t *m = mux(wire);

    // Initialize state
    memset(debounce_accum, 0, sizeof(debounce_accum));
    memset(key_state, 0, sizeof(key_state));
    memset(pending_events, 0, sizeof(pending_events));
    pending_head = pending_tail = pending_count = 0;

    // Configure mux address pins as outputs
    for (uint8_t i = 0; i < m->mux_pin_count; i++) {
        m->hw->pin_mode(m->mux_pins[i], CFW_PIN_OUTPUT);
        m->hw->pin_write(m->mux_pins[i], 0);
    }

    // Configure gate pin as output, initially inactive
    m->hw->pin_mode(m->gate_pin, CFW_PIN_OUTPUT);
    m->hw->pin_write(m->gate_pin, m->gate_active_low ? 1 : 0);

    // Configure sense pins as inputs with pull-up
    for (uint8_t i = 0; i < m->sense_pin_count; i++) {
        m->hw->pin_mode(m->sense_pins[i], CFW_PIN_INPUT_PULLUP);
    }

    // Do a few throwaway scans to let the debounce accumulators fill
    for (uint8_t i = 0; i < 4; i++) {
        scan_all_keys(wire);
    }

    // Discard any events from init scans
    pending_head = pending_tail = pending_count = 0;

    return true;
}

static bool ibm5291_next_event(cfw_wire_t *wire, cfw_keyboard_info_t *info,
                                uint8_t *row, uint8_t *col, bool *pressed) {
    // If no pending events, do a new scan
    if (pending_count == 0) {
        scan_all_keys(wire);
    }

    // Dequeue an event
    if (pending_count > 0) {
        uint8_t addr = pending_events[pending_head];
        pending_head = (pending_head + 1) % CFW_5291_NUM_KEYS;
        pending_count--;

        *row = addr / CFW_5291_MATRIX_COLS;
        *col = addr % CFW_5291_MATRIX_COLS;
        *pressed = key_state[addr];
        return true;
    }

    return false;
}

// The 5291 has no LEDs — it's a dumb capacitive matrix with no feedback path.
static void ibm5291_set_leds(cfw_wire_t *wire, cfw_keyboard_info_t *info,
                              uint8_t led_mask) {
    (void)wire; (void)info; (void)led_mask;
    // No LEDs on the keyboard. LED indicators live on the converter board.
}

// ============================================================================
// CONVERTER INSTANCE
// ============================================================================

const cfw_converter_t cfw_ibm5291_converter = {
    .name        = "IBM 5291 Bigfoot",
    .identify    = ibm5291_identify,
    .init        = ibm5291_init,
    .next_event  = ibm5291_next_event,
    .scan_matrix = NULL,  // event-driven via muxstrobe scanning
    .set_leds    = ibm5291_set_leds,
    .feedback    = NULL,
};
