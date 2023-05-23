#include <stdint.h>
#include <stdbool.h>
#include "print.h"
#include "matrix.h"
#include "ps2.h"
#include "timer.h"
#include "action.h"
#include "host.h"
#include "debug.h"
#include "util.h"
#include "pio_ps2_converter.h"

// buffer must have length >= sizeof(int) + 1
// Write to the buffer backwards so that the binary representation
// is in the correct order i.e.  the LSB is on the far right
// instead of the far left of the printed string
char *int2bin(uint16_t a, char *buffer, int buf_size) {
    buffer += (buf_size - 1);

    for (int i = BUF_SIZE - 2; i >= 0; i--) {
        *buffer-- = (a & 1) + '0';

        a >>= 1;
    }

    return buffer;
}

#define BUF_SIZE (MARTRIX_ROWS + 1)

#define print_matrix_row(row) print_bin_reverse8(matrix_get_row(row))
#define print_matrix_header() print("\nr/c 01234567\n")
#define matrix_bitpop(i) bitpop(matrix[i])
#define ROW_SHIFTER ((uint8_t)1)

#define ID_STR(id) (id == 0xFFFE ? "_????" : (id == 0xFFFD ? "_Z150" : (id == 0x0000 ? "_AT84" : "")))

int16_t         read_wait(uint16_t wait_ms);
static uint16_t read_keyboard_id(void);
static void     matrix_make(uint8_t code);
static void     matrix_break(uint8_t code);
void            matrix_clear(void);
static uint8_t  cs2_e0code(uint8_t code);
static int8_t   process_cs2(uint8_t code);
static int8_t   process_cs3(uint8_t code);
static uint8_t  translate_5576_cs2(uint8_t code);
static uint8_t  translate_5576_cs2_e0(uint8_t code);
static uint8_t  translate_5576_cs3(uint8_t code);
static uint8_t  translate_televideo_dec_cs3(uint8_t code);

static matrix_row_t matrix[MATRIX_ROWS];
uint16_t            keyboard_id      = 0x0000;
uint8_t             current_protocol = 0;
keyboard_kind_t     keyboard_kind    = NONE;

#define ROW(code) ((code >> 4) & 0x07)
#define COL(code) (code & 0x0F)

__attribute__((weak)) void matrix_init_kb(void) {
    matrix_init_user();
}

__attribute__((weak)) void matrix_scan_kb(void) {
    matrix_scan_user();
}

__attribute__((weak)) void matrix_init_user(void) {}

__attribute__((weak)) void matrix_scan_user(void) {}

static uint16_t read_keyboard_id(void) {
    uint16_t id   = 0;
    uint8_t  code = 0;

    // Read ID
    code = ps2_host_send(0xF2);

    if (code == -1) {
        id = 0xFFFF;
        goto DONE;
    } // No keyboard
    if (code != 0xFA) {
        id = 0xFFFE;
        goto DONE;
    } // Broken PS/2?

    // ID takes 500ms max TechRef [8] 4-41
    code = read_wait(500);

    if (code == -1) {
        id = 0x0000;
        goto DONE;
    } // AT
    id = (code & 0xFF) << 8;

    // Mouse responds with one-byte 00, this returns 00FF [y] p.14
    code = read_wait(500);

    id |= code & 0xFF;

DONE:
    // Enable
    // code = ps2_host_send(0xF4);

    return id;
}

int16_t read_wait(uint16_t wait_ms) {
    uint16_t start = timer_read();
    int8_t   code;
    while ((code = ps2_host_recv()) == 0 && timer_elapsed(start) < wait_ms)
        ;
    return code;
}

