// SPDX-License-Identifier: GPL-2.0-or-later
// CFW — STM32 Wire Driver (ChibiOS PAL callback)
//
// ChibiOS version of the ISR-based PS/2 driver. Functionally identical
// to the AVR version but uses ChibiOS APIs:
//   - palEnableLineEvent() for edge-triggered interrupt (any GPIO pin)
//   - palSetLineCallback() for the ISR handler
//   - PAL_MODE_OUTPUT_OPENDRAIN for native open-drain (no hacks)
//   - chSysLockFromISR()/chSysUnlockFromISR() for ring buffer atomicity
//
// The ISR is the same bit-by-bit state machine as AVR. The send_byte
// function is also the same: inhibit, bit-bang, release. The only
// differences are the GPIO and interrupt APIs.
//
// WHY NOT USE CHIBIOS THREADS?
// We could run the protocol in a dedicated ChibiOS thread, sleeping
// on a semaphore that the PAL callback signals. This would be
// architecturally cleaner than polling the ring buffer. But:
//   1. It adds thread stack overhead (~256-512 bytes)
//   2. Thread context switching adds latency (~2-5µs)
//   3. The polling approach is proven and simple
//   4. QMK's main loop already runs at ~1kHz, so polling delay is <1ms
// For a future "advanced" mode, a thread-based approach could be added
// alongside the polling approach, selected at compile time.

#include "cfw_stm32.h"
#include "wait.h"
#include <string.h>

// ---- Global context for PAL callback ----
cfw_stm32_ctx_t *cfw_stm32_active_ctx = NULL;

// ---- Ring buffer helpers ----

static inline void ring_push(cfw_stm32_ctx_t *ctx, uint8_t data) {
    uint8_t next = (ctx->ring_head + 1) & (CFW_STM32_RING_SIZE - 1);
    if (next != ctx->ring_tail) {
        ctx->ring[ctx->ring_head] = data;
        ctx->ring_head = next;
    }
}

static inline uint8_t ring_pop(cfw_stm32_ctx_t *ctx) {
    if (ctx->ring_head == ctx->ring_tail) return 0;
    uint8_t data = ctx->ring[ctx->ring_tail];
    ctx->ring_tail = (ctx->ring_tail + 1) & (CFW_STM32_RING_SIZE - 1);
    return data;
}

static inline bool ring_has_data(cfw_stm32_ctx_t *ctx) {
    return ctx->ring_head != ctx->ring_tail;
}

// ---- PAL Callback (ISR context) ----
//
// ChibiOS PAL event system:
//   palEnableLineEvent(pin, PAL_EVENT_MODE_FALLING_EDGE)
//   palSetLineCallback(pin, callback, arg)
//
// The callback fires in ISR context whenever the configured edge
// occurs on the pin. Unlike AVR where you need specific INT pins,
// STM32's EXTI system works on ANY GPIO pin. The only limitation:
// two pins with the same number on different ports (e.g., PA1 and
// PB1) share the same EXTI line and can't both have callbacks.
// For a converter with one clock pin, this is never a problem.
//
// The callback receives a void* argument, which we use to pass
// the context pointer. This avoids the global-pointer pattern
// (though we keep it for compatibility).

static void cfw_stm32_clock_callback(void *arg) {
    cfw_stm32_ctx_t *ctx = (cfw_stm32_ctx_t *)arg;
    if (!ctx) ctx = cfw_stm32_active_ctx;
    if (!ctx) return;

    // Sample data pin
    uint8_t bit = palReadLine(ctx->data_pin) == PAL_HIGH ? 1 : 0;

    switch (ctx->isr_state) {
        case 0:  // Start bit
            if (bit != 0) { ctx->isr_state = 0; return; }
            ctx->isr_data = 0;
            ctx->isr_parity = 0;
            ctx->isr_state = 1;
            break;

        case 1: case 2: case 3: case 4:
        case 5: case 6: case 7: case 8:  // Data bits
            ctx->isr_data >>= 1;
            if (bit) {
                ctx->isr_data |= 0x80;
                ctx->isr_parity++;
            }
            ctx->isr_state++;
            break;

        case 9:  // Parity
            if (bit) ctx->isr_parity++;
            if (!(ctx->isr_parity & 1)) { ctx->isr_state = 0; return; }
            ctx->isr_state = 10;
            break;

        case 10:  // Stop bit
            if (bit) {
                chSysLockFromISR();
                ring_push(ctx, ctx->isr_data);
                chSysUnlockFromISR();
            }
            ctx->isr_state = 0;
            break;

        default:
            ctx->isr_state = 0;
            break;
    }
}

