// SPDX-License-Identifier: GPL-2.0-or-later
// CFW — STM32 Platform (ChibiOS)
//
// ┌─────────────────────────────────────────────────────────────────┐
// │                    STM32 ARCHITECTURE                           │
// │                                                                 │
// │  Single-core with ChibiOS RTOS. The "goldilocks" platform:     │
// │  more capable than AVR (open-drain GPIO, timer input capture,   │
// │  DMA), but simpler than RP2040 (no PIO, no second core).       │
// │                                                                 │
// │  ┌─────────────┐      ┌──────────────┐                         │
// │  │  ChibiOS    │      │  EXTI / PAL  │                         │
// │  │  Thread     │      │  Callback    │                         │
// │  │             │◄─ring─┤              │                        │
// │  │  QMK +      │  buf  │ samples data │                        │
// │  │  converter  │      │ on clock     │                         │
// │  │  protocol   │      │ falling edge │                         │
// │  └─────────────┘      └──────┬─��─────┘                         │
// │                              │                                  │
// │                         Clock + Data pins                       │
// │                        (true open-drain, no hacks)              │
// └─────────────────────────────────────────────────────────────────┘
// │                                                                 │
// │  ADVANTAGES OVER AVR:                                           │
// │  - True open-drain GPIO (PAL_MODE_OUTPUT_OPENDRAIN)             │
// │  - Faster CPU (48-100MHz vs 16MHz): more headroom for protocol  │
// │  - ChibiOS PAL callbacks: clean ISR registration, no INT macros │
// │  - 32-bit EXTI: any GPIO pin can trigger an interrupt           │
// │    (AVR is limited to specific INT0-3 or PCINT pins)            │
// │  - DMA available for future UART-based protocols (Sun, Palm)    │
// │  - Timer input capture for precise clock frequency measurement  │
// │    (useful for identifying XT vs AT by clock speed)             │
// │                                                                 │
// │  VS RP2040:                                                     │
// │  - No PIO: bit timing is ISR-based like AVR, just faster        │
// │  - No second core: protocol runs in main loop like AVR          │
// │  - But: ChibiOS threading could dedicate a thread to protocol   │
// │    (not implemented yet, but architecturally possible)           │
// │                                                                 │
// │  SUPPORTED BOARDS:                                              │
// │  All use the same ChibiOS PAL API. Board differences are just   │
// │  pin mappings and clock config (handled by QMK's board files).  │
// │                                                                 │
// │  ┌──────────────┬────────────┬────────┬───────┬───────────────┐ │
// │  │ Board        │ MCU        │ Flash  │ USB   │ Notes         │ │
// │  ├──────────────┼────────────┼────────┼───────┼───────────────┤ │
// │  │ Blue Pill    │ F103C8T6   │ 64KB   │ Micro │ $2, 72MHz,    │ │
// │  │              │            │        │       │ needs USB res  │ │
// │  │              │            │        │       │ bodge (1.5kΩ   │ │
// │  │              │            │        │       │ on PA12)       │ │
// │  ├──────────────┼────────────┼────────┼───────┼───────────────┤ │
// │  │ Black Pill   │ F401CCU6   │ 256KB  │ C     │ $3, 84MHz,    │ │
// │  │ (WeAct v3)   │ F411CEU6   │ 512KB  │ C     │ $4, 100MHz,   │ │
// │  │              │            │        │       │ recommended   │ │
// │  ├──────────────┼────────────┼────────┼───────┼───────────────┤ │
// │  │ Purple Pill  │ F103C8T6   │ 64KB   │ C     │ Blue Pill +   │ │
// │  │              │            │        │       │ USB-C, rare   │ │
// │  ├──────────────┼────────────┼────────┼───────┼───────────────┤ │
// │  │ Proton C     │ F303CCT6   │ 256KB  │ C     │ QMK's own,    │ │
// │  │              │            │        │       │ Pro Micro     │ │
// │  │              │            │        │       │ footprint     │ │
// │  └──────────────┴────────────┴────────┴───────┴───────────────┘ │
// │                                                                 │
// │  CLOCK PIN FLEXIBILITY:                                         │
// │  Unlike AVR where the clock pin must be on INT0-3, STM32's     │
// │  EXTI system can trigger an interrupt on ANY GPIO pin. Just     │
// │  configure the pin and register a PAL callback. This means      │
// │  converter PCB designers aren't constrained to specific pins.   │
// │                                                                 │
// │  OPEN-DRAIN:                                                    │
// │  STM32 has native open-drain output mode. When configured as    │
// │  PAL_MODE_OUTPUT_OPENDRAIN:                                     │
// │    - Writing 0: pin is driven low                               │
// │    - Writing 1: pin is floating (pulled high by external R)     │
// │  No direction toggling needed. No fighting the keyboard.        │
// │  This is how PS/2 was meant to be driven.                       │
// └─────────────────────────────────────────────────────────────────┘

