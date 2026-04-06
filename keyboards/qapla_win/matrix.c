// SPDX-License-Identifier: GPL-2.0-or-later
// QAP'LA Windows custom matrix — reads from LLHOOK key state

#include "matrix.h"
#include <string.h>

extern uint8_t qapla_key_state(uint16_t code);

static matrix_row_t matrix[MATRIX_ROWS];
static matrix_row_t matrix_prev[MATRIX_ROWS];

void matrix_init(void) {
    memset(matrix, 0, sizeof(matrix));
    memset(matrix_prev, 0, sizeof(matrix_prev));
}

uint8_t matrix_scan(void) {
    memcpy(matrix_prev, matrix, sizeof(matrix_prev));
    memset(matrix, 0, sizeof(matrix));

    for (uint16_t vk = 0; vk < 256; vk++) {
        if (qapla_key_state(vk)) {
            uint8_t row = vk / 16;
            uint8_t col = vk % 16;
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

void matrix_print(void) {}