static int8_t process_cs2(uint8_t code) {
    // scan code reading states
    static enum {
        INIT,
        cF0,
        cE0,
        E0_F0,
        // Pause
        cE1,
        E1_14,
        E1_F0,
        E1_F0_14,
        E1_F0_14_F0,
    } state = INIT;

    switch (state) {
        case INIT:
            if (0xAB90 == keyboard_id || 0xAB91 == keyboard_id) {
                code = translate_5576_cs2(code);
            }
            switch (code) {
                case 0xE0:
                    state = cE0;
                    break;
                case 0xF0:
                    state = cF0;
                    break;
                case 0xE1:
                    state = cE1;
                    break;
                case 0x83: // F7
                    matrix_make(0x02);
                    state = INIT;
                    break;
                case 0x84: // Alt'd PrintScreen
                    matrix_make(0x7F);
                    state = INIT;
                    break;
                case 0xAA: // Self-test passed
                case 0xFC: // Self-test failed
                           // replug or unstable connection probably
                default: // normal key make
                    state = INIT;
                    if (code < 0x80) {
                        matrix_make(code);
                    } else {
                        matrix_clear();
                        xprintf("!CS2_INIT!\n");
                        return -1;
                    }
            }
            break;
        case cE0: // E0-Prefixed
            if (0xAB90 == keyboard_id || 0xAB91 == keyboard_id) {
                code = translate_5576_cs2_e0(code);
            }
            switch (code) {
                case 0x12: // to be ignored
                case 0x59: // to be ignored
                    state = INIT;
                    break;
                case 0xF0:
                    state = E0_F0;
                    break;
                default:
                    state = INIT;
                    if (code < 0x80) {
                        matrix_make(cs2_e0code(code));
                    } else {
                        matrix_clear();
                        xprintf("!CS2_E0!\n");
                        return -1;
                    }
            }
            break;
        case cF0: // Break code
            if (0xAB90 == keyboard_id || 0xAB91 == keyboard_id) {
                code = translate_5576_cs2(code);
            }
            switch (code) {
                case 0x83: // F7
                    matrix_break(0x02);
                    state = INIT;
                    break;
                case 0x84: // Alt'd PrintScreen
                    matrix_break(0x7F);
                    state = INIT;
                    break;
                default:
                    state = INIT;
                    if (code < 0x80) {
                        matrix_break(code);
                    } else {
                        matrix_clear();
                        xprintf("!CS2_F0!\n");
                        return -1;
                    }
            }
            break;
        case E0_F0: // Break code of E0-prefixed
            if (0xAB90 == keyboard_id || 0xAB91 == keyboard_id) {
                code = translate_5576_cs2_e0(code);
            }
            switch (code) {
                case 0x12: // to be ignored
                case 0x59: // to be ignored
                    state = INIT;
                    break;
                default:
                    state = INIT;
                    if (code < 0x80) {
                        matrix_break(cs2_e0code(code));
                    } else {
                        matrix_clear();
                        xprintf("!CS2_E0_F0!\n");
                        return -1;
                    }
            }
            break;
        // Pause make: E1 14 77
        case cE1:
            switch (code) {
                case 0x14:
                    state = E1_14;
                    break;
                case 0xF0:
                    state = E1_F0;
                    break;
                default:
                    state = INIT;
            }
            break;
        case E1_14:
            switch (code) {
                case 0x77:
                    matrix_make(0x00);
                    state = INIT;
                    break;
                default:
                    state = INIT;
            }
            break;
        // Pause break: E1 F0 14 F0 77
        case E1_F0:
            switch (code) {
                case 0x14:
                    state = E1_F0_14;
                    break;
                default:
                    state = INIT;
            }
            break;
        case E1_F0_14:
            switch (code) {
                case 0xF0:
                    state = E1_F0_14_F0;
                    break;
                default:
                    state = INIT;
            }
            break;
        case E1_F0_14_F0:
            switch (code) {
                case 0x77:
                    matrix_break(0x00);
                    state = INIT;
                    break;
                default:
                    state = INIT;
            }
            break;
        default:
            state = INIT;
    }
    return 0;
}

// IBM 5576-002/003 Scan code translation
// https://github.com/tmk/tmk_keyboard/wiki/IBM-PC-AT-Keyboard-Protocol#ibm-5576-code-set-82h
static uint8_t translate_5576_cs2(uint8_t code) {
    switch (code) {
        case 0x11:
            return 0x0F; // Zenmen   -> RALT
        case 0x13:
            return 0x11; // Kanji    -> LALT
        case 0x0E:
            return 0x54; // @
        case 0x54:
            return 0x5B; // [
        case 0x5B:
            return 0x5D; // ]
        case 0x5C:
            return 0x6A; // JYEN
        case 0x5D:
            return 0x6A; // JYEN
        case 0x62:
            return 0x0E; // Han/Zen  -> `~
        case 0x7C:
            return 0x77; // Keypad *
    }
    return code;
}
static uint8_t translate_5576_cs2_e0(uint8_t code) {
    switch (code) {
        case 0x11:
            return 0x13; // Hiragana -> KANA
        case 0x41:
            return 0x7C; // Keypad '
    }
    return code;
}

