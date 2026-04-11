// SPDX-License-Identifier: GPL-2.0-or-later
// CFW — AVR Platform Driver
// Pin I/O with open-collector emulation, timing via _delay_us/timer

#include "cfw_avr.h"
#include "gpio.h"
#include "timer.h"
#include "wait.h"
#include <avr/io.h>

// ---- Pin control ----
// AVR open-collector: output-low = drive low, input-pullup = release (high)

static void avr_pin_mode(cfw_pin_t pin, cfw_pin_mode_t mode) {
    switch (mode) {
        case CFW_PIN_INPUT:
            gpio_set_pin_input(pin);
            break;
        case CFW_PIN_INPUT_PULLUP:
            gpio_set_pin_input_high(pin);
            break;
        case CFW_PIN_OUTPUT:
            gpio_set_pin_output(pin);
            break;
        case CFW_PIN_OPEN_DRAIN:
            // Open-drain on AVR: set output value to 0, then
            // toggle between output (=driven low) and input-pullup (=released)
            gpio_write_pin_low(pin);
            gpio_set_pin_output(pin);
            break;
    }
}

static void avr_pin_write(cfw_pin_t pin, uint8_t value) {
    if (value) {
        // "High" for open-collector: release the line (input with pull-up)
        gpio_set_pin_input_high(pin);
    } else {
        // "Low" for open-collector: drive the line low
        gpio_write_pin_low(pin);
        gpio_set_pin_output(pin);
    }
}

static uint8_t avr_pin_read(cfw_pin_t pin) {
    return gpio_read_pin(pin) ? 1 : 0;
}

// ---- Timing ----

static void avr_delay_us(uint32_t us) {
    wait_us(us);
}

static uint32_t avr_micros(void) {
    // QMK uses Timer 0 for its millisecond counter.
    // Timer 0 runs at F_CPU/64 = 250kHz at 16MHz, so TCNT0
    // increments every 4us. We combine the ms counter with
    // TCNT0 for ~4us resolution — good enough for ADB bit
    // timing (35/65us cells) and general timeout calculations.
    //
    // We must read TCNT0 and the ms counter atomically, and
    // check for a pending overflow to avoid a race where TCNT0
    // has wrapped but the ISR hasn't incremented the ms counter yet.
    uint8_t sreg = SREG;
    cli();
    uint32_t ms = timer_read32();
    uint8_t tcnt = TCNT0;
    if ((TIFR0 & _BV(TOV0)) && tcnt < 128) {
        ms++;
    }
    SREG = sreg;
    return ms * 1000UL + (uint32_t)tcnt * (64UL * 1000000UL / F_CPU);
}

// ---- Edge detection ----
// Polling-based. The ISR handles the real-time edge detection;
// this is only used during send_byte when we wait for clock edges
// with the ISR disabled.

static bool avr_wait_edge(cfw_pin_t pin, cfw_edge_t edge, uint32_t timeout_us) {
    // Convert timeout to loop iterations. At 16MHz, each loop
    // iteration is roughly 0.5-1µs depending on optimization.
    // We use a generous loop count.
    uint16_t timeout_loops = (timeout_us > 65535) ? 65535 : (uint16_t)timeout_us;

    if (edge == CFW_EDGE_FALLING) {
        // Wait for pin to go high first (if it's already low)
        while (!gpio_read_pin(pin) && timeout_loops) timeout_loops--;
        // Now wait for it to go low
        while (gpio_read_pin(pin) && timeout_loops) timeout_loops--;
    } else if (edge == CFW_EDGE_RISING) {
        while (gpio_read_pin(pin) && timeout_loops) timeout_loops--;
        while (!gpio_read_pin(pin) && timeout_loops) timeout_loops--;
    }

    return timeout_loops > 0;
}

// ---- Platform instance ----

const cfw_platform_t cfw_avr_platform = {
    .pin_mode   = avr_pin_mode,
    .pin_write  = avr_pin_write,
    .pin_read   = avr_pin_read,
    .delay_us   = avr_delay_us,
    .micros     = avr_micros,
    .wait_edge  = avr_wait_edge,
};
