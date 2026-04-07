// SPDX-License-Identifier: GPL-2.0-or-later
// CFW — AVR Wire Driver (ISR-based PS/2)
//
// This implements cfw_wire_clock_data_t for AVR using the classic approach:
//   - External interrupt on clock falling edge
//   - ISR assembles bytes one bit at a time
//   - Ring buffer bridges ISR → main loop
//   - Sending inhibits bus and bit-bangs with edge polling
//
// This is essentially QMK's ps2_interrupt.c refactored into the CFW
// wire interface, so the same converter code runs on AVR, RP2040, or STM32.
//
// NOTE ON THE "ASSEMBLY TRICK":
// Some AVR converter implementations use inline assembly for the ISR
// to shave cycles off the bit-sampling path. At 16MHz with a 10-16kHz
// PS/2 clock, each clock half-cycle is 500-800 CPU cycles — plenty for
// C code. The assembly trick matters when you're also running USB
// interrupts that eat into the budget. We use C here for portability;
// if a specific board needs the assembly version, it can override
// recv_byte in the wire driver struct.

#include "cfw_avr.h"
#include "gpio.h"
#include "wait.h"
#include <string.h>

// ---- Global context for ISR access ----
cfw_avr_ctx_t *cfw_avr_active_ctx = NULL;

// ---- Ring buffer helpers (ISR-safe) ----

static inline void ring_push(cfw_avr_ctx_t *ctx, uint8_t data) {
    uint8_t next = (ctx->ring_head + 1) & (CFW_AVR_RING_SIZE - 1);
    if (next != ctx->ring_tail) {  // not full
        ctx->ring[ctx->ring_head] = data;
        ctx->ring_head = next;
    }
    // If full, drop the byte. This shouldn't happen at keyboard speeds
    // unless the main loop is stuck for >32ms.
}

static inline uint8_t ring_pop(cfw_avr_ctx_t *ctx) {
    if (ctx->ring_head == ctx->ring_tail) return 0;  // empty
    uint8_t data = ctx->ring[ctx->ring_tail];
    ctx->ring_tail = (ctx->ring_tail + 1) & (CFW_AVR_RING_SIZE - 1);
    return data;
}

static inline bool ring_has_data(cfw_avr_ctx_t *ctx) {
    return ctx->ring_head != ctx->ring_tail;
}

static inline void ring_clear(cfw_avr_ctx_t *ctx) {
    ctx->ring_head = ctx->ring_tail = 0;
}

// ---- Open-collector pin helpers ----
// Duplicated from ps2_io.c pattern — drive low or release to pull-up.
// We use these during send_byte when the ISR is disabled and we
// need manual control of both lines.

static inline void oc_lo(cfw_pin_t pin) {
    gpio_write_pin_low(pin);
    gpio_set_pin_output(pin);
}

static inline void oc_hi(cfw_pin_t pin) {
    gpio_set_pin_input_high(pin);
}

static inline bool oc_read(cfw_pin_t pin) {
    gpio_set_pin_input_high(pin);
    wait_us(1);  // settle time after switching to input
    return gpio_read_pin(pin);
}

// ---- Wait helpers for send_byte ----
// These poll with timeout, used when ISR is disabled during transmission.

static bool wait_clock_lo(cfw_avr_ctx_t *ctx, uint16_t timeout_us) {
    while (timeout_us--) {
        if (!gpio_read_pin(ctx->clock_pin)) return true;
        wait_us(1);
    }
    return false;
}

static bool wait_clock_hi(cfw_avr_ctx_t *ctx, uint16_t timeout_us) {
    while (timeout_us--) {
        if (gpio_read_pin(ctx->clock_pin)) return true;
        wait_us(1);
    }
    return false;
}

static bool wait_data_lo(cfw_avr_ctx_t *ctx, uint16_t timeout_us) {
    while (timeout_us--) {
        if (!gpio_read_pin(ctx->data_pin)) return true;
        wait_us(1);
    }
    return false;
}