/*******************************************************************************
 * AT, PS/2: Scan Code Set 2
 *
 * Exceptional Handling
 * --------------------
 * Some keys should be handled exceptionally. See [b].
 *
 * Scan codes are varied or prefix/postfix'd depending on modifier key state.
 *
 * 1) Insert, Delete, Home, End, PageUp, PageDown, Up, Down, Right, Left
 *     a) when Num Lock is off
 *     modifiers | make                      | break
 *     ----------+---------------------------+----------------------
 *     Ohter     |                    <make> | <break>
 *     LShift    | E0 F0 12           <make> | <break>  E0 12
 *     RShift    | E0 F0 59           <make> | <break>  E0 59
 *     L+RShift  | E0 F0 12  E0 F0 59 <make> | <break>  E0 59 E0 12
 *
 *     b) when Num Lock is on
 *     modifiers | make                      | break
 *     ----------+---------------------------+----------------------
 *     Other     | E0 12              <make> | <break>  E0 F0 12
 *     Shift'd   |                    <make> | <break>
 *
 *     Handling: These prefix/postfix codes are ignored.
 *
 *
 * 2) Keypad /
 *     modifiers | make                      | break
 *     ----------+---------------------------+----------------------
 *     Ohter     |                    <make> | <break>
 *     LShift    | E0 F0 12           <make> | <break>  E0 12
 *     RShift    | E0 F0 59           <make> | <break>  E0 59
 *     L+RShift  | E0 F0 12  E0 F0 59 <make> | <break>  E0 59 E0 12
 *
 *     Handling: These prefix/postfix codes are ignored.
 *
 *
 * 3) PrintScreen
 *     modifiers | make         | break
 *     ----------+--------------+-----------------------------------
 *     Other     | E0 12  E0 7C | E0 F0 7C  E0 F0 12
 *     Shift'd   |        E0 7C | E0 F0 7C
 *     Control'd |        E0 7C | E0 F0 7C
 *     Alt'd     |           84 | F0 84
 *
 *     Handling: These prefix/postfix codes are ignored, and both scan codes
 *               'E0 7C' and 84 are seen as PrintScreen.
 *
 * 4) Pause
 *     modifiers | make(no break code)
 *     ----------+--------------------------------------------------
 *     Other     | E1 14 77 E1 F0 14 F0 77
 *     Control'd | E0 7E E0 F0 7E
 *
 *     Handling: Both code sequences are treated as a whole.
 *               And we need a ad hoc 'pseudo break code' hack to get the key off
 *               because it has no break code.
 *
 * Notes:
 * 'Hanguel/English'(F1) and 'Hanja'(F2) have no break code. See [a].
 * These two Korean keys need exceptional handling and are not supported for now.
 *
 */
static uint8_t cs2_e0code(uint8_t code) {
    switch (code) {
        // E0 prefixed codes translation See [a].
        case 0x11:
            return 0x0F; // right alt
        case 0x14:
            return 0x17; // right control
        case 0x1F:
            return 0x19; // left GUI
        case 0x27:
            return 0x1F; // right GUI
        case 0x2F:
            return 0x5C; // apps
        case 0x4A:
            return 0x60; // keypad /
        case 0x5A:
            return 0x62; // keypad enter
        case 0x69:
            return 0x27; // end
        case 0x6B:
            return 0x53; // cursor left
        case 0x6C:
            return 0x2F; // home
        case 0x70:
            return 0x39; // insert
        case 0x71:
            return 0x37; // delete
        case 0x72:
            return 0x3F; // cursor down
        case 0x74:
            return 0x47; // cursor right
        case 0x75:
            return 0x4F; // cursor up
        case 0x7A:
            return 0x56; // page down
        case 0x7D:
            return 0x5E; // page up
        case 0x7C:
            return 0x7F; // Print Screen
        case 0x7E:
            return 0x00; // Control'd Pause

        case 0x21:
            return 0x65; // volume down
        case 0x32:
            return 0x6E; // volume up
        case 0x23:
            return 0x6F; // mute
        case 0x10:
            return 0x08; // (WWW search)     -> F13
        case 0x18:
            return 0x10; // (WWW favourites) -> F14
        case 0x20:
            return 0x18; // (WWW refresh)    -> F15
        case 0x28:
            return 0x20; // (WWW stop)       -> F16
        case 0x30:
            return 0x28; // (WWW forward)    -> F17
        case 0x38:
            return 0x30; // (WWW back)       -> F18
        case 0x3A:
            return 0x38; // (WWW home)       -> F19
        case 0x40:
            return 0x40; // (my computer)    -> F20
        case 0x48:
            return 0x48; // (email)          -> F21
        case 0x2B:
            return 0x50; // (calculator)     -> F22
        case 0x34:
            return 0x08; // (play/pause)     -> F13
        case 0x3B:
            return 0x10; // (stop)           -> F14
        case 0x15:
            return 0x18; // (previous track) -> F15
        case 0x4D:
            return 0x20; // (next track)     -> F16
        case 0x50:
            return 0x28; // (media select)   -> F17
        case 0x5E:
            return 0x50; // (ACPI wake)      -> F22
        case 0x3F:
            return 0x57; // (ACPI sleep)     -> F23
        case 0x37:
            return 0x5F; // (ACPI power)     -> F24

        // https://github.com/tmk/tmk_keyboard/pull/636
        case 0x03:
            return 0x18; // Help        DEC LK411 -> F15
        case 0x04:
            return 0x08; // F13         DEC LK411
        case 0x0B:
            return 0x20; // Do          DEC LK411 -> F16
        case 0x0C:
            return 0x10; // F14         DEC LK411
        case 0x0D:
            return 0x19; // LCompose    DEC LK411 -> LGUI
        case 0x79:
            return 0x6D; // KP-         DEC LK411 -> PCMM
        case 0x83:
            return 0x28; // F17         DEC LK411
        default:
            return (code & 0x7F);
    }
}

