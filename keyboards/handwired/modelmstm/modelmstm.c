#include "modelmstm.h"
#include QMK_KEYBOARD_H

//#define MATRIX_ROW_PINS { A7, A6, A5, A4, A3, A2, A1, A0 }
//#define MATRIX_COL_PINS { B12, B13, B14, B15, A8, A9, A10, A11, A12, A15, B3, B4, B4, B6, B7, B8 }

void board_init(void) {
    // B9 is configured as I2C1_SDA in the board file; that function must be
    // disabled before using B7 as I2C1_SDA.
/*    setPinInputHigh(B12);
    setPinInputHigh(B13);
    setPinInputHigh(B14);
    setPinInputHigh(B15);
    setPinInputHigh(A8);
    setPinInputHigh(A9);
    setPinInputHigh(A10);
    setPinInputHigh(A11);
    setPinInputHigh(A12);
    setPinInputHigh(A15);
    setPinInputHigh(B3);
    setPinInputHigh(B4);
    setPinInputHigh(B5);
    setPinInputHigh(B6);
    setPinInputHigh(B7);
    setPinInputHigh(B8);*/
}

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
