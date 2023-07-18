// copyright 2023 sekigon-gonnoc
// spdx-license-identifier: gpl-2.0-or-later

#pragma once
#include <stdint.h>
#include "process_key_override.h"

#define DYNAMIC_CONFIG_DEF_VERSION 13

typedef struct {
    uint16_t from;
    uint16_t to;
} map_t;

typedef struct {
    union {
        struct {
            int8_t scale_x;
            int8_t scale_y;
            int8_t scale_v;
            int8_t scale_h;
        };
        int8_t scales[4];
    };
} dmouse_t;

typedef struct {
    uint16_t           layer;
    uint16_t           keys_len;
    map_t const *const p_map;
    dmouse_t const     mouse;
} dkeymap_t;

enum {
    COMBO_OPTION_ONLY_NONE,
    COMBO_OPTION_ONLY_TAP,
    COMBO_OPTION_ONLY_HOLD,
} COMBO_OPTION_ONLY;

typedef struct {
    uint16_t const *keys;
    uint16_t const  keycode;
    uint16_t const  term;
    uint8_t const   only;
    bool const      press_in_order;
    uint16_t const  layer;
} dcombo_t;

typedef struct {
    uint16_t const *keys;
    uint16_t const  key_len;
    uint16_t const  keycode;
} dleader_t;

typedef key_override_t doverride_t;

typedef struct {
    char const        *p_title;
    char const        *p_process;
    char const        *p_url;
    uint8_t const      os_variant;
    uint16_t const     ime_mode;
    uint16_t const     ime_on;
    uint32_t const     keymap_len;
    dkeymap_t const   *p_keymap;
    uint32_t const     combo_len;
    dcombo_t const    *p_combo;
    uint32_t const     leader_len;
    dleader_t const   *p_leader;
    uint32_t const     override_len;
    doverride_t const *p_override;
} application_t;

typedef struct {
    uint16_t single_tap;
    uint16_t single_hold;
    uint16_t double_tap;
    uint16_t double_hold;
} dtap_dance_t;

typedef struct {
    uint16_t key;
    uint16_t tapping_term;
    uint16_t quick_tap_term;
    bool     permissive_hold;
    bool     hold_on_other_key_press;
    bool     retro_tapping;
} per_key_option_t;

enum { MACRO_TYPE_MACRO = 1, MACRO_TYPE_COMMAND, MACRO_TYPE_UNICODE_STRING };

typedef struct {
    struct {
        uint8_t type;
        char    str[];
    } const *macro;
} dmacro_t;

typedef struct {
    uint16_t       tapping_term;
    uint16_t       quick_tap_term;
    bool           permissive_hold;
    bool           hold_on_other_key_press;
    bool           retro_tapping;
    uint8_t        keyboard_language;
    uint8_t        os_language;
    uint16_t       mouse_gesture_threshold;
} default_values_t;

typedef struct {
    // header
    uint32_t const          magic;
    uint16_t const          version;
    uint16_t const          crc16; 
    uint32_t const          body_length; 
    // body
    uint32_t const          yaml_len;
    uint8_t const          *p_yaml;
    // ↑ Do not change these definitions
    default_values_t const  default_values;
    uint32_t const          app_len;
    application_t const    *p_app;
    uint32_t const          tap_dance_len;
    dtap_dance_t const     *p_tap_dance;
    uint32_t const          per_key_option_len;
    per_key_option_t const *p_per_key_option;
    uint32_t const          macro_len;
    dmacro_t const         *p_macro;
} config_t;

#define CONFIG_ADDR 0x10100000
#define CONFIG_MAX_LEN 0x000f0000

extern const int16_t  *p_active_app_cnt;
extern const uint8_t  *p_active_apps;
extern const config_t *p_config;