static bool wait_data_hi(cfw_avr_ctx_t *ctx, uint16_t timeout_us) {
    while (timeout_us--) {
        if (gpio_read_pin(ctx->data_pin)) return true;
        wait_us(1);
    }
    return false;
}

// ---- ISR: Clock Falling Edge Handler ----
//
// This fires on every falling edge of the PS/2 clock line.
// It samples the data line and assembles a byte over 11 edges
// (start + 8 data + parity + stop).
//
// TIMING BUDGET:
// At 16MHz, one clock cycle = 62.5ns. A PS/2 clock half-period
// is 30-50µs (at 10-16.7 kHz), giving us 480-800 CPU cycles per
// ISR invocation. Reading a GPIO pin and doing a shift/compare
// takes ~20-30 cycles in C. Plenty of margin.
//
// The ISR must NOT call any non-reentrant functions or access
// shared state without cli/sei. We only touch volatile ctx fields
// and the ring buffer (single-producer/single-consumer, lock-free
// by design since ISR is the only writer and main loop is the only
// reader).

void cfw_avr_clock_isr(void) {
    cfw_avr_ctx_t *ctx = cfw_avr_active_ctx;
    if (!ctx) return;

    // Double-check: only process on falling edge
    // (Some AVR interrupt configs fire on both edges)
    if (gpio_read_pin(ctx->clock_pin)) return;

    uint8_t bit = gpio_read_pin(ctx->data_pin) ? 1 : 0;

    switch (ctx->isr_state) {
        case 0:  // Start bit (must be 0)
            if (bit != 0) {
                // Bad start bit — noise or misalignment. Reset.
                ctx->isr_state = 0;
                return;
            }
            ctx->isr_data = 0;
            ctx->isr_parity = 0;
            ctx->isr_state = 1;
            break;

        case 1: case 2: case 3: case 4:
        case 5: case 6: case 7: case 8:  // Data bits 0-7 (LSB first)
            ctx->isr_data >>= 1;
            if (bit) {
                ctx->isr_data |= 0x80;
                ctx->isr_parity++;
            }
            ctx->isr_state++;
            break;

        case 9:  // Parity bit (odd parity)
            if (bit) ctx->isr_parity++;
            if (!(ctx->isr_parity & 1)) {
                // Parity error — discard
                ctx->isr_state = 0;
                return;
            }
            ctx->isr_state = 10;
            break;

        case 10:  // Stop bit (must be 1)
            if (bit) {
                // Valid frame — push to ring buffer
                ring_push(ctx, ctx->isr_data);
            }
            // Reset for next frame (whether valid or not)
            ctx->isr_state = 0;
            break;

        default:
            ctx->isr_state = 0;
            break;
    }
}

// ---- ISR Vector ----
// Which interrupt vector to use depends on the board config.
// Pro Micro typically uses INT0 (PD0) or INT1 (PD1) for the clock pin.
// This is configured via PS2_INT_VECT in config.h.
//
// If using QMK's existing interrupt infrastructure, the ISR is
// registered via the PS2_INT_VECT macro. For standalone CFW, we
// provide a generic hook that the board config connects.

#if defined(CFW_AVR_INT_VECT)
ISR(CFW_AVR_INT_VECT) {
    cfw_avr_clock_isr();
}
#endif

// ---- Interrupt enable/disable ----
// Board-specific. These macros should be defined in the keyboard's
// config.h based on which pin the clock line is connected to.
//
// Example for Pro Micro with clock on PD1 (INT1):
//   #define CFW_AVR_INT_VECT    INT1_vect
//   #define CFW_AVR_INT_INIT()  do { EICRA |= (1<<ISC11); } while(0)  // falling edge
//   #define CFW_AVR_INT_ON()    do { EIMSK |= (1<<INT1); } while(0)
//   #define CFW_AVR_INT_OFF()   do { EIMSK &= ~(1<<INT1); } while(0)

#ifndef CFW_AVR_INT_INIT
#define CFW_AVR_INT_INIT()  // defined by board config
#endif
#ifndef CFW_AVR_INT_ON
#define CFW_AVR_INT_ON()
#endif
#ifndef CFW_AVR_INT_OFF
#define CFW_AVR_INT_OFF()
#endif