static int8_t process_cs3(uint8_t code) {
    static enum {
        READY,
        cF0,
#ifdef G80_2551_SUPPORT
        // G80-2551 four extra keys around cursor keys
        G80,
        G80_F0,
#endif
    } state = READY;

    switch (code) {
        case 0xAA: // BAT code
        case 0xFC: // BAT code
        case 0xBF: // Part of keyboard ID
        case 0xAB: // Part keyboard ID
            state = READY;
            xprintf("!CS3_RESET!\n");
            return -1;
    }

    switch (state) {
        case READY:
            if (0xAB92 == keyboard_id) {
                code = translate_5576_cs3(code);
            }
            if (0xAB91 == keyboard_id) {
                // This must be the Televideo DEC keyboard. (For 5576-003 we don't use scan code set 3)
                code = translate_televideo_dec_cs3(code);
            }
            switch (code) {
                case 0xF0:
                    state = cF0;
                    break;
                case 0x83: // PrintScreen
                    matrix_make(0x02);
                    break;
                case 0x84: // Keypad *
                    matrix_make(0x7F);
                    break;
                case 0x85: // Muhenkan
                    matrix_make(0x68);
                    break;
                case 0x86: // Henkan
                    matrix_make(0x78);
                    break;
                case 0x87: // Hiragana
                    matrix_make(0x00);
                    break;
                case 0x8B: // Left GUI
                    matrix_make(0x01);
                    break;
                case 0x8C: // Right GUI
                    matrix_make(0x09);
                    break;
                case 0x8D: // Application
                    matrix_make(0x0A);
                    break;
#ifdef G80_2551_SUPPORT
                case 0x80: // G80-2551 four extra keys around cursor keys
                    state = G80;
                    break;
#endif
                default: // normal key make
                    if (code < 0x80) {
                        matrix_make(code);
                    } else {
                        xprintf("!CS3_READY!\n");
                    }
            }
            break;
        case cF0: // Break code
            state = READY;
            if (0xAB92 == keyboard_id) {
                code = translate_5576_cs3(code);
            }
            if (0xAB91 == keyboard_id) {
                // This must be the Televideo DEC keyboard. (For 5576-003 we don't use scan code set 3)
                code = translate_televideo_dec_cs3(code);
            }
            switch (code) {
                case 0x83: // PrintScreen
                    matrix_break(0x02);
                    break;
                case 0x84: // Keypad *
                    matrix_break(0x7F);
                    break;
                case 0x85: // Muhenkan
                    matrix_break(0x0B);
                    break;
                case 0x86: // Henkan
                    matrix_break(0x06);
                    break;
                case 0x87: // Hiragana
                    matrix_break(0x00);
                    break;
                case 0x8B: // Left GUI
                    matrix_break(0x01);
                    break;
                case 0x8C: // Right GUI
                    matrix_break(0x09);
                    break;
                case 0x8D: // Application
                    matrix_break(0x0A);
                    break;
                default:

                    if (code < 0x80) {
                        matrix_break(code);
                    } else {
                        xprintf("!CS3_F0!\n");
                    }
            }
            break;
#ifdef G80_2551_SUPPORT
        /*
         * G80-2551 terminal keyboard support
         * https://deskthority.net/wiki/Cherry_G80-2551
         * https://github.com/tmk/tmk_keyboard/wiki/IBM-PC-AT-Keyboard-Protocol#g80-2551-in-code-set-3
         */
        case G80: // G80-2551 four extra keys around cursor keys
            switch (code) {
                case (0x26): // TD= -> JYEN
                    matrix_make(0x5D);
                    break;
                case (0x25): // page with edge -> NUHS
                    matrix_make(0x53);
                    break;
                case (0x16): // two pages -> RO
                    matrix_make(0x51);
                    break;
                case (0x1E): // calc -> KANA
                    matrix_make(0x00);
                    break;
                case (0xF0):
                    state = G80_F0;
                    return 0;
                default:
                    // Not supported
                    matrix_clear();
                    break;
            }
            state = READY;
            break;
        case G80_F0:
            switch (code) {
                case (0x26): // TD= -> JYEN
                    matrix_break(0x5D);
                    break;
                case (0x25): // page with edge -> NUHS
                    matrix_break(0x53);
                    break;
                case (0x16): // two pages -> RO
                    matrix_break(0x51);
                    break;
                case (0x1E): // calc -> KANA
                    matrix_break(0x00);
                    break;
                default:
                    // Not supported
                    matrix_clear();
                    break;
            }
            state = READY;
            break;
#endif
    }
    return 0;
}

