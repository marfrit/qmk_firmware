// copyright 2023 sekigon-gonnoc
// spdx-license-identifier: gpl-2.0-or-later

#include "dynamic_config.h"
#include "dynamic_config_def.h"
#include "dynamic_config_override.h"

#include "quantum.h"
#include "process_key_override.h"
#include "keymap_japanese.h"

// 32 for JP/US override
#ifndef DYNAMIC_KEY_OVERRIDE_COUNT_MAX
#    define DYNAMIC_KEY_OVERRIDE_COUNT_MAX (64 + 32)
#endif

static const key_override_t *dynamic_key_override[DYNAMIC_KEY_OVERRIDE_COUNT_MAX + 1] = {NULL};
static int16_t               key_override_cnt                                         = 0;

typedef enum {
    NO_KEY_OS_OVERRIDE,
    JP_KEY_ON_US_OS_OVERRIDE,
    US_KEY_ON_JP_OS_OVERRIDE,
} JP_US_OVERRIDE;
static JP_US_OVERRIDE jp_us_override = NO_KEY_OS_OVERRIDE;

const key_override_t **key_overrides = dynamic_key_override;

static void append_us_key_on_jp_os_overrides(void);
static void append_jp_key_on_us_os_overrides(void);

int register_override(const key_override_t *override) {
    if (key_override_cnt >= DYNAMIC_KEY_OVERRIDE_COUNT_MAX) {
        return -1;
    }

    dynamic_key_override[key_override_cnt++] = override;
    dynamic_key_override[key_override_cnt]   = NULL;
    return 0;
}

void remove_all_overrides(void) {
    dynamic_key_override[0] = NULL;
    key_override_cnt        = 0;
}

void activate_override(void) {
    bool restore_key_override_enable = key_override_is_enabled();
    key_override_off();
    remove_all_overrides();

    if (jp_us_override == JP_KEY_ON_US_OS_OVERRIDE) {
        append_jp_key_on_us_os_overrides();
    }
    if (jp_us_override == US_KEY_ON_JP_OS_OVERRIDE) {
        append_us_key_on_jp_os_overrides();
    }

    int16_t app_cnt = *p_active_app_cnt;

    for (int app = app_cnt - 1; app >= 0; app--) {
        if (p_active_apps[app] >= p_config->app_len) break;

        const application_t *p_app        = &p_config->p_app[p_active_apps[app]];
        int                  override_len = p_app->override_len;
        for (int ovr = 0; ovr < override_len; ovr++) {
            register_override(&p_app->p_override[ovr]);
        }
    }

    if (restore_key_override_enable) {
        key_override_on();
    }
}

// helper macro
#define w_shift(kc, kc_override) &ko_make_basic(MOD_MASK_SHIFT, kc, kc_override)
#define wo_shift(kc, kc_override) &ko_make_with_layers_and_negmods(0, kc, kc_override, ~0, (uint8_t)MOD_MASK_SHIFT)

// If ALT+GRV is pressed, send JP_ZKHK on windows
// To do this, mask ALT in override config
static const key_override_t *grv_override_win_us_on_jp = {&ko_make_with_layers_and_negmods(0, KC_GRV, JP_GRV, ~0, (uint8_t)MOD_MASK_SHIFT | MOD_MASK_ALT)};

// Always override KC_GRV to JP_GRV on Mac
static const key_override_t *grv_override_mac_us_on_jp = {wo_shift(KC_GRV, JP_GRV)};

static const key_override_t *grv_override_win_jp_on_us = {wo_shift(KC_GRV, LALT(KC_GRV))};
static const key_override_t *grv_override_mac_jp_on_us = {wo_shift(KC_GRV, LCTL(KC_SPC))};

// Perform as a JP keyboard on US systems
static const key_override_t *jp_key_on_us_os_overrides[] = {
    w_shift(KC_2, KC_DQT), w_shift(KC_6, KC_AMPR), w_shift(KC_7, KC_QUOT), w_shift(KC_8, KC_LPRN), w_shift(KC_9, KC_RPRN), w_shift(KC_0, KC_NO), w_shift(KC_MINS, KC_EQL), wo_shift(KC_EQL, KC_CIRC), w_shift(KC_EQL, KC_TILD), wo_shift(KC_INT3, KC_BSLS), w_shift(KC_INT3, KC_PIPE), wo_shift(KC_LBRC, KC_AT), w_shift(KC_LBRC, KC_GRV), wo_shift(KC_RBRC, KC_LBRC), w_shift(KC_RBRC, KC_LCBR), w_shift(KC_SCLN, KC_PLUS), wo_shift(KC_QUOT, KC_COLN), w_shift(KC_QUOT, KC_ASTR), wo_shift(KC_NUHS, KC_RBRC), w_shift(KC_NUHS, KC_RCBR), wo_shift(KC_INT1, KC_BSLS), w_shift(KC_INT1, KC_UNDS),
};

// Perform as a US keyboard on JIS systems
static const key_override_t *us_key_on_jp_os_overrides[] = {
    // KC_GRV override depends on OS
    w_shift(KC_GRV, JP_TILD), w_shift(KC_2, JP_AT), w_shift(KC_6, JP_CIRC), w_shift(KC_7, JP_AMPR), w_shift(KC_8, JP_ASTR), w_shift(KC_9, JP_LPRN), w_shift(KC_0, JP_RPRN), w_shift(KC_MINS, JP_UNDS), wo_shift(KC_EQL, JP_EQL), w_shift(KC_EQL, JP_PLUS), wo_shift(KC_LBRC, JP_LBRC), w_shift(KC_LBRC, JP_LCBR), wo_shift(KC_RBRC, JP_RBRC), w_shift(KC_RBRC, JP_RCBR), wo_shift(KC_BSLS, JP_BSLS), w_shift(KC_BSLS, JP_PIPE), w_shift(KC_SCLN, JP_COLN), wo_shift(KC_QUOT, JP_QUOT), w_shift(KC_QUOT, JP_DQUO),
};

void register_key_on_os_lang_overrides(LANGUAGE_TYPE keyboard_lang, LANGUAGE_TYPE os_lang) {
    JP_US_OVERRIDE override = NO_KEY_OS_OVERRIDE;
    if (keyboard_lang == LANG_JP && os_lang == LANG_US) {
        override = JP_KEY_ON_US_OS_OVERRIDE;
    } else if (keyboard_lang == LANG_US && os_lang == LANG_JP) {
        override = US_KEY_ON_JP_OS_OVERRIDE;
    }
    jp_us_override = override;
    activate_override();
}

static void append_us_key_on_jp_os_overrides(void) {
    for (int idx = 0; idx < sizeof(us_key_on_jp_os_overrides) / sizeof(us_key_on_jp_os_overrides[0]); idx++) {
        register_override(us_key_on_jp_os_overrides[idx]);
    }

    if (keymap_config.swap_lalt_lgui) {
        register_override(grv_override_mac_us_on_jp);
    } else {
        register_override(grv_override_win_us_on_jp);
    }
}

static void append_jp_key_on_us_os_overrides(void) {
    for (int idx = 0; idx < sizeof(jp_key_on_us_os_overrides) / sizeof(jp_key_on_us_os_overrides[0]); idx++) {
        register_override(jp_key_on_us_os_overrides[idx]);
    }

    if (keymap_config.swap_lalt_lgui) {
        register_override(grv_override_mac_jp_on_us);
    } else {
        register_override(grv_override_win_jp_on_us);
    }
}