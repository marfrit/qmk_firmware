// Copyright 2023 sekigon-gonnoc
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

#include "report_parser.h"
#include "dynamic_config.h"

typedef enum {
    GESTURE_NONE = 0,
    GESTURE_DOWN_RIGHT,
    GESTURE_DOWN_LEFT,
    GESTURE_UP_LEFT,
    GESTURE_UP_RIGHT,
} gesture_id_t;

extern bool          matrix_has_changed;
extern matrix_row_t* matrix_dest;
extern bool          is_encoder_action;
extern bool          mouse_send_flag;

uint8_t  encoder_modifier            = 0;
uint16_t encoder_modifier_pressed_ms = 0;
bool     is_encoder_action           = false;
int      reset_flag                  = 0;

#ifndef ENCODER_MODIFIER_TIMEOUT_MS
#    define ENCODER_MODIFIER_TIMEOUT_MS 500
#endif

static int16_t  gesture_move_x          = 0;
static int16_t  gesture_move_y          = 0;
static bool     gesture_wait            = false;
static int16_t  wheel_move_v            = 0;
static int16_t  wheel_move_h            = 0;
static uint16_t mouse_gesture_threshold = 50;

// Start gesture recognition
static void gesture_start(void) {
    dprint("Gesture start\n");
    gesture_wait   = true;
    gesture_move_x = 0;
    gesture_move_y = 0;
}

void set_mouse_gesture_threshold(uint16_t val) {
    if (val > 0) {
        mouse_gesture_threshold = val;
    }
}

gesture_id_t recognize_gesture(int16_t x, int16_t y) {
    gesture_id_t gesture_id = 0;

    if (abs(x) + abs(y) < mouse_gesture_threshold) {
        gesture_id = GESTURE_NONE;
    } else if (x >= 0 && y >= 0) {
        gesture_id = GESTURE_DOWN_RIGHT;
    } else if (x < 0 && y >= 0) {
        gesture_id = GESTURE_DOWN_LEFT;
    } else if (x < 0 && y < 0) {
        gesture_id = GESTURE_UP_LEFT;
    } else if (x >= 0 && y < 0) {
        gesture_id = GESTURE_UP_RIGHT;
    }

    return gesture_id;
}

static uint16_t get_remapped_keycode_from_keycode(uint16_t keycode) {
    for (uint16_t layer = 15; layer > 0; layer--) {
        if (layer_state & (1 << layer)) {
            uint16_t kc = dynamic_config_keymap_keycode_to_keycode(layer, keycode);
            if (kc != KC_TRNS) {
                return kc;
            }
        }
    }

    return dynamic_config_keymap_keycode_to_keycode(0, keycode);
}

static int8_t get_mouse_scale(DYNAMIC_CONFIG_MOUSE_SCALE scale_type) {
    for (uint16_t layer = 15; layer > 0; layer--) {
        if (layer_state & (1 << layer)) {
            int8_t scale = dynamic_config_get_mouse_scale(layer, scale_type);
            if (scale != MOUSE_SCALE_TRNS) {
                return scale;
            }
        }
    }

    return dynamic_config_get_mouse_scale(0, scale_type);
}

void process_gesture(uint8_t layer, gesture_id_t gesture_id) {
    switch (gesture_id) {
        case GESTURE_DOWN_RIGHT ... GESTURE_UP_RIGHT: {
            uint16_t keycode = dynamic_config_keymap_keycode_to_keycode(layer, QK_KB_0 + gesture_id - GESTURE_DOWN_RIGHT);
            if (keycode == QK_KB_0 + gesture_id - GESTURE_DOWN_RIGHT) {
                return;
            }
            dynamic_config_tap_code(keycode);
        } break;
        default:
            break;
    }
}

typedef struct {
    int8_t x;
    int8_t y;
    int8_t v;
    int8_t h;
    int8_t xh;
    int8_t yv;
} scaled_report_t;