/*
 * Terminal: Scan Code Set 3
 *
 * See [3], [7] and
 * https://github.com/tmk/tmk_keyboard/wiki/IBM-PC-AT-Keyboard-Protocol#scan-code-set-3
 */
// IBM 5576-001 Scan code translation
// https://github.com/tmk/tmk_keyboard/wiki/IBM-PC-AT-Keyboard-Protocol#ibm-5576-code-set-3
static uint8_t translate_5576_cs3(uint8_t code) {
    switch (code) {
        // Fix positon of keys to fit 122-key layout
        case 0x13:
            return 0x5D; // JYEN
        case 0x5C:
            return 0x51; // RO
        case 0x76:
            return 0x7E; // Keypad '
        case 0x7E:
            return 0x76; // Keypad Dup
    }
    return code;
}

// Televideo DEC Scan code translation
static uint8_t translate_televideo_dec_cs3(uint8_t code) {
    switch (code) {
        case 0x08:
            return 0x76; // Esc
        case 0x8D:
            return 0x77; // Num Lock
        case 0x8E:
            return 0x67; // Numeric Keypad Slash
        case 0x8F:
            return 0x7F; // Numeric Keypad Asterisk
        case 0x90:
            return 0x7B; // Numeric Keypad Minus
        case 0x6E:
            return 0x65; // Insert
        case 0x65:
            return 0x6d; // Delete
        case 0x67:
            return 0x62; // Home
        case 0x6d:
            return 0x64; // End
        case 0x64:
            return 0x6e; // PageUp
        case 0x84:
            return 0x7c; // Numeric Keypad Plus (Legend says minus)
        case 0x87:
            return 0x02; // Print Screen
        case 0x88:
            return 0x7e; // Scroll Lock
        case 0x89:
            return 0x0c; // Pause
        case 0x8A:
            return 0x03; // VOLD
        case 0x8B:
            return 0x04; // VOLU
        case 0x8C:
            return 0x05; // MUTE
        case 0x85:
            return 0x08; // F13
        case 0x86:
            return 0x10; // F14
        case 0x91:
            return 0x01; // LGUI
        case 0x92:
            return 0x09; // RGUI
        case 0x77:
            return 0x58; // RCTRL
        case 0x57:
            return 0x5C; // Backslash
        case 0x5C:
            return 0x53; // Non-US Hash
        case 0x7c:
            return 0x68; // Kp Comma
    }
    return code;
}

