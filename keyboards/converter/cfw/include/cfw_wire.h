// SPDX-License-Identifier: GPL-2.0-or-later
// CFW — Wire protocol interface
// Middle layer: electrical signaling abstraction
// Each wire protocol (clock_data, uart) has a shared implementation
// that uses the platform driver, OR a platform-optimized override
// (e.g. PIO does the whole byte transfer in hardware)
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "cfw_platform.h"

// --- Error codes ---
#define CFW_OK           0
#define CFW_ERR_TIMEOUT -1
#define CFW_ERR_PARITY  -2
#define CFW_ERR_FRAME   -3
#define CFW_ERR_NAK     -4
#define CFW_ERR_BUS     -5

// --- Clock/Data wire protocol (PS/2, XT, ADB, M0110) ---
typedef struct {
    cfw_pin_t clock_pin;
    cfw_pin_t data_pin;
    const cfw_platform_t *hw;

    // --- Byte-level interface (primary) ---
    // Receive a byte from the keyboard
    // Returns 0-255 on success, negative on error (CFW_ERR_*)
    int16_t (*recv_byte)(void *self, uint32_t timeout_us);

    // Send a byte to the keyboard
    // Returns CFW_OK on success, negative on error
    int16_t (*send_byte)(void *self, uint8_t byte);

    // --- Bus control ---
    void (*inhibit)(void *self);   // pull clock low — keyboard must stop
    void (*release)(void *self);   // release both lines — keyboard may talk

    // --- Bit-level interface (escape hatch for non-standard framing) ---
    // Send a single bit (caller manages clock timing)
    void (*send_bit)(void *self, uint8_t bit);

    // Receive a single bit, waiting for clock edge
    // Returns 0 or 1 on success, negative on error
    int8_t (*recv_bit)(void *self, uint32_t timeout_us);

    // --- Bus state ---
    uint8_t (*clock_state)(void *self);
    uint8_t (*data_state)(void *self);

    // Platform-specific context (PIO sm number, timer channel, etc.)
    void *platform_ctx;

} cfw_wire_clock_data_t;

// --- UART wire protocol (Sun, Palm, Siemens) ---
typedef struct {
    const cfw_platform_t *hw;
    uint32_t baud;
    bool inverted;    // Sun Type 3/5 uses negative logic

    // Receive a byte (blocking with timeout)
    // Returns 0-255 on success, CFW_ERR_TIMEOUT on timeout
    int16_t (*recv_byte)(void *self, uint32_t timeout_us);

    // Send a byte
    void (*send_byte)(void *self, uint8_t byte);

    // --- Flow control (optional, NULL if not used) ---
    cfw_pin_t rts_pin;     // -1 if unused
    void (*set_rts)(void *self, uint8_t value);

    void *platform_ctx;

} cfw_wire_uart_t;

// --- Muxstrobe wire protocol (decoder-driven matrix, e.g. IBM 5291) ---
typedef struct {
    const cfw_platform_t *hw;

    // Muxstrobe address port: N pins driven as decoder address lines
    cfw_pin_t mux_pins[8];    // address pins, LSB first
    uint8_t mux_pin_count;    // number of address pins (e.g. 7 for 128 strobes)

    // Gate signal: active-low pulse triggers the decoder/capsense
    cfw_pin_t gate_pin;
    bool gate_active_low;     // true = active-low (typical for IBM)

    // Sense input: capsense chip output (single pin for 5291)
    cfw_pin_t sense_pins[8];  // sense input pins
    uint8_t sense_pin_count;  // typically 1 for capsense, more for direct matrix

    bool sense_active_high;   // polarity: true = keypress reads high

    uint8_t sense_delay_us;   // microseconds to wait after strobe before read

    void *platform_ctx;

} cfw_wire_muxstrobe_t;

// --- Generic wire handle (tagged union for converter API) ---
typedef enum {
    CFW_WIRE_CLOCK_DATA,
    CFW_WIRE_UART,
    CFW_WIRE_MATRIX,       // direct GPIO matrix — no wire protocol
    CFW_WIRE_MUXSTROBE,    // decoder-driven matrix (IBM 5291 etc.)
} cfw_wire_type_t;

typedef struct {
    cfw_wire_type_t type;
    union {
        cfw_wire_clock_data_t clock_data;
        cfw_wire_uart_t uart;
        cfw_wire_muxstrobe_t muxstrobe;
    };
} cfw_wire_t;

// --- Default implementations ---
// Generic clock/data recv_byte using platform pin_read/wait_edge polling
// Used when no platform-optimized override (PIO, timer capture) is available
int16_t cfw_clock_data_recv_byte_default(void *self, uint32_t timeout_us);
int16_t cfw_clock_data_send_byte_default(void *self, uint8_t byte);
void cfw_clock_data_inhibit_default(void *self);
void cfw_clock_data_release_default(void *self);
int8_t cfw_clock_data_recv_bit_default(void *self, uint32_t timeout_us);
void cfw_clock_data_send_bit_default(void *self, uint8_t bit);

// Generic software UART using platform timing + pin_read/pin_write
int16_t cfw_uart_recv_byte_default(void *self, uint32_t timeout_us);
void cfw_uart_send_byte_default(void *self, uint8_t byte);