static void calc_mouse_scaled_move(mouse_parse_result_t const* report, scaled_report_t* scaled) {
    static int16_t x_frac;
    static int16_t y_frac;
    static int16_t v_frac;
    static int16_t h_frac;
    static int16_t xh_frac;
    static int16_t yv_frac;

    int8_t scale_x = get_mouse_scale(DYNAMIC_CONFIG_MOUSE_SCALE_X);
    int8_t scale_y = get_mouse_scale(DYNAMIC_CONFIG_MOUSE_SCALE_Y);
    int8_t scale_v = get_mouse_scale(DYNAMIC_CONFIG_MOUSE_SCALE_V);
    int8_t scale_h = get_mouse_scale(DYNAMIC_CONFIG_MOUSE_SCALE_H);

    int16_t x_real  = (report->x * scale_x) + x_frac;
    int16_t y_real  = (report->y * scale_y) + y_frac;
    int16_t v_real  = (report->v * scale_v) + v_frac;
    int16_t h_real  = (report->h * scale_h) + h_frac;
    int16_t xh_real = (report->x * scale_x) + xh_frac;
    int16_t yv_real = (report->y * scale_y) + yv_frac;
    scaled->x       = x_real >> 4;
    scaled->y       = y_real >> 4;
    scaled->v       = v_real >> 4;
    scaled->h       = h_real >> 4;
    scaled->xh      = xh_real >> 8;
    scaled->yv      = yv_real >> 8;
    x_frac          = x_real - (scaled->x << 4);
    y_frac          = y_real - (scaled->y << 4);
    v_frac          = v_real - (scaled->v << 4);
    h_frac          = h_real - (scaled->h << 4);
    xh_frac         = xh_real - (scaled->xh << 8);
    yv_frac         = yv_real - (scaled->yv << 8);
}

void mouse_report_hook(mouse_parse_result_t const* report) {
    if (debug_enable) {
        xprintf("Mouse report\n");
        xprintf("b:%d ", report->button);
        xprintf("x:%d ", report->x);
        xprintf("y:%d ", report->y);
        xprintf("v:%d ", report->v);
        xprintf("h:%d ", report->h);
        xprintf("undef:%u\n", report->undefined);
    }

    //
    // Assign buttons to matrix
    // 8 button mouse is assumed
    //
    uint8_t button_current = report->button;
    for (int bit = 0; bit < 8 * sizeof(button_current); bit++) {
        if (button_current & (1 << bit)) {
            matrix_dest[(KC_MS_BTN1 + bit) / 8] |= (1 << ((KC_MS_BTN1 + bit) & 0x07));
        } else {
            matrix_dest[(KC_MS_BTN1 + bit) / 8] &= ~(1 << ((KC_MS_BTN1 + bit) & 0x07));
        }
    }

    mouse_parse_result_t raw_report = *report;
    scaled_report_t      scaled;
    calc_mouse_scaled_move(&raw_report, &scaled);

    uint16_t ms_left_map = get_remapped_keycode_from_keycode(KC_MS_LEFT);
    uint16_t ms_up_map   = get_remapped_keycode_from_keycode(KC_MS_UP);
    if (ms_left_map == KC_MS_WH_RIGHT) {
        // remap x to h, and h will remap again
        scaled.h += scaled.xh;
        scaled.x = 0;
    }

    if (ms_up_map == KC_MS_WH_DOWN) {
        // remap y to v, and v will remap again
        scaled.v -= scaled.yv;
        scaled.y = 0;
    }

    //
    // Assign wheel to key action
    //
    if (scaled.v != 0) {
        keypos_t key;
        wheel_move_v      = scaled.v;
        uint16_t kc       = scaled.v > 0 ? KC_MS_WH_UP : KC_MS_WH_DOWN;
        key.row           = kc >> 3;
        key.col           = kc & 0x07;
        is_encoder_action = true;
        action_exec((keyevent_t){.key = key, .type = KEY_EVENT, .pressed = true, .time = (timer_read() | 1)});
        action_exec((keyevent_t){.key = key, .type = KEY_EVENT, .pressed = false, .time = (timer_read() | 1)});
        is_encoder_action = false;
    }

    if (scaled.h != 0) {
        keypos_t key;
        wheel_move_h      = scaled.h;
        uint16_t kc       = scaled.h > 0 ? KC_MS_WH_LEFT : KC_MS_WH_RIGHT;
        key.row           = kc >> 3;
        key.col           = kc & 0x07;
        is_encoder_action = true;
        action_exec((keyevent_t){.key = key, .type = KEY_EVENT, .pressed = true, .time = (timer_read() | 1)});
        action_exec((keyevent_t){.key = key, .type = KEY_EVENT, .pressed = false, .time = (timer_read() | 1)});
        is_encoder_action = false;
    }

    //
    // Assign mouse movement
    //
    report_mouse_t mouse = pointing_device_get_report();

    if (scaled.x != 0 && ms_left_map == KC_MS_LEFT) {
        mouse_send_flag = true;
        mouse.x += scaled.x;
    } else if (scaled.xh != 0 && ms_left_map == KC_MS_WH_LEFT) {
        // remap x to h, and h will send as mouse report
        mouse_send_flag = true;
        mouse.h += scaled.xh;
    }

    if (scaled.y != 0 && ms_up_map == KC_MS_UP) {
        mouse_send_flag = true;
        mouse.y += scaled.y;
    } else if (scaled.yv != 0 && ms_up_map == KC_MS_WH_UP) {
        // remap y to v, and v will send as mouse report
        mouse_send_flag = true;
        mouse.v -= scaled.yv;
    }

    pointing_device_set_report(mouse);

    //
    // Save movement to recognize gesture
    //
    if (gesture_wait) {
        gesture_move_x += scaled.x;
        gesture_move_y += scaled.y;
    }
}

