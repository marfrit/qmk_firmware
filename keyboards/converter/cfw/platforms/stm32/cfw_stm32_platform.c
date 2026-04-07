// SPDX-License-Identifier: GPL-2.0-or-later
// CFW — STM32 Platform Driver (ChibiOS PAL)
// Pin I/O with native open-drain, microsecond timing via ChibiOS

#include "cfw_stm32.h"
#include "wait.h"

// ---- Pin control ----
// STM32 has true open-drain. No direction-toggling hacks needed.

static void stm32_pin_mode(cfw_pin_t pin, cfw_pin_mode_t mode) {
    switch (mode) {
        case CFW_PIN_INPUT:
            palSetLineMode(pin, PAL_MODE_INPUT);
            break;
        case CFW_PIN_INPUT_PULLUP:
            palSetLineMode(pin, PAL_MODE_INPUT_PULLUP);
            break;
        case CFW_PIN_OUTPUT:
            palSetLineMode(pin, PAL_MODE_OUTPUT_PUSHPULL);
            break;
        case CFW_PIN_OPEN_DRAIN:
            // This is the real deal. STM32 hardware open-drain:
            // write 0 = driven low, write 1 = high-Z (external pull-up)
            // No fighting, no glitches, no direction switching.
            palSetLineMode(pin, PAL_MODE_OUTPUT_OPENDRAIN);
            break;
    }
}

static void stm32_pin_write(cfw_pin_t pin, uint8_t value) {
    palWriteLine(pin, value ? PAL_HIGH : PAL_LOW);
}

static uint8_t stm32_pin_read(cfw_pin_t pin) {
    return palReadLine(pin) == PAL_HIGH ? 1 : 0;
}

// ---- Timing ----
// ChibiOS provides high-resolution timing via chVTGetSystemTimeX()
// and the RT (Real Time) counter.

static void stm32_delay_us(uint32_t us) {
    wait_us(us);
}

static uint32_t stm32_micros(void) {
    // ChibiOS doesn't have a direct microsecond counter in all configs.
    // We use chVTGetSystemTimeX() * (1000000 / CH_CFG_ST_FREQUENCY)
    // For most STM32 configs, ST_FREQUENCY is 10000 (100µs resolution)
    // or higher. For sub-µs we'd need the DWT cycle counter.
    //
    // For timeout calculations in the wire driver, millisecond
    // resolution is sufficient — the ISR handles the real-time part.
#if CH_CFG_ST_FREQUENCY >= 1000000
    return (uint32_t)chVTGetSystemTimeX();
#elif CH_CFG_ST_FREQUENCY >= 10000
    return (uint32_t)chVTGetSystemTimeX() * (1000000 / CH_CFG_ST_FREQUENCY);
#else
    return (uint32_t)chVTGetSystemTimeX() * (1000000 / CH_CFG_ST_FREQUENCY);
#endif
}

// ---- Edge detection ----
// ChibiOS PAL has event-driven edge detection (palEnableLineEvent),
// but for the platform driver we provide a polling fallback.
// The real edge detection happens in the PAL callback for the ISR.

static bool stm32_wait_edge(cfw_pin_t pin, cfw_edge_t edge, uint32_t timeout_us) {
    systime_t deadline = chVTGetSystemTimeX() + TIME_US2I(timeout_us);

    if (edge == CFW_EDGE_FALLING) {
        while (palReadLine(pin) == PAL_LOW) {
            if (chVTIsSystemTimeWithin(chVTGetSystemTimeX(), deadline, deadline)) return false;
        }
        while (palReadLine(pin) == PAL_HIGH) {
            if (!chVTIsSystemTimeWithinX(chVTGetSystemTimeX(), chVTGetSystemTimeX(), deadline)) return false;
        }
        return true;
    } else if (edge == CFW_EDGE_RISING) {
        while (palReadLine(pin) == PAL_HIGH) {
            if (!chVTIsSystemTimeWithinX(chVTGetSystemTimeX(), chVTGetSystemTimeX(), deadline)) return false;
        }
        while (palReadLine(pin) == PAL_LOW) {
            if (!chVTIsSystemTimeWithinX(chVTGetSystemTimeX(), chVTGetSystemTimeX(), deadline)) return false;
        }
        return true;
    }
    return false;
}

// ---- Platform instance ----

const cfw_platform_t cfw_stm32_platform = {
    .pin_mode   = stm32_pin_mode,
    .pin_write  = stm32_pin_write,
    .pin_read   = stm32_pin_read,
    .delay_us   = stm32_delay_us,
    .micros     = stm32_micros,
    .wait_edge  = stm32_wait_edge,
};
