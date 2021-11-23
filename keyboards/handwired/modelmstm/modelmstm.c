#include "modelmstm.h"
#include QMK_KEYBOARD_H

#ifdef MATRIX_HAS_GHOST
__attribute__ ((weak))
bool matrix_has_ghost_in_row(uint8_t row)
{
    matrix_row_t matrix_row = matrix_get_row(row);
    // No ghost exists when less than 2 keys are down on the row
    if (((matrix_row - 1) & matrix_row) == 0)
        return false;

    // Ghost occurs when the row shares column line with other row
    for (uint8_t i=0; i < MATRIX_ROWS; i++) {
        if (i != row && (matrix_get_row(i) & matrix_row))
            return true;
    }
    return false;
}
#endif
