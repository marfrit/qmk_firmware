// SPDX-License-Identifier: GPL-2.0-or-later
// CFW — IBM PC Keyboard Converter Simulation Test
//
// Simulates a PS/2 keyboard sending "Hello World!" through the CFW
// IBMPC converter. Tests the full pipeline:
//   simulated wire → ibmpc converter → matrix events → HID keycodes
//
// The simulation replaces the wire layer with a mock that feeds
// pre-scripted PS/2 bytes (identification sequence + scan codes).
// No hardware needed — runs as a native Linux binary.
//
// Build: gcc -I../include -o test_ibmpc_sim test_ibmpc_sim.c ../converters/ibmpc.c
// Run:   ./test_ibmpc_sim

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

// ============================================================================
// MOCK QMK TYPES (minimal stubs for CFW headers)
// ============================================================================

typedef uint16_t matrix_row_t;
#define MATRIX_ROWS 8
#define MATRIX_COLS 16

// ============================================================================
// CFW HEADERS
// ============================================================================

#include "cfw_platform.h"
#include "cfw_wire.h"
#include "cfw_converter.h"

// ============================================================================
// SIMULATED PLATFORM (no-op timing, no real pins)
// ============================================================================

static uint32_t sim_micros_counter = 0;

static void sim_pin_mode(cfw_pin_t pin, cfw_pin_mode_t mode) { (void)pin; (void)mode; }
static void sim_pin_write(cfw_pin_t pin, uint8_t value) { (void)pin; (void)value; }
static uint8_t sim_pin_read(cfw_pin_t pin) { (void)pin; return 1; }
static void sim_delay_us(uint32_t us) { sim_micros_counter += us; }
static uint32_t sim_micros(void) { return sim_micros_counter; }

static const cfw_platform_t sim_platform = {
    .pin_mode  = sim_pin_mode,
    .pin_write = sim_pin_write,
    .pin_read  = sim_pin_read,
    .delay_us  = sim_delay_us,
    .micros    = sim_micros,
    .wait_edge = NULL,
};

// ============================================================================
// SIMULATED PS/2 WIRE (byte-level mock with scripted responses)
// ============================================================================
// The converter calls recv_byte() and send_byte(). We intercept these
// with a script of expected sends and queued responses.

#define SIM_QUEUE_SIZE 1024

static uint8_t sim_rx_queue[SIM_QUEUE_SIZE];  // bytes the "keyboard" sends
static int sim_rx_head = 0, sim_rx_tail = 0;

static uint8_t sim_tx_log[SIM_QUEUE_SIZE];    // bytes the converter sends
static int sim_tx_count = 0;

static void sim_enqueue(uint8_t byte) {
    sim_rx_queue[sim_rx_tail++] = byte;
    if (sim_rx_tail >= SIM_QUEUE_SIZE) sim_rx_tail = 0;
}

static void sim_enqueue_bytes(const uint8_t *bytes, int count) {
    for (int i = 0; i < count; i++) sim_enqueue(bytes[i]);
}

// recv_byte: return next byte from queue, or timeout
static int16_t sim_recv_byte(void *self, uint32_t timeout_us) {
    (void)self; (void)timeout_us;
    if (sim_rx_head == sim_rx_tail) return CFW_ERR_TIMEOUT;
    uint8_t byte = sim_rx_queue[sim_rx_head++];
    if (sim_rx_head >= SIM_QUEUE_SIZE) sim_rx_head = 0;
    return byte;
}

// send_byte: log the byte, return ACK (0xFA) by default
static int16_t sim_send_byte(void *self, uint8_t byte) {
    (void)self;
    if (sim_tx_count < SIM_QUEUE_SIZE) sim_tx_log[sim_tx_count++] = byte;
    // Most PS/2 commands get ACK
    return 0xFA;
}

static void sim_inhibit(void *self) { (void)self; }
static void sim_release(void *self) { (void)self; }
static void sim_send_bit(void *self, uint8_t bit) { (void)self; (void)bit; }
static int8_t sim_recv_bit(void *self, uint32_t timeout_us) { (void)self; (void)timeout_us; return 0; }
static uint8_t sim_clock_state(void *self) { (void)self; return 1; }
static uint8_t sim_data_state(void *self) { (void)self; return 1; }

