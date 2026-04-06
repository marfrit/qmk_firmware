// SPDX-License-Identifier: GPL-2.0-or-later
// QAP'LA custom matrix: reads evdev key state from the protocol layer

#include "matrix.h"
#include <string.h>

// From protocol/linux/linux.c
extern uint8_t qapla_evdev_key_state(uint16_t code);

// Matrix state: row = evdev_code / 16, col = evdev_code % 16
static matrix_row_t matrix[MATRIX_ROWS];
static matrix_row_t matrix_prev[MATRIX_ROWS];

void matrix_init(void) {
    memset(matrix, 0, sizeof(matrix));
    memset(matrix_prev, 0, sizeof(matrix_prev));
}

uint8_t matrix_scan(void) {
    memcpy(matrix_prev, matrix, sizeof(matrix_prev));
    memset(matrix, 0, sizeof(matrix));

    // Scan all evdev keycodes (0-255) and set matrix bits
    for (uint16_t code = 0; code < 256; code++) {
        if (qapla_evdev_key_state(code)) {
            uint8_t row = code / 16;
            uint8_t col = code % 16;
            if (row < MATRIX_ROWS) {
                matrix[row] |= (1 << col);
            }
        }
    }

    return memcmp(matrix, matrix_prev, sizeof(matrix)) != 0;
}

matrix_row_t matrix_get_row(uint8_t row) {
    return matrix[row];
}

void matrix_print(void) {
    // Optional debug
}
