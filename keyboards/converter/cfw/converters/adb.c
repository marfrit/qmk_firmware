// SPDX-License-Identifier: GPL-2.0-or-later
// CFW — Apple Desktop Bus Converter
//
// Ported from TMK/QMK adb.c (Jun Wako, Shay Green) into the CFW
// converter interface. Original AVR bit-banging replaced with
// cfw_wire_t bit-level ops for multi-platform support.
//
// ADB is a single-wire protocol. The host drives the bus with
// attention + command, then the device responds with data.
// Bit timing: bit0 = 65us low + 35us high
//             bit1 = 35us low + 65us high
// Attention:  800us low, then start bit (1)

#include "adb.h"
#include <string.h>

// ============================================================================
// INTERNAL STATE
// ============================================================================

typedef struct {
    cfw_keyboard_info_t *info;
    uint8_t handler_id;
    bool is_iso;
    // Pending second keycode from a two-key response
    bool has_pending;
    uint8_t pending_key;
} adb_state_t;

static adb_state_t state;

// ============================================================================
// LOW-LEVEL ADB BUS OPERATIONS
// ============================================================================
// ADB uses a single data line (open-collector with external pull-up).
// We use the wire's data_pin for everything. clock_pin is unused.

static inline cfw_wire_clock_data_t *cd(cfw_wire_t *wire) {
    return &wire->clock_data;
}

// Drive data line low
static inline void data_lo(cfw_wire_t *wire) {
    cd(wire)->hw->pin_mode(cd(wire)->data_pin, CFW_PIN_OUTPUT);
    cd(wire)->hw->pin_write(cd(wire)->data_pin, 0);
}

// Release data line (pull-up brings it high)
static inline void data_hi(cfw_wire_t *wire) {
    cd(wire)->hw->pin_mode(cd(wire)->data_pin, CFW_PIN_INPUT_PULLUP);
}

// Read data line state
static inline uint8_t data_in(cfw_wire_t *wire) {
    return cd(wire)->hw->pin_read(cd(wire)->data_pin);
}

// Wait for data line to go low, returns remaining us (0 = timeout)
static uint16_t wait_data_lo(cfw_wire_t *wire, uint16_t us) {
    const cfw_platform_t *hw = cd(wire)->hw;
    uint32_t start = hw->micros();
    while (hw->pin_read(cd(wire)->data_pin)) {
        if ((hw->micros() - start) >= us) return 0;
    }
    return us - (uint16_t)(hw->micros() - start);
}

// Wait for data line to go high, returns remaining us (0 = timeout)
static uint16_t wait_data_hi(cfw_wire_t *wire, uint16_t us) {
    const cfw_platform_t *hw = cd(wire)->hw;
    uint32_t start = hw->micros();
    while (!hw->pin_read(cd(wire)->data_pin)) {
        if ((hw->micros() - start) >= us) return 0;
    }
    return us - (uint16_t)(hw->micros() - start);
}

// Place a 0 bit: 65us low, 35us high
static void place_bit0(cfw_wire_t *wire) {
    data_lo(wire);
    cd(wire)->hw->delay_us(65);
    data_hi(wire);
    cd(wire)->hw->delay_us(35);
}

// Place a 1 bit: 35us low, 65us high
static void place_bit1(cfw_wire_t *wire) {
    data_lo(wire);
    cd(wire)->hw->delay_us(35);
    data_hi(wire);
    cd(wire)->hw->delay_us(65);
}

// Send attention signal: 800us low, then start bit (1)
static void attention(cfw_wire_t *wire) {
    data_lo(wire);
    cd(wire)->hw->delay_us(800 - 35);  // bit1 holds lo for 35 more
    place_bit1(wire);
}

// Send a byte MSB-first
static void send_byte(cfw_wire_t *wire, uint8_t data) {
    for (int i = 0; i < 8; i++) {
        if (data & (0x80 >> i))
            place_bit1(wire);
        else
            place_bit0(wire);
    }
}

// ============================================================================
// ADB COMMANDS
// ============================================================================

// Build ADB command byte: addr(4) | cmd(2) | reg(2)
static inline uint8_t adb_cmd(uint8_t addr, uint8_t cmd, uint8_t reg) {
    return (addr << 4) | cmd | reg;
}

