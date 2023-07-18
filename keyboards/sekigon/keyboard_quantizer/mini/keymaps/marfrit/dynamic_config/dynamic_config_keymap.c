// copyright 2023 sekigon-gonnoc
// spdx-license-identifier: gpl-2.0-or-later

#include "dynamic_config.h"
#include "dynamic_config_def.h"
#include "quantum.h"

// use keycodes 0-3 remap for action injection
#define KC_NO_REMAP_SIZE 4
static uint16_t kc_no_remap[KC_NO_REMAP_SIZE] = {KC_NO};

static uint8_t set_kc_no_remap(uint16_t kc) {
    for (int col = 0; col < KC_NO_REMAP_SIZE; col++) {
        if (kc_no_remap[col] == KC_NO) {
            kc_no_remap[col] = kc;
            return col;
        }
    }
    return 0xff;
}

void dynamic_config_tap_code(uint16_t kc) {
    uint8_t col = set_kc_no_remap(kc);
    if (col < KC_NO_REMAP_SIZE) {
        action_exec((keyevent_t){.key = {.row = 0, .col = col}, .type = KEY_EVENT, .pressed = true, .time = (timer_read() | 1)});
        action_exec((keyevent_t){.key = {.row = 0, .col = col}, .type = KEY_EVENT, .pressed = false, .time = (timer_read() | 1)});
    }
}

uint8_t dynamic_config_register_code(uint16_t kc) {
    uint8_t col = set_kc_no_remap(kc);
    if (col < KC_NO_REMAP_SIZE) {
        action_exec((keyevent_t){.key = {.row = 0, .col = col}, .type = KEY_EVENT, .pressed = true, .time = (timer_read() | 1)});
    }
    return col;
}

void dynamic_config_unregister_code_col(uint8_t col) {
    if (col < KC_NO_REMAP_SIZE) {
        action_exec((keyevent_t){.key = {.row = 0, .col = col}, .type = KEY_EVENT, .pressed = false, .time = (timer_read() | 1)});
    }
}

void dynamic_config_unregister_code(uint16_t kc) {
    for (int col = 0; col < KC_NO_REMAP_SIZE; col++) {
        if (kc_no_remap[col] == kc) {
            dynamic_config_unregister_code_col(col);
            return;
        }
    }
}

uint16_t dynamic_config_keymap_keycode_to_keycode(uint8_t layer, uint16_t keycode) {
    if (layer >= MAX_LAYER) return keycode;

    // Search for the keymap starting from the bottom of the configuration
    // settings. Settings written in the bottom takes priority
    for (int app = (int16_t)*p_active_app_cnt - 1; app >= 0; app--) {
        if (p_active_apps[app] >= p_config->app_len) break;

        const application_t *p_app      = &p_config->p_app[p_active_apps[app]];
        int                  keymap_len = p_app->keymap_len;
        for (int km = 0; km < keymap_len; km++) {
            if (p_app->p_keymap[km].layer != layer) {
                continue;
            }

            const dkeymap_t *p_keymap = &p_app->p_keymap[km];
            int              keylen   = p_keymap->keys_len;
            for (int key = 0; key < keylen; key++) {
                if (p_keymap->p_map[key].from == keycode) {
                    return p_keymap->p_map[key].to;
                }
            }
        }
    }

    return layer == 0 ? keycode : KC_TRNS;
}

int8_t dynamic_config_get_mouse_scale(uint8_t layer, DYNAMIC_CONFIG_MOUSE_SCALE scale) {
    if (layer >= MAX_LAYER) return MOUSE_SCALE_BASE;

    // Search for the keymap starting from the bottom of the configuration
    // settings. Settings written in the bottom takes priority
    for (int app = (int16_t)*p_active_app_cnt - 1; app >= 0; app--) {
        if (p_active_apps[app] >= p_config->app_len) break;

        const application_t *p_app      = &p_config->p_app[p_active_apps[app]];
        int                  keymap_len = p_app->keymap_len;
        for (int km = 0; km < keymap_len; km++) {
            if (p_app->p_keymap[km].layer != layer) {
                continue;
            }

            const dkeymap_t *p_keymap = &p_app->p_keymap[km];
            if (p_keymap->mouse.scales[scale] != MOUSE_SCALE_TRNS) {
                return p_keymap->mouse.scales[scale];
            }
        }
    }

    return layer == 0 ? MOUSE_SCALE_BASE : MOUSE_SCALE_TRNS;
}

// override keymap_key_to_keycode
uint16_t keymap_key_to_keycode(uint8_t layer, keypos_t key) {
    if (key.row == 0 && key.col < KC_NO_REMAP_SIZE) return kc_no_remap[key.col];

    return dynamic_config_keymap_keycode_to_keycode(layer, key.row * MATRIX_COLS + key.col);
}

void post_process_record_kb(uint16_t keycode, keyrecord_t* record) {
    post_process_record_user(keycode, record);

    if (!record->event.pressed && record->event.type == KEY_EVENT && record->event.key.row == 0 && record->event.key.col < KC_NO_REMAP_SIZE) {
        kc_no_remap[record->event.key.col] = KC_NO;
    }
}

#if !defined(TAPPING_TERM_PER_KEY)
#    error "Option not satisfied"
#endif
uint16_t get_tapping_term(uint16_t keycode, keyrecord_t *record) {
    for (int idx = 0; idx < p_config->per_key_option_len; idx++) {
        if (p_config->p_per_key_option[idx].key == keycode) {
            return p_config->p_per_key_option[idx].tapping_term;
        }
    }

    return p_config->default_values.tapping_term;
}


#if !defined(QUICK_TAP_TERM_PER_KEY)
#    error "Option not satisfied"
#endif
uint16_t get_quick_tap_term(uint16_t keycode, keyrecord_t *record) {
    for (int idx = 0; idx < p_config->per_key_option_len; idx++) {
        if (p_config->p_per_key_option[idx].key == keycode) {
            return p_config->p_per_key_option[idx].quick_tap_term;
        }
    }

    return p_config->default_values.quick_tap_term;
}


#if !defined(PERMISSIVE_HOLD_PER_KEY)
#    error "Option not satisfied"
#endif
bool get_permissive_hold(uint16_t keycode, keyrecord_t *record) {
    for (int idx = 0; idx < p_config->per_key_option_len; idx++) {
        if (p_config->p_per_key_option[idx].key == keycode) {
            return p_config->p_per_key_option[idx].permissive_hold;
        }
    }

    return p_config->default_values.permissive_hold;
}


#if !defined(HOLD_ON_OTHER_KEY_PRESS_PER_KEY)
#    error "Option not satisfied"
#endif
bool get_hold_on_other_key_press(uint16_t keycode, keyrecord_t *record) {
    for (int idx = 0; idx < p_config->per_key_option_len; idx++) {
        if (p_config->p_per_key_option[idx].key == keycode) {
            return p_config->p_per_key_option[idx].hold_on_other_key_press;
        }
    }

    return p_config->default_values.hold_on_other_key_press;
}

#if !defined(RETRO_TAPPING_PER_KEY)
#    error "Option not satisfied"
#endif
bool get_retro_tapping(uint16_t keycode, keyrecord_t *record) {
    for (int idx = 0; idx < p_config->per_key_option_len; idx++) {
        if (p_config->p_per_key_option[idx].key == keycode) {
            return p_config->p_per_key_option[idx].retro_tapping;
        }
    }

    return p_config->default_values.retro_tapping;
}

#if !defined(LAYER_STATE_16BIT)
#    error "Option not satisfied"
#endif