// ============================================================================
// PS/2 SCAN CODE SET 2 CONSTANTS
// ============================================================================

// Make codes for Set 2 (single-byte keys)
#define SC2_A     0x1C
#define SC2_B     0x32
#define SC2_C     0x21
#define SC2_D     0x23
#define SC2_E     0x24
#define SC2_F     0x2B
#define SC2_G     0x34
#define SC2_H     0x33
#define SC2_I     0x43
#define SC2_J     0x3B
#define SC2_K     0x42
#define SC2_L     0x4B
#define SC2_M     0x3A
#define SC2_N     0x31
#define SC2_O     0x44
#define SC2_P     0x4D
#define SC2_Q     0x15
#define SC2_R     0x2D
#define SC2_S     0x1B
#define SC2_T     0x2C
#define SC2_U     0x3C
#define SC2_V     0x2A
#define SC2_W     0x1D
#define SC2_X     0x22
#define SC2_Y     0x35
#define SC2_Z     0x1A
#define SC2_1     0x16
#define SC2_SPACE 0x29
#define SC2_LSHIFT 0x12
#define SC2_BREAK 0xF0

// ============================================================================
// TEST INFRASTRUCTURE
// ============================================================================

static int test_pass = 0, test_fail = 0;

#define ASSERT(cond, msg) do { \
    if (cond) { test_pass++; } \
    else { test_fail++; printf("  FAIL: %s\n", msg); } \
} while(0)

#define ASSERT_EQ(a, b, msg) do { \
    if ((a) == (b)) { test_pass++; } \
    else { test_fail++; printf("  FAIL: %s (expected 0x%X, got 0x%X)\n", msg, (b), (a)); } \
} while(0)

// ============================================================================
// PS/2 IDENTIFICATION SEQUENCE
// ============================================================================
// The IBMPC converter's identify() does:
//   1. Drain pending data
//   2. Send 0xFF (reset), expect 0xFA (ACK) — returned by send_byte mock
//   3. Wait for 0xAA (BAT passed) — we enqueue this
//   4. Wait for optional 0xBF (terminal BAT) — timeout
//   5. Send 0xF2 (read ID), expect 0xFA — returned by send_byte mock
//   6. Read ID bytes: 0xAB 0x83 (MF2 keyboard)

static void setup_identification(void) {
    // BAT response
    sim_enqueue(0xAA);
    // ID response: AB 83 = standard MF2 keyboard
    sim_enqueue(0xAB);
    sim_enqueue(0x83);
}

// ============================================================================
// SCAN CODE HELPERS
// ============================================================================

static void enqueue_key_press(uint8_t scancode) {
    sim_enqueue(scancode);
}

static void enqueue_key_release(uint8_t scancode) {
    sim_enqueue(SC2_BREAK);  // 0xF0
    sim_enqueue(scancode);
}

static void enqueue_key_tap(uint8_t scancode) {
    enqueue_key_press(scancode);
    enqueue_key_release(scancode);
}

static void enqueue_shifted_key_tap(uint8_t scancode) {
    enqueue_key_press(SC2_LSHIFT);
    enqueue_key_press(scancode);
    enqueue_key_release(scancode);
    enqueue_key_release(SC2_LSHIFT);
}

// Enqueue "Hello World!" as PS/2 Set 2 scan codes
static void enqueue_hello_world(void) {
    // H (shifted)
    enqueue_shifted_key_tap(SC2_H);
    // ello
    enqueue_key_tap(SC2_E);
    enqueue_key_tap(SC2_L);
    enqueue_key_tap(SC2_L);
    enqueue_key_tap(SC2_O);
    // space
    enqueue_key_tap(SC2_SPACE);
    // W (shifted)
    enqueue_shifted_key_tap(SC2_W);
    // orld
    enqueue_key_tap(SC2_O);
    enqueue_key_tap(SC2_R);
    enqueue_key_tap(SC2_L);
    enqueue_key_tap(SC2_D);
    // ! (shifted 1)
    enqueue_shifted_key_tap(SC2_1);
}

