// SPDX-License-Identifier: GPL-2.0-or-later
// CFW — RP2040 Platform Driver
// Pin I/O, timing, and edge detection using Pico SDK
//
// This implements cfw_platform_t for the RP2040. It's the "boring" layer —
// basic GPIO and timing that any microcontroller can do. The interesting
// RP2040-specific stuff (PIO, DMA, multicore) is in the other files.

#include "cfw_rp2040.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"

// ---- Pin control ----

static void rp2040_pin_mode(cfw_pin_t pin, cfw_pin_mode_t mode) {
    gpio_init(pin);
    switch (mode) {
        case CFW_PIN_INPUT:
            gpio_set_dir(pin, GPIO_IN);
            gpio_disable_pulls(pin);
            break;
        case CFW_PIN_INPUT_PULLUP:
            gpio_set_dir(pin, GPIO_IN);
            gpio_pull_up(pin);
            break;
        case CFW_PIN_OUTPUT:
            gpio_set_dir(pin, GPIO_OUT);
            break;
        case CFW_PIN_OPEN_DRAIN:
            // RP2040 doesn't have true open-drain. We simulate it by
            // toggling direction: output-low = driven low, input = high-Z
            // (with external pull-up, this acts as open-drain).
            // The PIO programs handle this with inverted output-enable
            // (PAL_RP_IOCTRL_OEOVER_DRVINVPERI), which is more elegant
            // but only works for PIO-controlled pins.
            gpio_set_dir(pin, GPIO_OUT);
            gpio_put(pin, 0);  // output value always 0
            break;
    }
}

static void rp2040_pin_write(cfw_pin_t pin, uint8_t value) {
    gpio_put(pin, value);
}

static uint8_t rp2040_pin_read(cfw_pin_t pin) {
    return gpio_get(pin) ? 1 : 0;
}

// ---- Timing ----

static void rp2040_delay_us(uint32_t us) {
    busy_wait_us_32(us);
}

static uint32_t rp2040_micros(void) {
    // RP2040 has a 64-bit microsecond timer. We return the low 32 bits,
    // which wraps every ~71 minutes. For timeout calculations using
    // subtraction, this is fine (unsigned overflow is defined behavior).
    return (uint32_t)time_us_64();
}

// ---- Edge detection ----
// The RP2040 GPIO block supports edge-triggered IRQs, but for the
// platform driver we use a simple polling approach. The PIO handles
// the timing-critical edge detection in hardware — this is only used
// as a fallback for non-PIO wire protocols.

static bool rp2040_wait_edge(cfw_pin_t pin, cfw_edge_t edge, uint32_t timeout_us) {
    uint32_t start = rp2040_micros();
    uint8_t target_state = (edge == CFW_EDGE_FALLING) ? 0 : 1;

    // Wait for pin to be in the opposite state first
    if (edge != CFW_EDGE_ANY) {
        while (gpio_get(pin) == target_state) {
            if ((rp2040_micros() - start) >= timeout_us) return false;
        }
    }

    // Now wait for the edge
    while (true) {
        if (edge == CFW_EDGE_FALLING && !gpio_get(pin)) return true;
        if (edge == CFW_EDGE_RISING && gpio_get(pin)) return true;
        if (edge == CFW_EDGE_ANY) {
            // Detect any change — sample, wait, compare
            uint8_t initial = gpio_get(pin);
            while (gpio_get(pin) == initial) {
                if ((rp2040_micros() - start) >= timeout_us) return false;
            }
            return true;
        }
        if ((rp2040_micros() - start) >= timeout_us) return false;
    }
}

// ---- Platform instance ----

const cfw_platform_t cfw_rp2040_platform = {
    .pin_mode   = rp2040_pin_mode,
    .pin_write  = rp2040_pin_write,
    .pin_read   = rp2040_pin_read,
    .delay_us   = rp2040_delay_us,
    .micros     = rp2040_micros,
    .wait_edge  = rp2040_wait_edge,
};
