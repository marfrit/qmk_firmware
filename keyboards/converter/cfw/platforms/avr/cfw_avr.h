// SPDX-License-Identifier: GPL-2.0-or-later
// CFW — AVR Platform (Pro Micro / ATmega32U4)
//
// ┌─────────────────────────────────────────────────────────��───────┐
// │                    AVR ARCHITECTURE                              │
// │                                                                  │
// │  Single-core, interrupt-driven. The classic approach.            │
// │                                                                  │
// │  ┌─────────────┐      ┌──────────────┐                          │
// │  │  Main Loop  │      │  Pin Change  │                          │
// │  │             │      │  ISR         │                          │
// │  │  QMK +      │◄─ring─┤              │                         │
// │  │  converter  │  buf  │ samples data │                         │
// │  │  protocol   │      │ on clock     │                          │
// │  │             │      │ falling edge │                          │
// │  └─────────────┘      └─��────┬───────┘                          │
// │                              │                                   │
// │                         Clock + Data pins                        │
// │                        (open-collector with external pullups)     │
// └─────────────────────────────────────────────────────────────────┘
// │                                                                  │
// │  HOW IT COMPARES TO RP2040:                                      │
// │                                                                  │
// │  The RP2040 platform uses PIO for bit timing, DMA for buffering, │
// │  and a second core for protocol processing — all in hardware.    │
// │  The AVR does everything in software on a single core:           │
// │                                                                  │
// │    - Bit timing: ISR fires on clock edge, samples data pin       │
// │    - Buffering: ring buffer in SRAM (32 bytes, ISR-filled)       │
// │    - Protocol: main loop processes bytes between USB polls        │
// │                                                                  │
// │  This means USB interrupts CAN delay byte processing (the ISR    │
// │  only takes ~2µs per bit, but the main loop might not read the   │
// │  ring buffer for up to 1ms). With a 32-byte ring buffer, that's  │
// │  fine for keyboard speeds (~1 byte/ms). It would NOT be fine for │
// │  protocols faster than ~32 bytes/ms, which no keyboard is.       │
// │                                                                  │
// │  WHY THE PRO MICRO IS STILL RELEVANT:                            │
// │                                                                  │
// │  Everyone in the keyboard community has Pro Micros in a drawer.  │
// │  They're $3, they have USB, and every existing QMK converter     │
// │  targets them. CFW must support them or it's dead on arrival.    │
// │  The RP2040 backend is "better" but the AVR backend is "first."  │
// └─────────────────────────────────────────────────────────────────┘
//
// PIN INTERRUPT SETUP:
// The ATmega32U4 (Pro Micro) has INT0-INT3 plus PCINT0-7 for pin
// change interrupts. Most converter designs use INT0 or INT1 for the
// PS/2 clock line (falling edge trigger). The ISR fires on each
// falling clock edge and samples the data line.
//
// OPEN-COLLECTOR EMULATION:
// AVR GPIOs aren't true open-drain. We simulate it:
//   - "High" = input mode with internal pull-up (or external pull-up)
//   - "Low"  = output mode, driving low
//   - Never drive high — that would fight the keyboard if it's pulling low
// This is the same approach QMK's ps2_io.c uses:
//   clock_lo(): write pin low, set output
//   clock_hi(): set input with pull-up (releases line)

#pragma once

#include "cfw.h"
#include <avr/interrupt.h>

// ---- Configuration ----

// Ring buffer size for ISR → main loop communication
// Must be power of 2. 32 bytes = 32 complete PS/2 bytes buffered.
// At ~1 byte/ms keyboard speed, this is 32ms of buffering — plenty
// for any main loop latency.
#ifndef CFW_AVR_RING_SIZE
#define CFW_AVR_RING_SIZE 32
#endif

// ---- AVR Platform Context ----

typedef struct {
    // Pin configuration
    cfw_pin_t data_pin;
    cfw_pin_t clock_pin;

    // ISR state: the interrupt handler builds bytes here
    // These are volatile because ISR and main loop share them.
    //
    // HOW THE ISR BUILDS A BYTE:
    // Each falling clock edge fires the ISR. The ISR has a state
    // counter (0-10) tracking which bit we're on:
    //   state 0: start bit (must be 0, else reset)
    //   state 1-8: data bits, shifted in LSB first
    //   state 9: parity bit (odd parity check)
    //   state 10: stop bit (must be 1), push to ring buffer
    //
    // If any bit fails validation (bad start, bad parity, bad stop),
    // the state resets to 0 and the partial byte is discarded.
    // The keyboard will re-send if we inhibit and release.
    volatile uint8_t isr_state;
    volatile uint8_t isr_data;
    volatile uint8_t isr_parity;

    // Ring buffer: ISR writes, main loop reads
    // Access must be atomic (cli/sei around multi-byte operations
    // on head/tail, or use single-byte operations which are inherently
    // atomic on AVR).
    uint8_t ring[CFW_AVR_RING_SIZE];
    volatile uint8_t ring_head;  // ISR writes here
    volatile uint8_t ring_tail;  // main loop reads here

} cfw_avr_ctx_t;

// ---- Global context (ISR needs access) ----
// AVR ISRs can't take parameters, so we use a global pointer.
// This limits us to one converter per AVR, which is physically
// reasonable — a Pro Micro has one PS/2 port.
extern cfw_avr_ctx_t *cfw_avr_active_ctx;

// ---- API ----

// Initialize the AVR platform (pins, ISR)
void cfw_avr_init(cfw_avr_ctx_t *ctx, cfw_pin_t data_pin, cfw_pin_t clock_pin);

// Create a wire driver using the AVR ISR backend
void cfw_avr_wire_init(cfw_wire_t *wire, cfw_avr_ctx_t *ctx);