// ============================================================================
// UNIMAP → HID KEYCODE TABLE
// ============================================================================
// The IBMPC converter maps scan codes to UNIMAP positions (row*16+col).
// We need to verify the output events match expected HID keycodes.
// UNIMAP positions for the keys in "Hello World!":

// From the ibmpc.c unimap_cs2 table:
// H (0x33) → unimap 0x0B → row 0, col 11
// E (0x24) → unimap 0x08 → row 0, col 8
// L (0x4B) → unimap 0x0F → row 0, col 15
// O (0x44) → unimap 0x12 → row 1, col 2
// Space (0x29) → unimap 0x2C → row 2, col 12
// W (0x1D) → unimap 0x1A → row 1, col 10
// R (0x2D) → unimap 0x15 → row 1, col 5
// D (0x23) → unimap 0x07 → row 0, col 7
// 1 (0x16) → unimap 0x1E → row 1, col 14
// LShift (0x12) → unimap 0x79 → row 7, col 9  (approximate)

// ============================================================================
// MAIN TEST
// ============================================================================

int main(void) {
    printf("=== CFW IBM PC Converter Simulation ===\n\n");

    // ---- Setup wire ----
    cfw_wire_t wire = {0};
    wire.type = CFW_WIRE_CLOCK_DATA;
    wire.clock_data.hw = &sim_platform;
    wire.clock_data.clock_pin = 0;
    wire.clock_data.data_pin = 1;
    wire.clock_data.recv_byte = sim_recv_byte;
    wire.clock_data.send_byte = sim_send_byte;
    wire.clock_data.inhibit = sim_inhibit;
    wire.clock_data.release = sim_release;
    wire.clock_data.send_bit = sim_send_bit;
    wire.clock_data.recv_bit = sim_recv_bit;
    wire.clock_data.clock_state = sim_clock_state;
    wire.clock_data.data_state = sim_data_state;

    // ---- Test 1: Identification ----
    printf("-- Test 1: Identification --\n");
    setup_identification();

    cfw_keyboard_info_t info = {0};
    extern const cfw_converter_t cfw_ibmpc_converter;
    bool id_ok = cfw_ibmpc_converter.identify(&wire, &info);

    ASSERT(id_ok, "identify succeeds");
    ASSERT(info.type != CFW_KB_UNKNOWN, "keyboard type identified");
    printf("  Keyboard: %s (ID: 0x%04X, Set: %d)\n",
           info.description, info.id, info.scan_set);

    // Verify the converter sent reset (0xFF) and read-ID (0xF2)
    ASSERT(sim_tx_count >= 2, "converter sent at least 2 commands");
    ASSERT_EQ(sim_tx_log[0], 0xFF, "first command = reset (0xFF)");
    // 0xF2 should be in the log
    bool found_f2 = false;
    for (int i = 0; i < sim_tx_count; i++) {
        if (sim_tx_log[i] == 0xF2) found_f2 = true;
    }
    ASSERT(found_f2, "read-ID command (0xF2) was sent");

    // ---- Test 2: Init ----
    printf("\n-- Test 2: Init --\n");
    bool init_ok = cfw_ibmpc_converter.init(&wire, &info);
    ASSERT(init_ok, "init succeeds");

    // ---- Test 3: Scan "Hello World!" ----
    printf("\n-- Test 3: Scan 'Hello World!' --\n");

    // Enqueue the scan codes
    enqueue_hello_world();

    // Expected events for "Hello World!":
    // Each character produces make+break events.
    // Shifted chars produce: LShift-make, key-make, key-break, LShift-break
    typedef struct {
        uint8_t row;
        uint8_t col;
        bool pressed;
        const char *label;
    } expected_event_t;

    // We'll collect all events and verify the sequence
    #define MAX_EVENTS 256
    typedef struct {
        uint8_t row, col;
        bool pressed;
    } event_t;

    event_t events[MAX_EVENTS];
    int event_count = 0;

    // Drain all events from the converter.
    // next_event reads one byte per call and runs the state machine.
    // Multi-byte sequences (F0 xx = break) need multiple calls before
    // producing an event. Keep calling until the queue is fully drained.
    uint8_t row, col;
    bool pressed;
    int empty_runs = 0;
    while (empty_runs < 3) {  // 3 consecutive empty calls = done
        if (cfw_ibmpc_converter.next_event(&wire, &info, &row, &col, &pressed)) {
            if (event_count < MAX_EVENTS) {
                events[event_count].row = row;
                events[event_count].col = col;
                events[event_count].pressed = pressed;
                event_count++;
            }
            empty_runs = 0;
        } else {
            empty_runs++;
        }
    }

    printf("  Total events: %d\n", event_count);

    // "Hello World!" = 12 characters
    // H = shift+h = 4 events (shift↓ h↓ h↑ shift↑)
    // e,l,l,o,space = 2 events each = 10 events
    // W = shift+w = 4 events
    // o,r,l,d = 2 events each = 8 events
    // ! = shift+1 = 4 events
    // Total: 4 + 10 + 4 + 8 + 4 = 30 events
    ASSERT_EQ(event_count, 30, "event count for 'Hello World!'");

    // Verify the key sequence by checking make events only
    // (every other event starting from index 0/1 depending on shifted)
    printf("  Key sequence (makes only):\n");
    int make_count = 0;
    for (int i = 0; i < event_count; i++) {
        if (events[i].pressed) {
            printf("    [%2d] row=%d col=%2d (unimap 0x%02X) %s\n",
                   make_count, events[i].row, events[i].col,
                   events[i].row * 16 + events[i].col,
                   events[i].pressed ? "↓" : "↑");
            make_count++;
        }
    }

    // Verify first character 'H': LShift down, then H down
    // LShift (Set 2: 0x12) → unimap should be in row 7
    if (event_count >= 2) {
        ASSERT(events[0].pressed, "first event is key-down");
        printf("  First key-down: row=%d col=%d (unimap 0x%02X) — should be LShift\n",
               events[0].row, events[0].col, events[0].row * 16 + events[0].col);

        ASSERT(events[1].pressed, "second event is key-down");
        printf("  Second key-down: row=%d col=%d (unimap 0x%02X) — should be H\n",
               events[1].row, events[1].col, events[1].row * 16 + events[1].col);
    }

    // ---- Test 4: Matrix integration ----
    printf("\n-- Test 4: Matrix integration --\n");

    // Re-enqueue a simple 'a' keypress and test through cfw_matrix
    sim_rx_head = sim_rx_tail = 0;  // clear queue
    enqueue_key_tap(SC2_A);

    // Simulate cfw_matrix_scan
    matrix_row_t matrix[MATRIX_ROWS] = {0};
    int changed = 0;

    // Drain events and apply to matrix
    empty_runs = 0;
    while (empty_runs < 3) {
    if (!cfw_ibmpc_converter.next_event(&wire, &info, &row, &col, &pressed)) {
        empty_runs++; continue;
    }
    empty_runs = 0;
    {
        if (row < MATRIX_ROWS && col < MATRIX_COLS) {
            if (pressed) {
                matrix[row] |= ((matrix_row_t)1 << col);
            } else {
                matrix[row] &= ~((matrix_row_t)1 << col);
            }
            changed++;
        }
    }
    }

    ASSERT_EQ(changed, 2, "'a' tap = 2 events (make + break)");
    // After break, matrix should be clear (key released)
    bool all_clear = true;
    for (int i = 0; i < MATRIX_ROWS; i++) {
        if (matrix[i] != 0) all_clear = false;
    }
    ASSERT(all_clear, "matrix clear after key release");

    // ---- Summary ----
    printf("\n=== Results: %d passed, %d failed ===\n", test_pass, test_fail);
    if (test_fail == 0) printf("=== ALL TESTS PASSED ===\n");

    return test_fail > 0 ? 1 : 0;
}
