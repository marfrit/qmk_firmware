#include "matrix.h"
#include "quantum.h"
#include "scancodetranslator.h"
#include "protocoladaptor.h"

void matrix_init_custom(void) {
    // TODO: initialize hardware here
    debug_enable = true;
    debug_keyboard = true;
}

bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    bool             matrix_has_changed = false;

    return matrix_has_changed;
}
