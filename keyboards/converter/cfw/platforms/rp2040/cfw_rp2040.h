// SPDX-License-Identifier: GPL-2.0-or-later
// CFW — RP2040 Platform: Dual-Core + PIO + DMA Architecture
//
// ┌─────────────────────────────────────────────────────────────────────┐
// │                        ARCHITECTURE OVERVIEW                        │
// │                                                                     │
// │  This is the "overkill" RP2040 platform for CFW. It uses every      │
// │  relevant hardware feature of the RP2040 to achieve deterministic,  │
// │  zero-jitter keyboard protocol handling while running QMK on the    │
// │  other core without interference.                                   │
// │                                                                     │
// │  ┌──────────┐    ┌──────────┐                                       │
// │  │  Core 0  │    │  Core 1  │                                       │
// │  │  (QMK)   │    │ (Wire)   │                                       │
// │  │          │    │          │                                        │
// │  │ keymap   │    │ protocol │                                        │
// │  │ layers   │◄──SIO──┤ state    │                                   │
// │  │ tap-hold │  FIFO  │ machine  │                                   │
// │  │ USB HID  │    │          │                                        │
// │  └──────────┘    └────▲─────┘                                       │
// │                       │                                              │
// │                      DMA                                             │
// │                       │                                              │
// │                  ┌────┴─────┐                                        │
// │                  │   PIO    │                                        │
// │                  │ Block 1  │                                        │
// │                  │          │                                        │
// │                  │ SM0: R/W │◄──── Clock + Data pins                 │
// │                  │          │      (PS/2, XT, ADB...)               │
// │                  └──────────┘                                        │
// │                                                                     │
// │  Data flow:                                                         │
// │  1. PIO samples clock/data pins, assembles 11-bit frames            │
// │  2. DMA transfers completed frames from PIO RX FIFO to ring buffer  │
// │  3. Core 1 processes frames: validates, decodes, tags replies        │
// │  4. Core 1 pushes (row, col, pressed) events through SIO FIFO       │
// │  5. Core 0 drains SIO FIFO in matrix_scan(), feeds QMK keymap       │
// │                                                                     │
// │  Key properties:                                                    │
// │  - Core 0 NEVER touches wire protocol (zero jitter on USB/QMK)      │
// │  - Core 1 NEVER touches USB (zero jitter on wire protocol)          │
// │  - PIO handles bit-level timing in hardware (cycle-accurate)        │
// │  - DMA moves data without CPU involvement                           │
// │  - SIO FIFO is hardware-synchronized (no locks, no shared memory)   │
// └─────────────────────────────────────────────────────────────────────┘
//
// PRIOR ART NOTE:
// Documentation on RP2040 multicore + PIO + DMA combined usage is sparse.
// The Pico SDK examples cover each feature individually but not the
// composition. The key references are:
//   - RP2040 Datasheet §2.3.1 (SIO FIFO): Two 32-bit FIFOs, one per
//     direction, 8 entries deep, with IRQ on not-empty/not-full.
//   - RP2040 Datasheet §2.5 (DMA): 12 channels, can be triggered by
//     PIO DREQ (Data Request), transfers without CPU involvement.
//   - RP2040 Datasheet §3.2 (PIO): 4 state machines per block, each
//     with TX/RX FIFOs (4x32-bit each), IRQ flags for inter-SM sync.
//   - Pico SDK pico_multicore: multicore_launch_core1(), sio_hw->fifo_*
//
// The combination PIO→DMA→ring buffer is documented in a few community
// projects (logic analyzers, audio samplers) but not for keyboard
// protocols. The SIO FIFO for structured event passing between cores
// is, as far as we know, novel for this use case.

#pragma once

#include "cfw.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "pico/multicore.h"

// ---- Configuration ----

// Which PIO block to use (pio0 or pio1)
// pio1 is preferred because pio0 is often used by TinyUSB/ChibiOS USB
#ifndef CFW_PIO_BLOCK
#define CFW_PIO_BLOCK pio1
#endif

// DMA ring buffer size (must be power of 2)
// Each entry is a 32-bit PIO frame. 256 entries = 1KB, enough for
// ~23ms of continuous PS/2 data at 11 bits * 1kHz = 11kbit/s
#ifndef CFW_DMA_RING_SIZE
#define CFW_DMA_RING_SIZE 256
#endif

// SIO FIFO event encoding
// We pack (row, col, pressed) into a single 32-bit word for the
// inter-core FIFO. The SIO FIFO is only 32 bits wide and 8 entries
// deep, so packing efficiency matters for burst scenarios (keyboard
// sends multiple scancodes quickly, e.g. after scan code set change).
//
// Encoding: [31:16] = magic marker, [15:8] = row, [7:1] = col, [0] = pressed
// The magic marker prevents misinterpreting stale FIFO data or
// multicore_lockout protocol messages as key events.
#define CFW_SIO_MAGIC        0xCF00  // upper 16 bits
#define CFW_SIO_PACK(r,c,p)  (((uint32_t)CFW_SIO_MAGIC << 16) | ((uint32_t)(r) << 8) | ((uint32_t)(c) << 1) | ((p) ? 1 : 0))
#define CFW_SIO_IS_EVENT(w)  (((w) >> 16) == CFW_SIO_MAGIC)
#define CFW_SIO_ROW(w)       (((w) >> 8) & 0xFF)
#define CFW_SIO_COL(w)       (((w) >> 1) & 0x7F)
#define CFW_SIO_PRESSED(w)   ((w) & 1)

