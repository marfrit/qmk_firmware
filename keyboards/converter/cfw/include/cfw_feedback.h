// SPDX-License-Identifier: GPL-2.0-or-later
// CFW — Feedback driver interface
// Standard LED indicators + optional extended display (OLED, etc.)
#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    // --- Standard indicators (every keyboard) ---
    // Handled by converter's set_leds(), not here

    // --- Extended display (optional, converter-specific) ---
    bool has_display;
    uint8_t display_width;    // characters (text mode) or pixels (raw mode)
    uint8_t display_height;   // lines (text mode) or pixels (raw mode)

    // Text display
    void (*display_text)(uint8_t line, const char *text);
    void (*display_clear)(void);

    // Raw framebuffer (for SSD1306 etc. — Corne/Lily58 OLEDs)
    void (*display_raw)(const uint8_t *framebuffer, uint16_t size);

    // Status line (single-line updates, e.g. layer name, WPM, LLM status)
    void (*display_status)(const char *text);

} cfw_feedback_t;