// ---- Open-drain helpers ----
// STM32 has native open-drain so these are clean.

static inline void oc_lo(cfw_pin_t pin) {
    palSetLineMode(pin, PAL_MODE_OUTPUT_OPENDRAIN);
    palWriteLine(pin, PAL_LOW);
}

static inline void oc_hi(cfw_pin_t pin) {
    palSetLineMode(pin, PAL_MODE_OUTPUT_OPENDRAIN);
    palWriteLine(pin, PAL_HIGH);
}

static inline void stm32_idle(cfw_stm32_ctx_t *ctx) {
    oc_hi(ctx->clock_pin);
    oc_hi(ctx->data_pin);
}

// ---- Wait helpers ----

static bool wait_clock_lo(cfw_stm32_ctx_t *ctx, uint16_t timeout_us) {
    while (timeout_us--) {
        if (palReadLine(ctx->clock_pin) == PAL_LOW) return true;
        wait_us(1);
    }
    return false;
}

static bool wait_clock_hi(cfw_stm32_ctx_t *ctx, uint16_t timeout_us) {
    while (timeout_us--) {
        if (palReadLine(ctx->clock_pin) == PAL_HIGH) return true;
        wait_us(1);
    }
    return false;
}

static bool wait_data_lo(cfw_stm32_ctx_t *ctx, uint16_t timeout_us) {
    while (timeout_us--) {
        if (palReadLine(ctx->data_pin) == PAL_LOW) return true;
        wait_us(1);
    }
    return false;
}

static bool wait_data_hi(cfw_stm32_ctx_t *ctx, uint16_t timeout_us) {
    while (timeout_us--) {
        if (palReadLine(ctx->data_pin) == PAL_HIGH) return true;
        wait_us(1);
    }
    return false;
}

// ---- Interrupt enable/disable ----

static void int_on(cfw_stm32_ctx_t *ctx) {
    palEnableLineEvent(ctx->clock_pin, PAL_EVENT_MODE_FALLING_EDGE);
    palSetLineCallback(ctx->clock_pin, cfw_stm32_clock_callback, ctx);
}

static void int_off(cfw_stm32_ctx_t *ctx) {
    palDisableLineEvent(ctx->clock_pin);
}

// ---- Wire interface: recv_byte ----

static int16_t stm32_recv_byte(void *self, uint32_t timeout_us) {
    cfw_stm32_ctx_t *ctx = (cfw_stm32_ctx_t *)((cfw_wire_clock_data_t *)self)->platform_ctx;
    uint16_t timeout_ms = (timeout_us + 999) / 1000;
    if (timeout_ms == 0) timeout_ms = 1;

    while (timeout_ms--) {
        chSysLock();
        bool has_data = ring_has_data(ctx);
        uint8_t data = has_data ? ring_pop(ctx) : 0;
        chSysUnlock();

        if (has_data) return (int16_t)data;
        wait_ms(1);
    }

    return CFW_ERR_TIMEOUT;
}

// ---- Wire interface: send_byte ----
// Same protocol as AVR: inhibit, bit-bang, re-enable callback.

static int16_t stm32_send_byte(void *self, uint8_t data) {
    cfw_stm32_ctx_t *ctx = (cfw_stm32_ctx_t *)((cfw_wire_clock_data_t *)self)->platform_ctx;
    bool parity = true;

    int_off(ctx);

    // Inhibit
    oc_lo(ctx->clock_pin);
    wait_us(100);

    // Request-to-send
    oc_lo(ctx->data_pin);
    oc_hi(ctx->clock_pin);
    if (!wait_clock_lo(ctx, 10000)) goto error;

    // Data bits
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

    // Parity
    wait_us(15);
    if (parity) oc_hi(ctx->data_pin);
    else oc_lo(ctx->data_pin);
    if (!wait_clock_hi(ctx, 50)) goto error;
    if (!wait_clock_lo(ctx, 50)) goto error;

    // Stop
    wait_us(15);
    oc_hi(ctx->data_pin);

    // ACK
    if (!wait_data_lo(ctx, 50)) goto error;
    if (!wait_clock_lo(ctx, 50)) goto error;
    if (!wait_clock_hi(ctx, 50)) goto error;
    if (!wait_data_hi(ctx, 50)) goto error;

    stm32_idle(ctx);
    int_on(ctx);
    return stm32_recv_byte(self, 25000);

error:
    stm32_idle(ctx);
    int_on(ctx);
    return CFW_ERR_TIMEOUT;
}

