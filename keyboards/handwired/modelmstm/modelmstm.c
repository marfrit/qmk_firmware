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

//#ifdef HAS_LEDS
void keyboard_pre_init_user(void) {
  // Call the keyboard pre init code.

  // Set our LED pins as output
//   for (int i=0, i<4, i++) {
//       setPinOutput(F + i);
//   }
  setPinOutput(B0);  // Scroll Lock
  setPinOutput(B1);  // Caps Lock
  setPinOutput(B10);  // Num Lock
 // setPinOutput(F3);  // Ground

  writePinLow(B0);
  writePinLow(B1);
  writePinLow(B10);
//  writePinHigh(F3);
}

bool led_update_kb(led_t led_state) {
    bool res = led_update_user(led_state);
    if(res) {
        // writePin sets the pin high for 1 and low for 0.
        // In this example the pins are inverted, setting
        // it low/0 turns it on, and high/1 turns the LED off.
        // This behavior depends on whether the LED is between the pin
        // and VCC or the pin and GND.
        // writePinLow(F3);
        writePin(B0, !led_state.scroll_lock);
        writePin(B1, !led_state.caps_lock);
        writePin(B10, !led_state.num_lock);

    }
    return res;
}
//#endif
