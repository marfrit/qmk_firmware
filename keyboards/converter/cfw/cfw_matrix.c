// SPDX-License-Identifier: GPL-2.0-or-later
// CFW — QMK matrix integration
// Generic matrix_init/matrix_scan that delegates to the active converter

#include "cfw.h"
#include <string.h>

static const cfw_converter_t *active_converter;
static cfw_wire_t *active_wire;
static cfw_keyboard_info_t kb_info;
static bool initialized = false;

void cfw_matrix_init(const cfw_converter_t *conv, cfw_wire_t *wire) {
    active_converter = conv;
    active_wire = wire;
    memset(&kb_info, 0, sizeof(kb_info));
    kb_info.id = 0xFFFF;

    // Identify keyboard (one-shot, boot-time only)
    if (conv->identify) {
        if (!conv->identify(wire, &kb_info)) {
            kb_info.type = CFW_KB_UNKNOWN;
            kb_info.description = "Unidentified keyboard";
        }
    }

    // Initialize keyboard
    if (conv->init) {
        conv->init(wire, &kb_info);
    }

    initialized = true;
}

uint8_t cfw_matrix_scan(matrix_row_t matrix[]) {
    if (!initialized || !active_converter) return 0;

    // Event-driven converters
    if (active_converter->next_event) {
        uint8_t row, col;
        bool pressed;
        bool changed = false;

        while (active_converter->next_event(active_wire, &kb_info,
                                             &row, &col, &pressed)) {
            if (row < MATRIX_ROWS && col < MATRIX_COLS) {
                matrix_row_t prev = matrix[row];
                if (pressed) {
                    matrix[row] |= ((matrix_row_t)1 << col);
                } else {
                    matrix[row] &= ~((matrix_row_t)1 << col);
                }
                if (matrix[row] != prev) changed = true;
            }
        }
        return changed;
    }

    // Polled matrix converters
    if (active_converter->scan_matrix) {
        return active_converter->scan_matrix(active_wire, &kb_info, matrix);
    }

    return 0;
}

void cfw_set_leds(uint8_t led_mask) {
    if (!initialized || !active_converter || !active_converter->set_leds) return;
    active_converter->set_leds(active_wire, &kb_info, led_mask);
}

const cfw_keyboard_info_t *cfw_get_keyboard_info(void) {
    return &kb_info;
}