// ---- Wire interface: inhibit/release ----

static void stm32_inhibit(void *self) {
    cfw_stm32_ctx_t *ctx = (cfw_stm32_ctx_t *)((cfw_wire_clock_data_t *)self)->platform_ctx;
    int_off(ctx);
    oc_lo(ctx->clock_pin);
}

static void stm32_release(void *self) {
    cfw_stm32_ctx_t *ctx = (cfw_stm32_ctx_t *)((cfw_wire_clock_data_t *)self)->platform_ctx;
    stm32_idle(ctx);
    int_on(ctx);
}

// ---- Bit-level interface ----

static int8_t stm32_recv_bit(void *self, uint32_t timeout_us) {
    cfw_stm32_ctx_t *ctx = (cfw_stm32_ctx_t *)((cfw_wire_clock_data_t *)self)->platform_ctx;
    if (!wait_clock_lo(ctx, (uint16_t)(timeout_us > 65535 ? 65535 : timeout_us)))
        return CFW_ERR_TIMEOUT;
    uint8_t bit = palReadLine(ctx->data_pin) == PAL_HIGH ? 1 : 0;
    if (!wait_clock_hi(ctx, 50)) return CFW_ERR_TIMEOUT;
    return bit;
}

static void stm32_send_bit(void *self, uint8_t bit) {
    cfw_stm32_ctx_t *ctx = (cfw_stm32_ctx_t *)((cfw_wire_clock_data_t *)self)->platform_ctx;
    if (bit) oc_hi(ctx->data_pin);
    else oc_lo(ctx->data_pin);
    wait_clock_lo(ctx, 15000);
    wait_clock_hi(ctx, 15000);
}

static uint8_t stm32_clock_state(void *self) {
    cfw_stm32_ctx_t *ctx = (cfw_stm32_ctx_t *)((cfw_wire_clock_data_t *)self)->platform_ctx;
    return palReadLine(ctx->clock_pin) == PAL_HIGH ? 1 : 0;
}

static uint8_t stm32_data_state(void *self) {
    cfw_stm32_ctx_t *ctx = (cfw_stm32_ctx_t *)((cfw_wire_clock_data_t *)self)->platform_ctx;
    return palReadLine(ctx->data_pin) == PAL_HIGH ? 1 : 0;
}

// ---- Initialization ----

void cfw_stm32_init(cfw_stm32_ctx_t *ctx, cfw_pin_t data_pin, cfw_pin_t clock_pin) {
    memset(ctx, 0, sizeof(*ctx));
    ctx->data_pin = data_pin;
    ctx->clock_pin = clock_pin;
    cfw_stm32_active_ctx = ctx;

    // Configure pins as open-drain, released
    stm32_idle(ctx);

    // Enable falling edge interrupt on clock pin
    int_on(ctx);
}

void cfw_stm32_wire_init(cfw_wire_t *wire, cfw_stm32_ctx_t *ctx) {
    wire->type = CFW_WIRE_CLOCK_DATA;
    wire->clock_data.clock_pin = ctx->clock_pin;
    wire->clock_data.data_pin = ctx->data_pin;
    wire->clock_data.hw = &cfw_stm32_platform;
    wire->clock_data.platform_ctx = ctx;

    wire->clock_data.recv_byte = stm32_recv_byte;
    wire->clock_data.send_byte = stm32_send_byte;
    wire->clock_data.inhibit = stm32_inhibit;
    wire->clock_data.release = stm32_release;

    wire->clock_data.recv_bit = stm32_recv_bit;
    wire->clock_data.send_bit = stm32_send_bit;
    wire->clock_data.clock_state = stm32_clock_state;
    wire->clock_data.data_state = stm32_data_state;
}