// ---- Wire interface: idle (release both lines) ----
static void avr_idle(cfw_avr_ctx_t *ctx) {
    oc_hi(ctx->clock_pin);
    oc_hi(ctx->data_pin);
}

// ---- Wire interface: recv_byte ----
// Reads from the ISR ring buffer. The ISR does the actual bit sampling;
// we just wait for a complete byte to appear.
static int16_t avr_recv_byte(void *self, uint32_t timeout_us) {
    cfw_avr_ctx_t *ctx = (cfw_avr_ctx_t *)((cfw_wire_clock_data_t *)self)->platform_ctx;

    // Convert to ms for the poll loop (AVR timer is ms resolution)
    uint16_t timeout_ms = (timeout_us + 999) / 1000;
    if (timeout_ms == 0) timeout_ms = 1;

    while (timeout_ms--) {
        uint8_t sreg = SREG;
        cli();
        bool has_data = ring_has_data(ctx);
        uint8_t data = has_data ? ring_pop(ctx) : 0;
        SREG = sreg;

        if (has_data) return (int16_t)data;
        wait_ms(1);
    }

    return CFW_ERR_TIMEOUT;
}

// ---- Wire interface: send_byte ----
// Disables ISR, takes control of the bus, bit-bangs the frame,
// waits for ACK, then re-enables ISR.
//
// This follows the standard PS/2 host-to-device protocol:
//   1. Pull clock low for >100µs (request-to-send)
//   2. Pull data low (start bit)
//   3. Release clock — keyboard will start clocking
//   4. On each falling clock edge, set next data bit
//   5. After 8 data + parity, release data (stop bit)
//   6. Keyboard pulls data low for ACK
//
// NOTE: This DOES inhibit the bus during transmission.
// The RP2040 PIO backend avoids inhibiting (reply tagging), but on
// AVR we don't have the luxury of hardware-tagged frames. The
// keyboard can't send during our transmission. This is standard
// PS/2 behavior and all keyboards handle it correctly — even the
// elderly ones with methusalem crystals. The inhibit period is
// short (~2ms for a full byte) and bounded.

static int16_t avr_send_byte(void *self, uint8_t data) {
    cfw_avr_ctx_t *ctx = (cfw_avr_ctx_t *)((cfw_wire_clock_data_t *)self)->platform_ctx;
    bool parity = true;

    CFW_AVR_INT_OFF();

    // Inhibit: pull clock low
    oc_lo(ctx->clock_pin);
    wait_us(100);

    // Request-to-send: pull data low, release clock
    oc_lo(ctx->data_pin);
    oc_hi(ctx->clock_pin);
    if (!wait_clock_lo(ctx, 10000)) goto error;  // keyboard should pull clock low

    // Data bits (LSB first)
    for (uint8_t i = 0; i < 8; i++) {
        if (data & (1 << i)) {
            parity = !parity;
            oc_hi(ctx->data_pin);
        } else {
            oc_lo(ctx->data_pin);
        }
        if (!wait_clock_hi(ctx, 50)) goto error;
        if (!wait_clock_lo(ctx, 50)) goto error;
    }

    // Parity bit
    wait_us(15);
    if (parity) {
        oc_hi(ctx->data_pin);
    } else {
        oc_lo(ctx->data_pin);
    }
    if (!wait_clock_hi(ctx, 50)) goto error;
    if (!wait_clock_lo(ctx, 50)) goto error;

    // Stop bit
    wait_us(15);
    oc_hi(ctx->data_pin);

    // ACK from keyboard
    if (!wait_data_lo(ctx, 50)) goto error;
    if (!wait_clock_lo(ctx, 50)) goto error;

    // Wait for idle
    if (!wait_clock_hi(ctx, 50)) goto error;
    if (!wait_data_hi(ctx, 50)) goto error;

    avr_idle(ctx);
    CFW_AVR_INT_ON();

    // Wait for response (ACK byte = 0xFA, or error)
    return avr_recv_byte(self, 25000);  // 25ms timeout for response

error:
    avr_idle(ctx);
    CFW_AVR_INT_ON();
    return CFW_ERR_TIMEOUT;
}