bool process_record_mouse(uint16_t keycode, keyrecord_t* record) {
    if (encoder_modifier != 0 && !is_encoder_action) {
        unregister_mods(encoder_modifier);
        encoder_modifier = 0;
    }

    switch (keycode) {
        case QK_MODS ... QK_MODS_MAX:
            if (is_encoder_action) {
                if (record->event.pressed) {
                    uint8_t current_mods        = keycode >> 8;
                    encoder_modifier_pressed_ms = timer_read();
                    if (current_mods != encoder_modifier) {
                        del_mods(encoder_modifier);
                        encoder_modifier = current_mods;
                        add_mods(encoder_modifier);
                    }
                    register_code(keycode & 0xff);
                } else {
                    unregister_code(keycode & 0xff);
                }
                return false;
            } else {
                return true;
            }
            break;
    }

    switch (keycode) {
        case KC_BTN1 ... KC_BTN5: {
            mouse_send_flag = true;
            return true;
        } break;

        case KC_MS_WH_UP ... KC_MS_WH_DOWN: {
            if (wheel_move_v != 0) {
                report_mouse_t report = pointing_device_get_report();
                report.v              = keycode == KC_MS_WH_UP ? abs(wheel_move_v) : -abs(wheel_move_v);
                pointing_device_set_report(report);
                mouse_send_flag = true;
                return false;
            } else {
                return true;
            }
        } break;

        case KC_MS_WH_LEFT ... KC_MS_WH_RIGHT: {
            if (wheel_move_h != 0) {
                report_mouse_t report = pointing_device_get_report();
                report.h              = keycode == KC_MS_WH_LEFT ? abs(wheel_move_h) : -abs(wheel_move_h);
                pointing_device_set_report(report);
                mouse_send_flag = true;
                return false;
            } else {
                return true;
            }
        } break;
    }

    return true;
}

void post_process_record_mouse(uint16_t keycode, keyrecord_t* record) {
    if (keycode >= QK_MOMENTARY && keycode <= QK_MOMENTARY_MAX) {
        if (record->event.pressed && gesture_wait == false) {
            gesture_start();
        }
    }

    if ((keycode >= QK_LAYER_TAP && keycode <= QK_LAYER_TAP_MAX) || (keycode >= QK_MOMENTARY && keycode <= QK_MOMENTARY_MAX)) {
        if ((!record->event.pressed) && gesture_wait == true) {
            gesture_wait            = false;
            gesture_id_t gesture_id = recognize_gesture(gesture_move_x, gesture_move_y);

            uint8_t layer = 0;
            if ((keycode >= QK_LAYER_TAP && keycode <= QK_LAYER_TAP_MAX)) {
                layer = QK_LAYER_TAP_GET_LAYER(keycode);
            } else {
                layer = QK_MOMENTARY_GET_LAYER(keycode);
            }

            process_gesture(layer, gesture_id);
            // printf("id:%d x:%d,y:%d\n", gesture_id, gesture_move_x, gesture_move_y);
        }
    }
}

bool pre_process_record_mouse(uint16_t keycode, keyrecord_t *record) {
    // Start gesture when LT key is pressed
    if (keycode >= QK_LAYER_TAP && keycode <= QK_LAYER_TAP_MAX) {
        if (record->event.pressed && gesture_wait == false) {
            gesture_start();
        }
    }

    return true;
}