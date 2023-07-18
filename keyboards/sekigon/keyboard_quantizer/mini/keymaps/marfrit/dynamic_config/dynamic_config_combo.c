// copyright 2023 sekigon-gonnoc
// spdx-license-identifier: gpl-2.0-or-later

#include "dynamic_config.h"
#include "dynamic_config_def.h"
#include "process_combo.h"

combo_t                key_combos[COMBO_LEN_MAX];
uint16_t               COMBO_LEN = 0;
static const dcombo_t *dcombos[COMBO_LEN_MAX];

#if !defined(COMBO_MUST_HOLD_PER_COMBO)
#    error "Option not satisfied"
#endif
bool get_combo_must_hold(uint16_t index, combo_t *combo) {
    return dcombos[index]->only == COMBO_OPTION_ONLY_HOLD;
}

#if !defined(COMBO_MUST_TAP_PER_COMBO)
#    error "Option not satisfied"
#endif
bool get_combo_must_tap(uint16_t index, combo_t *combo) {
    return dcombos[index]->only == COMBO_OPTION_ONLY_TAP;
}

#if !defined(COMBO_TERM_PER_COMBO)
#    error "Option not satisfied"
#endif
uint16_t get_combo_term(uint16_t index, combo_t *combo) {
    return dcombos[index]->term;
}

#if !defined(COMBO_MUST_PRESS_IN_ORDER_PER_COMBO)
#    error "Option not satisfied"
#endif
bool get_combo_must_press_in_order(uint16_t index, combo_t *combo) {
    return dcombos[index]->press_in_order;
}

#if !defined(COMBO_SHOULD_TRIGGER)
#    error "Option not satisfied"
#endif
bool combo_should_trigger(uint16_t index, combo_t *combo, uint16_t keycode, keyrecord_t *record) {
    uint16_t layer = get_highest_layer(layer_state);
    return ((1 << layer) & dcombos[index]->layer) != 0;
}

#if defined(COMBO_ONLY_FROM_LAYER)
#    error "Option not satisfied"
#endif
uint8_t combo_ref_from_layer(uint8_t layer) {
    return 0xff;
}

void activate_combos(void) {
    bool restore_combo_enable = is_combo_enabled();
    combo_disable();

    for (int cmb = 0; cmb < COMBO_LEN; cmb++) {
        memset(&key_combos[cmb], 0, sizeof(key_combos[cmb]));
    }
    COMBO_LEN = 0;

    // Add combo from the top of the configuration settings.
    // Settings written in the bottom takes priority
    for (int app = 0; app < *p_active_app_cnt; app++) {
        if (p_active_apps[app] >= p_config->app_len) break;

        const application_t *p_app     = &p_config->p_app[p_active_apps[app]];
        int                  combo_len = p_app->combo_len;
        for (int cmb = 0; cmb < combo_len; cmb++) {
            key_combos[COMBO_LEN].keys    = p_app->p_combo[cmb].keys;
            key_combos[COMBO_LEN].keycode = p_app->p_combo[cmb].keycode;
            dcombos[COMBO_LEN]            = &p_app->p_combo[cmb];
            COMBO_LEN++;
            if (COMBO_LEN == COMBO_LEN_MAX) {
                combo_enable();
                printf("Too many combo definitions >%d\n", COMBO_LEN_MAX);
                return;
            }
        }
    }

    dprintf("%d combos are registered\n", COMBO_LEN);
    if (restore_combo_enable) {
        combo_enable();
    };
}