uint8_t matrix_scan(void) {
    // scan code reading states
    static enum {
        INIT,
        WAIT_SETTLE,
        AT_RESET,
        WAIT_AA,
        WAIT_AABF,
        WAIT_AABFBF,
        READ_ID,
        SETUP,
        LOOP,
        ERROR,
    } state = INIT;

    bool            changed = false;
    static uint16_t init_time;

    if (ps2_error && !PS2_ERR_NODATA) {
        xprintf("\n%u ERR:%02X ", timer_read(), ps2_error);

        // when recv error, neither send error nor buffer full
        if (!(ps2_error)) {
            // keyboard init again
            if (state == LOOP) {
                xprintf("[RST] ");
                state = ERROR;
            }
        }

        // clear or process error
        ps2_error = PS2_ERR_NONE;
    }

    switch (state) {
        case INIT:
            xprintf("I%u ", timer_read());
            keyboard_kind    = NONE;
            keyboard_id      = 0x0000;
            current_protocol = 0;

            matrix_clear();

            init_time = timer_read();
            state     = WAIT_SETTLE;
            break;
        case WAIT_SETTLE:
            while (ps2_host_recv() != 0)
                ; // read data
            // wait for keyboard to settle after plugin
            if (timer_elapsed(init_time) > 3000) {
                state = AT_RESET;
            }
            break;
        case AT_RESET:
            xprintf("A%u ", timer_read());

            // and keeps it until receiving reset. Sending reset here may be useful to clear it, perhaps.
            // https://github.com/tmk/tmk_keyboard/wiki/IBM-PC-AT-Keyboard-Protocol#select-alternate-scan-codesf0

            // reset command
            if (0xFA == ps2_host_send(0xFF)) {
                state = WAIT_AA;
            }
            break;
        case WAIT_AA:
            // 1) Read BAT code and ID on keybaord power-up
            // For example, XT/AT sends 'AA' and Terminal sends 'AA BF BF' after BAT
            // AT 84-key: POR and BAT can take 900-9900ms according to AT TechRef [8] 4-7
            // AT 101/102-key: POR and BAT can take 450-2500ms according to AT TechRef [8] 4-39
            // 2) Read key typed by user or anything after error on protocol or scan code
            // This can happen in case of keyboard hotswap, unstable hardware, signal integrity problem or bug

            /* wait until keyboard sends any code without 10000ms timeout
            if (timer_elapsed(init_time) > 10000) {
                state = READ_ID;
            }
            */
            if (ps2_host_recv() != 0) { // wait for AA
                xprintf("W%u ", timer_read());
                init_time = timer_read();
                state     = WAIT_AABF;
            }
            break;
        case WAIT_AABF:
            // NOTE: we can omit to wait BF BF
            // ID takes 500ms max? TechRef [8] 4-41, though 1ms is enough for 122-key Terminal 6110345
            if (timer_elapsed(init_time) > 500) {
                state = READ_ID;
            }
            if (ps2_host_recv() != 0) { // wait for BF
                xprintf("W%u ", timer_read());
                init_time = timer_read();
                state     = WAIT_AABFBF;
            }
            break;
        case WAIT_AABFBF:
            if (timer_elapsed(init_time) > 500) {
                state = READ_ID;
            }
            if (ps2_host_recv() != 0) { // wait for BF
                xprintf("W%u ", timer_read());
                state = READ_ID;
            }
            break;
        case READ_ID:
            keyboard_id = read_keyboard_id();
            xprintf("R%u ", timer_read());

            if (0x0000 == keyboard_id) { // CodeSet2 AT(IBM PC AT 84-key)
                keyboard_kind = PC_AT;
            } else if (0xFFFE == keyboard_id) { // CodeSet2 PS/2 fails to response?
                keyboard_kind = PC_AT;
            } else if (0xFFFD == keyboard_id) { // Zenith Z-150 AT
                keyboard_kind = PC_AT;
            } else if (0xAB85 == keyboard_id || // IBM 122-key Model M, NCD N-97
                       0xAB86 == keyboard_id || // Cherry G80-2551, IBM 1397000
                       0xAB92 == keyboard_id) { // IBM 5576-001
                // https://github.com/tmk/tmk_keyboard/wiki/IBM-PC-AT-Keyboard-Protocol#ab85
                // https://github.com/tmk/tmk_keyboard/wiki/IBM-PC-AT-Keyboard-Protocol#ab86
                // https://github.com/tmk/tmk_keyboard/wiki/IBM-PC-AT-Keyboard-Protocol#ab92

                if ((0xFA == ps2_host_send(0xF0)) && (0xFA == ps2_host_send(0x03))) {
                    // switch to code set 3
                    keyboard_kind = PC_TERMINAL;
                } else {
                    keyboard_kind = PC_AT;
                }
            } else if (0xAB90 == keyboard_id || // IBM 5576-002
                       0xAB91 == keyboard_id) { // IBM 5576-003 or Televideo DEC
                // https://github.com/tmk/tmk_keyboard/wiki/IBM-PC-AT-Keyboard-Protocol#ab90
                // https://github.com/tmk/tmk_keyboard/wiki/IBM-PC-AT-Keyboard-Protocol#ab91

                xprintf("\n5576_CS82h:");
                keyboard_kind = PC_AT;
                if ((0xFA == ps2_host_send(0xF0)) && (0xFA == ps2_host_send(0x82))) {
                    // switch to code set 82h
                    // https://github.com/tmk/tmk_keyboard/wiki/IBM-PC-AT-Keyboard-Protocol#ibm-5576-scan-codes-set
                    xprintf("OK ");
                } else {
                    xprintf("NG ");
                    if (0xAB91 == keyboard_id) {
                        // This must be a Televideo DEC keyboard, which piggybacks on the same keyboard_id as IBM 5576-003
                        // This keyboard normally starts up using code set 1, but we request code set 2 here:
                        if ((0xFA == ps2_host_send(0xF0)) && (0xFA == ps2_host_send(0x03))) {
                            xprintf("OK ");
                            keyboard_kind = PC_TERMINAL;
                        } else {
                            xprintf("NG ");
                        }
                    }
                }
            } else if (0xBFB0 == keyboard_id) { // IBM RT Keyboard
                // https://github.com/tmk/tmk_keyboard/wiki/IBM-PC-AT-Keyboard-Protocol#bfb0
                // TODO: LED indicator fix
                // keyboard_kind = PC_TERMINAL_IBM_RT;
                keyboard_kind = PC_TERMINAL;
            } else if (0xAB00 == (keyboard_id & 0xFF00)) { // CodeSet2 PS/2
                keyboard_kind = PC_AT;
            } else if (0xBF00 == (keyboard_id & 0xFF00)) { // CodeSet3 Terminal
                keyboard_kind = PC_TERMINAL;
            } else if (0x7F00 == (keyboard_id & 0xFF00)) { // CodeSet3 Terminal 1394204
                keyboard_kind = PC_TERMINAL;
            } else {
                xprintf("\nUnknown ID: Report ");
                if ((0xFA == ps2_host_send(0xF0)) && (0xFA == ps2_host_send(0x02))) {
                    // switch to code set 2
                    keyboard_kind = PC_AT;
                } else if ((0xFA == ps2_host_send(0xF0)) && (0xFA == ps2_host_send(0x03))) {
                    // switch to code set 3
                    keyboard_kind = PC_TERMINAL;
                } else {
                    keyboard_kind = PC_AT;
                }
            }

            xprintf("\nID:%04X(%s%s) ", keyboard_id, KEYBOARD_KIND_STR(keyboard_kind), ID_STR(keyboard_id));

            state = SETUP;
            break;
        case SETUP:
            xprintf("S%u ", timer_read());
            switch (keyboard_kind) {
                case PC_AT:
                    led_set(host_keyboard_leds());
                    break;
                case PC_TERMINAL:
                    // Set all keys to make/break type
                    ps2_host_send(0xF8);
                    // This should not be hankkkrmful
                    led_set(host_keyboard_leds());
                    break;
                default:
                    break;
            }
            state = LOOP;
            xprintf("L%u ", timer_read());
        case LOOP: {
            uint8_t code = ps2_host_recv();
            if (!code) {
                // no code
                break;
            }

            // Keyboard Error/Overrun([3]p.26) or Buffer full
            // Scan Code Set 1: 0xFF
            // Scan Code Set 2 and 3: 0x00
            // Buffer full(ps2_ERR_FULL): 0xFF
            if (code == 0xFF) {
                // clear stuck keys
                matrix_clear();
                clear_keyboard();

                xprintf("\n[CLR] ");
                break;
            }

            switch (keyboard_kind) {
                case PC_AT:
                    if (process_cs2(code) == -1) state = ERROR;
                    break;
                case PC_TERMINAL:
                    if (process_cs3(code) == -1) state = ERROR;
                    break;
                default:
                    break;
            }
        } break;
        case ERROR:
            // something goes wrong
            clear_keyboard();
            state = INIT;
            break;
        default:
            break;
    }
    changed = true;
    return (uint8_t)changed;
}

