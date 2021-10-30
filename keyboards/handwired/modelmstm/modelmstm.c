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