// ---- Wire interface: inhibit/release ----
static void avr_inhibit(void *self) {
    cfw_avr_ctx_t *ctx = (cfw_avr_ctx_t *)((cfw_wire_clock_data_t *)self)->platform_ctx;
    CFW_AVR_INT_OFF();
    oc_lo(ctx->clock_pin);
}

static void avr_release(void *self) {
    cfw_avr_ctx_t *ctx = (cfw_avr_ctx_t *)((cfw_wire_clock_data_t *)self)->platform_ctx;
    avr_idle(ctx);
    CFW_AVR_INT_ON();
}

// ---- Bit-level interface (escape hatch) ----
// Available for protocols that need non-standard framing.
// Disables ISR and manually clocks bits.

static int8_t avr_recv_bit(void *self, uint32_t timeout_us) {
    cfw_avr_ctx_t *ctx = (cfw_avr_ctx_t *)((cfw_wire_clock_data_t *)self)->platform_ctx;

    if (!wait_clock_lo(ctx, (uint16_t)(timeout_us > 65535 ? 65535 : timeout_us)))
        return CFW_ERR_TIMEOUT;

    uint8_t bit = gpio_read_pin(ctx->data_pin) ? 1 : 0;

    if (!wait_clock_hi(ctx, 50))
        return CFW_ERR_TIMEOUT;

    return bit;
}

static void avr_send_bit(void *self, uint8_t bit) {
    cfw_avr_ctx_t *ctx = (cfw_avr_ctx_t *)((cfw_wire_clock_data_t *)self)->platform_ctx;

    if (bit) {
        oc_hi(ctx->data_pin);
    } else {
        oc_lo(ctx->data_pin);
    }
    wait_clock_lo(ctx, 15000);
    wait_clock_hi(ctx, 15000);
}

static uint8_t avr_clock_state(void *self) {
    cfw_avr_ctx_t *ctx = (cfw_avr_ctx_t *)((cfw_wire_clock_data_t *)self)->platform_ctx;
    return gpio_read_pin(ctx->clock_pin) ? 1 : 0;
}

static uint8_t avr_data_state(void *self) {
    cfw_avr_ctx_t *ctx = (cfw_avr_ctx_t *)((cfw_wire_clock_data_t *)self)->platform_ctx;
    return gpio_read_pin(ctx->data_pin) ? 1 : 0;
}

// ---- Initialization ----

void cfw_avr_init(cfw_avr_ctx_t *ctx, cfw_pin_t data_pin, cfw_pin_t clock_pin) {
    memset(ctx, 0, sizeof(*ctx));
    ctx->data_pin = data_pin;
    ctx->clock_pin = clock_pin;

    // Set global context for ISR access
    cfw_avr_active_ctx = ctx;

    // Release both lines (input with pull-up)
    avr_idle(ctx);

    // Configure external interrupt on clock falling edge
    CFW_AVR_INT_INIT();
    CFW_AVR_INT_ON();
}

void cfw_avr_wire_init(cfw_wire_t *wire, cfw_avr_ctx_t *ctx) {
    wire->type = CFW_WIRE_CLOCK_DATA;
    wire->clock_data.clock_pin = ctx->clock_pin;
    wire->clock_data.data_pin = ctx->data_pin;
    wire->clock_data.hw = &cfw_avr_platform;
    wire->clock_data.platform_ctx = ctx;

    // ISR-based implementations
    wire->clock_data.recv_byte = avr_recv_byte;
    wire->clock_data.send_byte = avr_send_byte;
    wire->clock_data.inhibit = avr_inhibit;
    wire->clock_data.release = avr_release;

    // Bit-level interface (for non-PS/2 protocols like ADB/M0110)
    wire->clock_data.recv_bit = avr_recv_bit;
    wire->clock_data.send_bit = avr_send_bit;
    wire->clock_data.clock_state = avr_clock_state;
    wire->clock_data.data_state = avr_data_state;
}