#pragma once

#include "cfw.h"
#include "ch.h"
#include "hal.h"

// ---- Configuration ----

#ifndef CFW_STM32_RING_SIZE
#define CFW_STM32_RING_SIZE 32
#endif

// ---- Board Pin Presets ----
// Define CFW_STM32_BOARD in your keyboard's config.h to use a preset,
// or define CFW_STM32_DATA_PIN and CFW_STM32_CLOCK_PIN manually.

// Blue Pill (STM32F103C8T6) — common wiring
// PA0 = data, PA1 = clock (EXTI1)
// NOTE: Blue Pill needs a 1.5kΩ resistor from PA12 to 3.3V for USB.
// Without it, the host won't enumerate the device. This is the
// famous "Blue Pill USB bodge" that every forum post mentions.
#if defined(CFW_BOARD_BLUEPILL)
#   ifndef CFW_STM32_DATA_PIN
#       define CFW_STM32_DATA_PIN  PAL_LINE(GPIOA, 0)
#   endif
#   ifndef CFW_STM32_CLOCK_PIN
#       define CFW_STM32_CLOCK_PIN PAL_LINE(GPIOA, 1)
#   endif
#endif

// Black Pill v3 (WeAct STM32F401/F411) — recommended board
// PB12 = data, PB13 = clock
// USB-C, no bodge needed, 100MHz, 512KB flash. The upgrade from Pro Micro.
#if defined(CFW_BOARD_BLACKPILL)
#   ifndef CFW_STM32_DATA_PIN
#       define CFW_STM32_DATA_PIN  PAL_LINE(GPIOB, 12)
#   endif
#   ifndef CFW_STM32_CLOCK_PIN
#       define CFW_STM32_CLOCK_PIN PAL_LINE(GPIOB, 13)
#   endif
#endif

// Proton C (STM32F303CCT6) — QMK's own, Pro Micro footprint
// D1 = data (TX, PB6), D0 = clock (RX, PB7) — same as Pro Micro convention
#if defined(CFW_BOARD_PROTONC)
#   ifndef CFW_STM32_DATA_PIN
#       define CFW_STM32_DATA_PIN  PAL_LINE(GPIOB, 6)
#   endif
#   ifndef CFW_STM32_CLOCK_PIN
#       define CFW_STM32_CLOCK_PIN PAL_LINE(GPIOB, 7)
#   endif
#endif

// ---- STM32 Platform Context ----

typedef struct {
    cfw_pin_t data_pin;
    cfw_pin_t clock_pin;

    // ISR state (same approach as AVR, just faster)
    volatile uint8_t isr_state;
    volatile uint8_t isr_data;
    volatile uint8_t isr_parity;

    // Ring buffer
    uint8_t ring[CFW_STM32_RING_SIZE];
    volatile uint8_t ring_head;
    volatile uint8_t ring_tail;

} cfw_stm32_ctx_t;

// Global context (PAL callback needs access)
extern cfw_stm32_ctx_t *cfw_stm32_active_ctx;

// ---- API ----
void cfw_stm32_init(cfw_stm32_ctx_t *ctx, cfw_pin_t data_pin, cfw_pin_t clock_pin);
void cfw_stm32_wire_init(cfw_wire_t *wire, cfw_stm32_ctx_t *ctx);