// ---- RP2040 Platform Context ----

typedef struct {
    // PIO configuration
    PIO pio;
    uint sm;              // state machine index
    uint pio_offset;      // program offset in PIO instruction memory

    // DMA configuration
    //
    // HOW DMA + PIO WORKS:
    // The PIO state machine has a 4-entry RX FIFO. When the PIO program
    // completes a frame (11 bits shifted in, auto-push), the frame appears
    // in the RX FIFO. Without DMA, we'd need to poll or use an IRQ to
    // read each frame before the FIFO overflows (4 frames = ~4ms at PS/2
    // speed — tight for a CPU also running protocol logic).
    //
    // Instead, we configure a DMA channel with:
    //   - Source: PIO RX FIFO register (hardware address, not a pointer)
    //   - Destination: ring buffer in RAM
    //   - Transfer size: 32 bits (one PIO frame)
    //   - Trigger: DREQ from PIO RX FIFO not-empty
    //   - Wrap: destination address wraps at ring buffer boundary
    //
    // The DMA channel runs autonomously. Every time PIO pushes a frame,
    // DMA moves it to RAM without any CPU involvement. The ring buffer
    // write pointer advances automatically (DMA write address register).
    // Core 1 reads from the ring buffer at its own pace, comparing its
    // read pointer against DMA's current write position.
    //
    // This gives us effectively unlimited FIFO depth (256 entries vs 4)
    // and completely decouples PIO timing from CPU processing speed.
    //
    int dma_channel;
    uint32_t dma_ring_buffer[CFW_DMA_RING_SIZE] __attribute__((aligned(CFW_DMA_RING_SIZE * 4)));
    volatile uint32_t dma_read_idx;  // Core 1's read position (index, not pointer)

    // Reply framing state
    //
    // THE REPLY TAGGING TRICK:
    // In standard PS/2, the host must inhibit the clock line (pull low)
    // before sending a command, then release and wait for the keyboard's
    // response. This means the keyboard can't send key events while the
    // host is waiting for a reply — you lose keystrokes.
    //
    // The PIO program here uses a different approach: it tags each
    // received frame with a "reply bit" that indicates whether the
    // frame arrived in response to a host-to-device transmission.
    // The PIO sets this bit based on its internal state (was the TX
    // FIFO non-empty when the frame arrived?). This lets us sort
    // incoming data into two streams:
    //   - replies: ACK (0xFA), device ID, BAT result, etc.
    //   - messages: unsolicited key make/break events
    //
    // The keyboard never needs to be silenced. Free speech for all
    // keyboards, even the ones with aging crystals that don't handle
    // inhibit/release timing gracefully.
    //
    uint8_t reply_buffer[16];
    volatile uint8_t reply_head;
    volatile uint8_t reply_tail;

    // Pin configuration
    cfw_pin_t data_pin;    // PS/2 data (must be clock_pin - 1)
    cfw_pin_t clock_pin;   // PS/2 clock (must be data_pin + 1)

} cfw_rp2040_ctx_t;

// ---- API ----

// Initialize the RP2040 platform (PIO, DMA, pins)
void cfw_rp2040_init(cfw_rp2040_ctx_t *ctx, cfw_pin_t data_pin, cfw_pin_t clock_pin);

// Create a wire driver using this platform's PIO backend
void cfw_rp2040_wire_init(cfw_wire_t *wire, cfw_rp2040_ctx_t *ctx);

// Launch Core 1 with the wire protocol handler
// converter: the active converter (e.g. ibmpc)
// wire: the initialized wire driver
// Core 1 runs forever, pushing events through SIO FIFO
void cfw_rp2040_launch_core1(const cfw_converter_t *converter, cfw_wire_t *wire);

// Drain SIO FIFO into QMK matrix (called from Core 0's matrix_scan)
// Returns true if any key events were received
//
// HOW THE SIO FIFO WORKS:
// The RP2040 has two dedicated hardware FIFOs in the SIO (Single-cycle I/O)
// block, one for each direction of inter-core communication. Each FIFO is
// 8 entries deep, 32 bits wide.
//
// Key properties:
//   - Writing to a full FIFO stalls the writing core (not an error)
//   - Reading from an empty FIFO stalls the reading core (use _available)
//   - Completely lock-free: no mutexes, spinlocks, or shared memory needed
//   - Single-cycle access: reading/writing is 1 clock cycle (8ns at 125MHz)
//   - IRQ available on not-empty/not-full (we don't use it — polling is fast)
//
// We use Core1→Core0 FIFO only. Core 1 pushes packed key events,
// Core 0 drains them in matrix_scan(). If the FIFO fills up (8 events
// backed up), Core 1 stalls until Core 0 catches up. At QMK's ~1kHz
// scan rate, this can only happen if 8+ keys change state between
// consecutive scans — physically impossible on a keyboard.
//
// The Pico SDK provides multicore_fifo_push_blocking() and
// multicore_fifo_pop_blocking(), but we use the raw SIO registers
// for the non-blocking variants:
//   sio_hw->fifo_st: FIFO status (VLD = data available, RDY = space available)
//   sio_hw->fifo_rd: read one word (must check VLD first)
//   sio_hw->fifo_wr: write one word (must check RDY first)
//
bool cfw_rp2040_drain_sio_fifo(matrix_row_t matrix[]);

// Send a command to the keyboard (called from Core 0, routed to Core 1)
// Uses Core0→Core1 FIFO for the reverse direction
void cfw_rp2040_send_command(uint8_t cmd);