inline bool matrix_is_on(uint8_t row, uint8_t col) {
    return (matrix[row] & (1 << col));
}

void matrix_clear(void) {
    for (uint8_t i = 0; i < MATRIX_ROWS; i++)
        matrix[i] = 0x00;
}

inline matrix_row_t matrix_get_row(uint8_t row) {
    return matrix[row];
}

uint8_t matrix_key_count(void) {
    uint8_t count = 0;
    for (uint8_t i = 0; i < MATRIX_ROWS; i++) {
        count += bitpop(matrix[i]);
    }
    return count;
}

inline static uint8_t to_unimap(uint8_t code) {
    uint8_t row = ROW(code);
    uint8_t col = COL(code);
    xprintf("Before unimap: %02X,", code);
    switch (keyboard_kind) {
        case PC_AT:
            return pgm_read_byte(&unimap_cs2[row][col]);
        case PC_TERMINAL:
            return pgm_read_byte(&unimap_cs3[row][col]);
        default:
            return UNIMAP_NO;
    }
}

inline static void matrix_make(uint8_t code) {
    uint8_t newcode = to_unimap(code);
    xprintf(" after unimap: %02X\n", newcode);
    if (!matrix_is_on(ROW(newcode), COL(newcode))) {
        matrix[ROW(newcode)] |= 1 << COL(newcode);
        matrix_print();
    }
}