// Send Talk command and receive up to `len` bytes of response.
// Returns number of bytes received (0 = no response / timeout).
static uint8_t adb_talk_buf(cfw_wire_t *wire, uint8_t addr, uint8_t reg,
                             uint8_t *buf, uint8_t len) {
    for (uint8_t i = 0; i < len; i++) buf[i] = 0;

    // Send command
    attention(wire);
    send_byte(wire, adb_cmd(addr, ADB_CMD_TALK, reg));
    place_bit0(wire);  // stop bit

    // Wait for service request to clear (device may hold low up to 310us)
    if (!wait_data_hi(wire, 500)) {
        return 0;
    }

    // Wait for device to start responding (Tlt: 140-260us)
    if (!wait_data_lo(wire, 500)) {
        return 0;  // No data — not an error, device has nothing to say
    }

    // Start bit (1): low then high
    if (!wait_data_hi(wire, 40)) {
        return 0;
    }
    if (!wait_data_lo(wire, 100)) {
        return 0;
    }

    // Receive data bits
    uint8_t n = 0;
    do {
        // Measure low and high durations within a bit cell (max 130us)
        uint16_t lo = wait_data_hi(wire, 130);
        if (!lo) break;  // no more bits or stop

        uint16_t hi = wait_data_lo(wire, lo);
        if (!hi) break;  // stop bit or bus error

        if (n / 8 >= len) continue;  // overflow protection

        buf[n / 8] <<= 1;
        // If low time > high time, it's a 0; otherwise 1
        if ((130 - lo) < (lo - hi)) {
            buf[n / 8] |= 1;
        }
    } while (++n);

    return n / 8;
}

// Talk: read 16-bit register value (returns 0 on error/no data)
static uint16_t adb_talk(cfw_wire_t *wire, uint8_t addr, uint8_t reg) {
    uint8_t buf[8];
    uint8_t len = adb_talk_buf(wire, addr, reg, buf, 8);
    if (len != 2) return 0;
    return (buf[0] << 8) | buf[1];
}

// Listen: write 16-bit value to register
static void adb_listen(cfw_wire_t *wire, uint8_t addr, uint8_t reg,
                        uint8_t data_h, uint8_t data_l) {
    attention(wire);
    send_byte(wire, adb_cmd(addr, ADB_CMD_LISTEN, reg));
    place_bit0(wire);           // stop bit
    cd(wire)->hw->delay_us(200); // Tlt
    place_bit1(wire);           // start bit
    send_byte(wire, data_h);
    send_byte(wire, data_l);
    place_bit0(wire);           // stop bit
}

// Flush device buffer
static void adb_flush(cfw_wire_t *wire, uint8_t addr) {
    attention(wire);
    send_byte(wire, adb_cmd(addr, ADB_CMD_FLUSH, 0));
    place_bit0(wire);
    cd(wire)->hw->delay_us(200);
}

// ============================================================================
// CONVERTER INTERFACE
// ============================================================================

static bool adb_identify(cfw_wire_t *wire, cfw_keyboard_info_t *info) {
    memset(&state, 0, sizeof(state));
    state.info = info;
    state.has_pending = false;

    // Let keyboard settle after power-on
    cd(wire)->hw->delay_us(2000000);  // 2 seconds

    // Flush any pending data
    adb_flush(wire, ADB_ADDR_KEYBOARD);

    // Read register 3 to get handler ID
    uint16_t reg3 = adb_talk(wire, ADB_ADDR_KEYBOARD, ADB_REG_3);
    if (reg3 == 0) {
        // No response — try once more after a reset-like delay
        cd(wire)->hw->delay_us(500000);
        reg3 = adb_talk(wire, ADB_ADDR_KEYBOARD, ADB_REG_3);
        if (reg3 == 0) {
            info->type = CFW_KB_UNKNOWN;
            info->description = "No ADB keyboard detected";
            return false;
        }
    }

    state.handler_id = reg3 & 0xFF;
    info->id = reg3;
    info->bidirectional = true;
    info->has_ack = false;  // ADB doesn't have byte-level ACK
    info->scan_set = 0;     // N/A for ADB

    // Classify by handler ID
    switch (state.handler_id) {
        case ADB_HANDLER_STD:
            info->type = CFW_KB_ADB_STANDARD;
            info->description = "Apple Standard Keyboard (M0116)";
            state.is_iso = false;
            break;

        case ADB_HANDLER_AEK:
            info->type = CFW_KB_ADB_EXTENDED;
            info->description = "Apple Extended Keyboard (M0115/M3501)";
            state.is_iso = false;
            // Try switching to handler 0x03 for right modifier support
            // Listen reg 3: upper byte = address | flags, lower byte = handler
            adb_listen(wire, ADB_ADDR_KEYBOARD, ADB_REG_3,
                       (ADB_ADDR_KEYBOARD << 4) | 0x60,  // address + SRQ enable + exceptional
                       ADB_HANDLER_AEK_RMOD);
            // Verify it took
            reg3 = adb_talk(wire, ADB_ADDR_KEYBOARD, ADB_REG_3);
            if ((reg3 & 0xFF) == ADB_HANDLER_AEK_RMOD) {
                state.handler_id = ADB_HANDLER_AEK_RMOD;
                info->description = "Apple Extended Keyboard II (right modifiers enabled)";
            }
            break;

        case ADB_HANDLER_AEK_RMOD:
            info->type = CFW_KB_ADB_EXTENDED;
            info->description = "Apple Extended Keyboard (right modifiers)";
            state.is_iso = false;
            break;

        case ADB_HANDLER_STD_ISO:
            info->type = CFW_KB_ADB_ISO;
            info->description = "Apple Standard Keyboard ISO (M0118)";
            state.is_iso = true;
            break;

        case ADB_HANDLER_AEK_ISO:
            info->type = CFW_KB_ADB_ISO;
            info->description = "Apple Extended Keyboard ISO";
            state.is_iso = true;
            // Try handler 0x05 stays as ISO, no right-mod variant known
            break;

        case ADB_HANDLER_ADJUSTABLE:
            info->type = CFW_KB_ADB_EXTENDED;
            info->description = "Apple Adjustable Keyboard (M1242)";
            state.is_iso = false;
            break;

        default:
            // Unknown handler — treat as standard ADB keyboard
            info->type = CFW_KB_ADB_STANDARD;
            info->description = "ADB keyboard (unknown handler)";
            state.is_iso = false;
            break;
    }

    return true;
}

