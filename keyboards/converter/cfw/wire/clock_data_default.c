// SPDX-License-Identifier: GPL-2.0-or-later
// CFW — Default clock/data wire implementation
// Generic bit-banged PS/2-style framing using platform pin_read + timing
// Works on any platform. PIO/timer backends override for better performance.

#include "cfw_wire.h"

// --- Helpers ---

static inline cfw_wire_clock_data_t *cd(void *self) {
    return (cfw_wire_clock_data_t *)self;
}

static inline const cfw_platform_t *hw(void *self) {
    return cd(self)->hw;
}

// Wait for clock to reach a specific state, with timeout
static bool wait_clock(void *self, uint8_t state, uint32_t timeout_us) {
    cfw_wire_clock_data_t *w = cd(self);

    // Use hardware edge detection if available
    if (w->hw->wait_edge && state == 0) {
        return w->hw->wait_edge(w->clock_pin, CFW_EDGE_FALLING, timeout_us);
    }
    if (w->hw->wait_edge && state == 1) {
        return w->hw->wait_edge(w->clock_pin, CFW_EDGE_RISING, timeout_us);
    }

    // Fallback: poll
    uint32_t start = w->hw->micros();
    while (w->hw->pin_read(w->clock_pin) != state) {
        if ((w->hw->micros() - start) >= timeout_us) return false;
    }
    return true;
}

// --- Bus control ---

void cfw_clock_data_inhibit_default(void *self) {
    cfw_wire_clock_data_t *w = cd(self);
    w->hw->pin_mode(w->clock_pin, CFW_PIN_OUTPUT);
    w->hw->pin_write(w->clock_pin, 0);  // pull clock low
}

void cfw_clock_data_release_default(void *self) {
    cfw_wire_clock_data_t *w = cd(self);
    w->hw->pin_mode(w->clock_pin, CFW_PIN_INPUT_PULLUP);
    w->hw->pin_mode(w->data_pin, CFW_PIN_INPUT_PULLUP);
}

// --- Bit-level ---

int8_t cfw_clock_data_recv_bit_default(void *self, uint32_t timeout_us) {
    cfw_wire_clock_data_t *w = cd(self);

    // Wait for clock falling edge
    if (!wait_clock(self, 0, timeout_us)) return CFW_ERR_TIMEOUT;

    // Sample data on falling edge of clock
    uint8_t bit = w->hw->pin_read(w->data_pin);

    // Wait for clock to go high again
    if (!wait_clock(self, 1, timeout_us)) return CFW_ERR_TIMEOUT;

    return bit;
}

void cfw_clock_data_send_bit_default(void *self, uint8_t bit) {
    cfw_wire_clock_data_t *w = cd(self);

    // Set data line
    w->hw->pin_mode(w->data_pin, CFW_PIN_OUTPUT);
    w->hw->pin_write(w->data_pin, bit ? 1 : 0);

    // Wait for keyboard to clock it in (falling edge)
    wait_clock(self, 0, 15000);
    wait_clock(self, 1, 15000);
}

// --- Byte-level: PS/2 framing (11 bits: start=0, 8 data LSB, odd parity, stop=1) ---

int16_t cfw_clock_data_recv_byte_default(void *self, uint32_t timeout_us) {
    int8_t bit;
    uint8_t data = 0;
    uint8_t parity_count = 0;

    // Start bit (must be 0)
    bit = cfw_clock_data_recv_bit_default(self, timeout_us);
    if (bit < 0) return bit;  // timeout or error
    if (bit != 0) return CFW_ERR_FRAME;

    // 8 data bits, LSB first
    for (int i = 0; i < 8; i++) {
        bit = cfw_clock_data_recv_bit_default(self, 2000);
        if (bit < 0) return bit;
        if (bit) {
            data |= (1 << i);
            parity_count++;
        }
    }

    // Parity bit (odd parity)
    bit = cfw_clock_data_recv_bit_default(self, 2000);
    if (bit < 0) return bit;
    if (bit) parity_count++;
    if ((parity_count & 1) == 0) return CFW_ERR_PARITY;

    // Stop bit (must be 1)
    bit = cfw_clock_data_recv_bit_default(self, 2000);
    if (bit < 0) return bit;
    if (bit != 1) return CFW_ERR_FRAME;

    return (int16_t)data;
}

int16_t cfw_clock_data_send_byte_default(void *self, uint8_t byte) {
    cfw_wire_clock_data_t *w = cd(self);
    uint8_t parity = 1;  // odd parity

    // Request-to-send: pull clock low for >100us, then pull data low, release clock
    w->hw->pin_mode(w->clock_pin, CFW_PIN_OUTPUT);
    w->hw->pin_write(w->clock_pin, 0);
    w->hw->delay_us(150);

    w->hw->pin_mode(w->data_pin, CFW_PIN_OUTPUT);
    w->hw->pin_write(w->data_pin, 0);  // start bit

    // Release clock — keyboard will start clocking
    w->hw->pin_mode(w->clock_pin, CFW_PIN_INPUT_PULLUP);

    // Wait for keyboard to pull clock low
    if (!wait_clock(self, 0, 15000)) {
        cfw_clock_data_release_default(self);
        return CFW_ERR_TIMEOUT;
    }

    // Clock goes high — start bit consumed. Now 8 data bits.
    wait_clock(self, 1, 2000);

    for (int i = 0; i < 8; i++) {
        uint8_t bit = (byte >> i) & 1;
        parity ^= bit;

        // Set data, wait for clock cycle
        w->hw->pin_write(w->data_pin, bit);
        if (!wait_clock(self, 0, 2000)) { cfw_clock_data_release_default(self); return CFW_ERR_TIMEOUT; }
        wait_clock(self, 1, 2000);
    }

    // Parity bit
    w->hw->pin_write(w->data_pin, parity);
    if (!wait_clock(self, 0, 2000)) { cfw_clock_data_release_default(self); return CFW_ERR_TIMEOUT; }
    wait_clock(self, 1, 2000);

    // Release data for stop bit and ACK
    w->hw->pin_mode(w->data_pin, CFW_PIN_INPUT_PULLUP);

    // Stop bit (keyboard clocks it)
    if (!wait_clock(self, 0, 2000)) { cfw_clock_data_release_default(self); return CFW_ERR_TIMEOUT; }

    // ACK: keyboard pulls data low
    uint8_t ack = w->hw->pin_read(w->data_pin);
    wait_clock(self, 1, 2000);

    if (ack != 0) return CFW_ERR_NAK;

    return CFW_OK;
}