inline static void matrix_break(uint8_t code) {
    uint8_t newcode = to_unimap(code);
    if (matrix_is_on(ROW(newcode), COL(newcode))) {
        matrix[ROW(newcode)] &= ~(1 << COL(newcode));
    }
}

void matrix_init(void) {
    debug_enable = true;
    debug_matrix = true;
    wait_ms(2500);
    xprintf("TURNING ON POWER\n");
    setPinOutput(POWERPIN1);
    writePinHigh(POWERPIN1);
    wait_ms(100);
    setPinOutput(POWERPIN2);
    writePinHigh(POWERPIN2);

    ps2_host_init();
    xprintf("PS/2 INITIALIZED\n");

    wait_ms(2000);

    // initialize matrix state: all keys off
    for (uint8_t i = 0; i < MATRIX_ROWS; i++)
        matrix[i] = 0x00;

    matrix_init_kb();
    xprintf("KEYBOARD INITIALIZED\n");
    return;
}

void matrix_print(void) {
    char buffer[BUF_SIZE];
    buffer[BUF_SIZE - 1] = '\0';
#if (MATRIX_COLS <= 8)
    print("r/c 01234567\n");
#elif (MATRIX_COLS <= 16)
    print("r/c 0123456789ABCDEF\n");
#elif (MATRIX_COLS <= 32)
    print("r/c 0123456789ABCDEF0123456789ABCDEF\n");
#endif

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        buffer[BUF_SIZE - 1] = '\0';
#if (MATRIX_COLS <= 8)
        int2bin(bitrev(matrix_get_row(row)), buffer, BUF_SIZE - 1);
        xprintf("%02X: %s%s\n", row, buffer,
#elif (MATRIX_COLS <= 16)
        int2bin(bitrev16(matrix_get_row(row)), buffer, BUF_SIZE - 1);
        xprintf("%02X: %s%s\n", row, buffer,
#elif (MATRIX_COLS <= 32)
        int2bin(bitrev32(matrix_get_row(row)), buffer, BUF_SIZE - 1);
        xprintf("%02X: %s%s\n", row, buffer,
#endif
#ifdef MATRIX_HAS_GHOST
                matrix_has_ghost_in_row(row) ? " <ghost" : ""
#else
                ""
#endif
        );
    }
}

#ifdef MATRIX_HAS_GHOST
__attribute__((weak)) bool matrix_has_ghost_in_row(uint8_t row) {
    matrix_row_t matrix_row = matrix_get_row(row);
    // No ghost exists when less than 2 keys are down on the row
    if (((matrix_row - 1) & matrix_row) == 0) return false;

    // Ghost occurs when the row shares column line with other row
    for (uint8_t i = 0; i < MATRIX_ROWS; i++) {
        if (i != row && (matrix_get_row(i) & matrix_row)) return true;
    }
    return false;
}
#endif
