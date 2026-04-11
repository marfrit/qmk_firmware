// SPDX-License-Identifier: GPL-2.0-or-later
// CFW — Shared PS/2 ISR state machine
//
// The PS/2 receive logic is identical across platforms. Only the pin-read
// and ring-buffer-push mechanics differ. This header provides the common
// state machine as a static inline function, parameterized by:
//   - a data bit (0 or 1, sampled by the platform ISR)
//   - ISR state/data/parity fields (via a context struct)
//   - a push callback to put completed bytes into a ring buffer
//
// Each platform ISR:
//   1. Samples the data pin (platform-specific)
//   2. Calls cfw_ps2_isr_step() with the bit value
//   3. ring_push is handled via the callback
//
// This eliminates the duplicated state machine between AVR/STM32/etc.

#pragma once

#include <stdint.h>

// Embed this struct in your platform context as `cfw_ps2_isr_t isr`,
// then pass &ctx->isr to cfw_ps2_isr_step().
typedef struct {
    volatile uint8_t state;
    volatile uint8_t data;
    volatile uint8_t parity;
} cfw_ps2_isr_t;

// Process one clock edge. Returns the completed byte (0-255) or -1 if
// the frame is incomplete/invalid. The caller is responsible for pushing
// valid bytes into their platform-specific ring buffer.
static inline int16_t cfw_ps2_isr_step(cfw_ps2_isr_t *isr, uint8_t bit) {
    switch (isr->state) {
        case 0:  // Start bit (must be 0)
            if (bit != 0) {
                isr->state = 0;
                return -1;
            }
            isr->data = 0;
            isr->parity = 0;
            isr->state = 1;
            return -1;

        case 1: case 2: case 3: case 4:
        case 5: case 6: case 7: case 8:  // Data bits 0-7 (LSB first)
            isr->data >>= 1;
            if (bit) {
                isr->data |= 0x80;
                isr->parity++;
            }
            isr->state++;
            return -1;

        case 9:  // Parity bit (odd parity)
            if (bit) isr->parity++;
            if (!(isr->parity & 1)) {
                // Parity error — discard frame
                isr->state = 0;
                return -1;
            }
            isr->state = 10;
            return -1;

        case 10:  // Stop bit (must be 1)
        {
            uint8_t data = isr->data;
            isr->state = 0;
            if (bit) {
                return data;  // Valid frame
            }
            return -1;  // Bad stop bit
        }

        default:
            isr->state = 0;
            return -1;
    }
}
