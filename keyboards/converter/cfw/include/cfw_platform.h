// SPDX-License-Identifier: GPL-2.0-or-later
// CFW — Platform driver interface
// Lowest layer: pin I/O, timing, edge detection
// Implemented per-platform: PIO (RP2040), ChibiOS (STM32), AVR, Linux GPIO
#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef uint32_t cfw_pin_t;

typedef enum {
    CFW_PIN_INPUT,
    CFW_PIN_INPUT_PULLUP,
    CFW_PIN_OUTPUT,
    CFW_PIN_OPEN_DRAIN,
} cfw_pin_mode_t;

typedef enum {
    CFW_EDGE_FALLING,
    CFW_EDGE_RISING,
    CFW_EDGE_ANY,
} cfw_edge_t;

typedef struct {
    // --- Pin control ---
    void (*pin_mode)(cfw_pin_t pin, cfw_pin_mode_t mode);
    void (*pin_write)(cfw_pin_t pin, uint8_t value);
    uint8_t (*pin_read)(cfw_pin_t pin);

    // --- Timing ---
    void (*delay_us)(uint32_t us);
    uint32_t (*micros)(void);   // microsecond counter

    // --- Edge detection (optional, NULL if not supported) ---
    // Waits for an edge on pin, returns true if edge detected within timeout
    // If NULL, wire layer falls back to polling pin_read in a tight loop
    bool (*wait_edge)(cfw_pin_t pin, cfw_edge_t edge, uint32_t timeout_us);

} cfw_platform_t;