static bool adb_init(cfw_wire_t *wire, cfw_keyboard_info_t *info) {
    // Flush any stale key data
    adb_flush(wire, ADB_ADDR_KEYBOARD);
    // Read and discard any pending keystrokes
    adb_talk(wire, ADB_ADDR_KEYBOARD, ADB_REG_0);
    return true;
}

// Process a single ADB keycode into matrix coordinates.
// ADB scancode is 7 bits: row = bits 6-3, col = bits 2-0
// Bit 7 = release flag.
static bool process_adb_key(uint8_t key, uint8_t *row, uint8_t *col, bool *pressed) {
    // Power key: scancode 0x7F
    // Release: 0xFF (which is 0x7F | 0x80)
    // Both map naturally: row=15, col=7

    // ISO scancode swap:
    // ANSI key *a (0x32) and ISO key *b (0x0A) are swapped on ISO boards
    if (state.is_iso) {
        uint8_t sc = key & 0x7F;
        if (sc == 0x32) {
            key = (key & 0x80) | 0x0A;
        } else if (sc == 0x0A) {
            key = (key & 0x80) | 0x32;
        }
    }

    *col = key & 0x07;
    *row = (key >> 3) & 0x0F;
    *pressed = !(key & 0x80);
    return true;
}

static bool adb_next_event(cfw_wire_t *wire, cfw_keyboard_info_t *info,
                            uint8_t *row, uint8_t *col, bool *pressed) {
    // Drain pending second key from previous poll first
    if (state.has_pending) {
        state.has_pending = false;
        if (state.pending_key != 0xFF) {
            return process_adb_key(state.pending_key, row, col, pressed);
        }
    }

    // Poll keyboard — Talk reg 0
    uint16_t codes = adb_talk(wire, ADB_ADDR_KEYBOARD, ADB_REG_0);
    if (codes == 0) return false;

    uint8_t key0 = codes >> 8;
    uint8_t key1 = codes & 0xFF;

    // Power key: 0x7F7F = press, 0xFFFF = release
    if (codes == 0x7F7F) {
        *col = ADB_KEY_POWER & 0x07;
        *row = (ADB_KEY_POWER >> 3) & 0x0F;
        *pressed = true;
        return true;
    }
    if (codes == 0xFFFF) {
        *col = ADB_KEY_POWER & 0x07;
        *row = (ADB_KEY_POWER >> 3) & 0x0F;
        *pressed = false;
        return true;
    }

    // Error condition: key0 is 0xFF
    if (key0 == 0xFF) return false;

    // Stash second key for next call
    if (key1 != 0xFF) {
        state.has_pending = true;
        state.pending_key = key1;
    }

    return process_adb_key(key0, row, col, pressed);
}

static void adb_set_leds(cfw_wire_t *wire, cfw_keyboard_info_t *info,
                          uint8_t led_mask) {
    // CFW LED mask: bit0=NUM, bit1=CAPS, bit2=SCROLL
    // ADB reg 2 lower byte: bit0=NumLock, bit1=CapsLock, bit2=ScrollLock
    // Same order — pass through directly
    adb_listen(wire, ADB_ADDR_KEYBOARD, ADB_REG_2, 0, led_mask & 0x07);
}

// ============================================================================
// CONVERTER INSTANCE
// ============================================================================

const cfw_converter_t cfw_adb_converter = {
    .name        = "Apple Desktop Bus",
    .identify    = adb_identify,
    .init        = adb_init,
    .next_event  = adb_next_event,
    .scan_matrix = NULL,  // event-driven, not polled
    .set_leds    = adb_set_leds,
    .feedback    = NULL,
};
