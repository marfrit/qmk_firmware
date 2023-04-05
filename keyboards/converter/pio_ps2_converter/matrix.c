#include "matrix.h"
#include "ps2.h"

#define print_matrix_row(row)  print_bin_reverse8(matrix_get_row(row))
#define print_matrix_header()  print("\nr/c 01234567\n")
#define matrix_bitpop(i)       bitpop(matrix[i])
#define ROW_SHIFTER ((uint8_t)1)

static uint16_t read_keyboard_id(void);
static void matrix_make(uint8_t code);
static void matrix_break(uint8_t code);
void matrix_clear(void);
static int8_t process_cs1(uint8_t code);
static int8_t process_cs2(uint8_t code);
static int8_t process_cs3(uint8_t code);

static uint8_t matrix[MATRIX_ROWS];
#define ROW(code)      ((code>>4)&0x07)
#define COL(code)      (code&0x0F)

__attribute__((weak)) void matrix_init_kb(void) { matrix_init_user(); }

__attribute__((weak)) void matrix_scan_kb(void) { matrix_scan_user(); }

__attribute__((weak)) void matrix_init_user(void) {}

__attribute__((weak)) void matrix_scan_user(void) {}

matrix_row_t matrix_get_row(uint8_t row) {
    // TODO: return the requested row data
    return 1; //MFR
}

void matrix_print(void) {
    // TODO: use print() to dump the current matrix state to console
}

void matrix_init(void) {
    // TODO: initialize hardware and global matrix state here

    // This *must* be called for correct keyboard behavior
    matrix_init_kb();
}

uint8_t matrix_scan(void) {
    bool changed = false;

    // TODO: add matrix scanning routine here

    // Unless hardware debouncing - use the configured debounce routine
    changed = true; //MFR

    // This *must* be called for correct keyboard behavior
    matrix_scan_kb();

    return changed;
}

static uint16_t read_keyboard_id(void)
{
    uint16_t id = 0;
    int16_t  code = 0;

    // Read ID
    code = ps2_host_send(0xF2);
    if (code == -1) { id = 0xFFFF; goto DONE; }     // XT or No keyboard
    if (code != 0xFA) { id = 0xFFFE; goto DONE; }   // Broken PS/2?

    // ID takes 500ms max TechRef [8] 4-41
    code = read_wait(500);
    if (code == -1) { id = 0x0000; goto DONE; }     // AT
    id = (code & 0xFF)<<8;

    // Mouse responds with one-byte 00, this returns 00FF [y] p.14
    code = read_wait(500);
    id |= code & 0xFF;

DONE:
    // Enable
    //code = ibmpc_host_send(0xF4);

    return id;
}
