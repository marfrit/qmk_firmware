// copyright 2023 sekigon-gonnoc
// spdx-license-identifier: gpl-2.0-or-later

#include "ascii_to_keycode_lut.h"
#include "dynamic_config_macro.h"
#include "dynamic_config.h"
#include "dynamic_config_def.h"

#define MACRO_STACK_MAX 4

typedef struct {
    const char *p_macro_string;
    uint16_t    active_macro_step;
} macro_data_t;
typedef struct {
    macro_data_t data[MACRO_STACK_MAX];
    int16_t      pos;
} macro_stack_t;

static macro_data_t  empty_macro;
static macro_stack_t macro_stack = {.pos = -1};
static bool          macro_abort = false;

static void push_macro(const char *str) {
    if (macro_stack.pos < MACRO_STACK_MAX - 1) {
        macro_stack.data[macro_stack.pos + 1].p_macro_string    = str;
        macro_stack.data[macro_stack.pos + 1].active_macro_step = 0;
        macro_stack.pos++;
    }
}

static void pop_macro(void) {
    if (macro_stack.pos >= 0) {
        macro_stack.data[macro_stack.pos].p_macro_string    = NULL;
        macro_stack.data[macro_stack.pos].active_macro_step = 0;
        macro_stack.pos--;
    }
}

static macro_data_t *peek_macro_stack(void) {
    if (macro_stack.pos >= 0) {
        return &macro_stack.data[macro_stack.pos];
    } else {
        return &empty_macro;
    }
}

void set_macro_string(const char *str) {
    push_macro(str);
}

bool process_record_dynamic_config_macro(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        if (keycode >= QK_UNICODE && keycode <= QK_UNICODEMAP_MAX) {
            uint8_t id         = keycode - QK_UNICODE;
            uint8_t macro_type = p_config->p_macro[id].macro->type;
            if (macro_type == MACRO_TYPE_MACRO) {
                push_macro(p_config->p_macro[id].macro->str);
                macro_abort = false;
            } else if (macro_type == MACRO_TYPE_COMMAND) {
                printf("command: %s\n", p_config->p_macro[id].macro->str);
            } else if (macro_type == MACRO_TYPE_UNICODE_STRING) {
                printf("send_string_u: %s\n", p_config->p_macro[id].macro->str);
            } else {
                printf("unknown macro\n");
            }
            return false;
        } else if (macro_stack.pos >= 0 && keycode == KC_ESC) {
            macro_abort = true;
            return false;
        }
    }

    return true;
}

void process_macro_send_char(void) {
    if (macro_stack.pos < 0) return;

    if (macro_abort) {
        pop_macro();
        clear_keyboard();

        if (macro_stack.pos < 0) {
            macro_abort = false;
        }

        return;
    }

    macro_data_t *macro_data = peek_macro_stack();
    char          data[4]    = {0, 0, 0, 0};

    data[0] = macro_data->p_macro_string[macro_data->active_macro_step++];
    data[1] = 0;
    if (data[0] == 0) {
        // macro complete
        pop_macro();
        return;
    }

    if (data[0] == SS_TAP_CODE || data[0] == SS_DOWN_CODE || data[0] == SS_UP_CODE) {
        data[2]          = macro_data->p_macro_string[macro_data->active_macro_step++];
        data[3]          = macro_data->p_macro_string[macro_data->active_macro_step++];
        uint16_t keycode = data[2] | ((uint16_t)data[3]) << 8;
        if (keycode == KC_NO) {
            pop_macro();
        } else if (keycode == KC_ESC) {
            tap_code(KC_ESC);
        } else {
            if (data[0] == SS_TAP_CODE) {
                dynamic_config_tap_code(keycode);
            } else if (data[0] == SS_DOWN_CODE) {
                dynamic_config_register_code(keycode);
            } else if (data[0] == SS_UP_CODE) {
                dynamic_config_unregister_code(keycode);
            }
        }
    } else if (data[0] == SS_DELAY_CODE) {
        data[1] = macro_data->p_macro_string[macro_data->active_macro_step++];
        data[2] = macro_data->p_macro_string[macro_data->active_macro_step++];

        if (data[1] == 0 && data[2] == 0) {
            pop_macro();
        } else {
            wait_ms(((((uint32_t)data[2]) << 8) | data[1]));
        }
    } else {
        send_string(data);
    }
}