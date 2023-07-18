// copyright 2023 sekigon-gonnoc
// spdx-license-identifier: gpl-2.0-or-later

#include QMK_KEYBOARD_H
#include <stdint.h>
#include <stdio.h>
#include "quantum.h"
#include "virtser.h"
#include "os_detection.h"

#include "dynamic_config_def.h"
#include "dynamic_config.h"
#include "dynamic_config_tap_dance.h"
#include "dynamic_config_leader.h"
#include "dynamic_config_override.h"
#include "dynamic_config_combo.h"
#include "dynamic_config_macro.h"

extern uint16_t calc_usb_crc16(const uint8_t *data, uint16_t len); // #include "usb_crc.h"

static uint8_t       active_apps[ACTIVE_APP_CNT_MAX];
static int16_t       active_app_cnt;
static LANGUAGE_TYPE keyboard_language;
static LANGUAGE_TYPE os_language;

const config_t  default_config = {.magic          = 0x999b999b,
                                  .version        = DYNAMIC_CONFIG_DEF_VERSION,
                                  .default_values = {
                                      .tapping_term            = TAPPING_TERM,
                                      .quick_tap_term          = TAPPING_TERM,
                                      .permissive_hold         = false,
                                      .hold_on_other_key_press = false,
                                      .retro_tapping           = false,
                                 }};
const config_t *p_config       = &default_config;
static const config_t *config_rom     = (config_t *)CONFIG_ADDR;

const int16_t *p_active_app_cnt = &active_app_cnt;
const uint8_t *p_active_apps    = active_apps;

void print_config(void) {
    if (p_config->magic != 0x999b999b) {
        printf("invalid config %08lx\n", p_config->magic);
    }

    for (int app = 0; app < p_config->app_len; app++) {
        printf("title:%s\n", p_config->p_app[app].p_title ? p_config->p_app[app].p_title : "");
        printf("process:%s\n", p_config->p_app[app].p_process ? p_config->p_app[app].p_process : "");
        printf("url:%s\n", p_config->p_app[app].p_url ? p_config->p_app[app].p_url : "");
        printf("os_variant:%d\n", p_config->p_app[app].os_variant);
        for (int layer = 0; layer < p_config->p_app[app].keymap_len; layer++) {
            printf("\tlayer: %d\n", p_config->p_app[app].p_keymap[layer].layer);
            for (int key = 0; key < p_config->p_app[app].p_keymap[layer].keys_len; key++) {
                printf("\t\t0x%02x to 0x%04x\n", p_config->p_app[app].p_keymap[layer].p_map[key].from, p_config->p_app[app].p_keymap[layer].p_map[key].to);
            }
        }

        printf("\tcombo:\n");
        for (int combo = 0; combo < p_config->p_app[app].combo_len; combo++) {
            printf("\t\tkey:");
            int key = 0;
            while (p_config->p_app[app].p_combo[combo].keys[key] != 0) {
                printf("0x%04x ", p_config->p_app[app].p_combo[combo].keys[key]);
                key++;
            }
            printf("\n");
            printf("\t\t\tkeycode:0x%04x\n", p_config->p_app[app].p_combo[combo].keycode);
            printf("\t\t\tterm:%d\n", p_config->p_app[app].p_combo[combo].term);
            printf("\t\t\tonly:%d\n", p_config->p_app[app].p_combo[combo].only);
        }

        printf("\tleader:\n");
        for (int leader = 0; leader < p_config->p_app[app].leader_len; leader++) {
            {
                printf("\t\tkey:");
                int key_len = p_config->p_app[app].p_leader[leader].key_len;
                for (int key = 0; key < key_len; key++) {
                    printf("0x%04x ", p_config->p_app[app].p_leader[leader].keys[key]);
                }
                printf("\n");
                printf("\t\t\tkeycode:0x%04x\n", p_config->p_app[app].p_leader[leader].keycode);
            }
        }

        printf("\toverride:\n");
        for (int override = 0; override < p_config->p_app[app].override_len; override++) {
            printf("\t\ttrigger mods: %02x\n", p_config->p_app[app].p_override[override].trigger_mods);
            printf("\t\ttrigger key: %04x\n", p_config->p_app[app].p_override[override].trigger);
            printf("\t\treplacement key: %04x\n", p_config->p_app[app].p_override[override].replacement);
        }
    }
    printf("default:\n");
    printf("\ttapping term: %d\n", p_config->default_values.tapping_term);
    printf("\tquick tap term: %d\n", p_config->default_values.quick_tap_term);
    printf("\tpermissive hold: %d\n", p_config->default_values.permissive_hold);
    printf("\thold on other key press: %d\n", p_config->default_values.hold_on_other_key_press);
    printf("\tretro tapping: %d\n", p_config->default_values.retro_tapping);
}

void print_app(void) {
    printf("{\"app\":[");
    for (int app = 0; app < p_config->app_len; app++) {
        printf("{\"title\":\"%s\",\"process\":\"%s\",\"url\":\"%s\"}", p_config->p_app[app].p_title ? p_config->p_app[app].p_title : "", p_config->p_app[app].p_process ? p_config->p_app[app].p_process : "", p_config->p_app[app].p_url ? p_config->p_app[app].p_url : "");
        if (app != p_config->app_len - 1) {
            printf(",");
        }
    }
    printf("]}\n");
}

void set_active_apps(uint8_t *app_indexes, uint8_t len) {
    if (len > ACTIVE_APP_CNT_MAX) {
        len = ACTIVE_APP_CNT_MAX;
    }

    active_app_cnt = 0;
    for (int idx = 0; idx < len; idx++) {
        const application_t *p_app = &p_config->p_app[app_indexes[idx]];
        if (p_app->os_variant != 0 && p_app->os_variant != detected_host_os()) continue;
        active_apps[active_app_cnt++] = app_indexes[idx];
        dprintf("activate app:%02x\n", active_apps[idx]);

        if (active_app_cnt == ACTIVE_APP_CNT_MAX) {
            break;
        }
    }

    activate_combos();
    activate_override();
}

static bool is_config_rom_valid_crc(void)
{
    return config_rom->magic == default_config.magic &&      //
           config_rom->body_length <= CONFIG_MAX_LEN - 12 && //
           config_rom->crc16 == calc_usb_crc16((const uint8_t *)&config_rom->yaml_len, config_rom->body_length);
}

void dynamic_config_init(void) {
    if (config_rom->version == default_config.version && is_config_rom_valid_crc()) {
        p_config = config_rom;
    }

    dynamic_config_activate_default_apps();

    activate_tap_dances();
    set_keyboard_language(p_config->default_values.keyboard_language);
    set_os_language(p_config->default_values.os_language);
}

void dynamic_config_activate_default_apps(void) {
    // call after os detection completed
    active_app_cnt = 0;
    for (int app = 0; app < p_config->app_len; app++) {
        if (p_config->p_app[app].p_title == NULL && p_config->p_app[app].p_process == NULL && p_config->p_app[app].p_url == NULL) {
            const application_t *p_app = &p_config->p_app[app];
            if (p_app->os_variant != 0 && p_app->os_variant != detected_host_os()) continue;
            active_apps[active_app_cnt++] = app;
            dprintf("activate app:%02x\n", active_apps[active_app_cnt - 1]);
            if (active_app_cnt >= ACTIVE_APP_CNT_MAX) {
                break;
            }
        }
    }

    activate_combos();
    activate_override();
}

void dynamic_config_task(void) {
    process_macro_send_char();
    dynamic_tap_dance_task();
    process_dynamic_config_leader_task();
}

void set_keyboard_language(LANGUAGE_TYPE lang) {
    keyboard_language = lang;
    register_key_on_os_lang_overrides(keyboard_language, os_language);
}

void set_os_language(LANGUAGE_TYPE lang) {
    os_language = lang;
    register_key_on_os_lang_overrides(keyboard_language, os_language);
    set_ascii_to_keycode_lang_lut(lang);
}

LANGUAGE_TYPE get_keyboard_language(void) {
    return keyboard_language;
}

LANGUAGE_TYPE get_os_language(void) {
    return os_language;
}

uint16_t get_mouse_gesture_threshold(void) {
    return p_config->default_values.mouse_gesture_threshold;
}

void send_bootstrap_macro(void) {
    send_string(SS_DOWN(X_LWIN) SS_TAP(X_X) SS_UP(X_LWIN) "i" SS_DELAY(1000));

    set_macro_string("$p=gwmi Win32_SerialPort|"
                     "?{($d=Get-PnpDevice -FriendlyName \"USB Composite Device\"|"
                     "?{$_.InstanceId -match \"VID_FEED&PID_999B.*" SERIAL_NUMBER "\" -and "
                     "$_.Status -match \"OK\"}|"
                     "Get-PnpDeviceProperty DEVPKEY_Device_Children|"
                     "select -ExpandProperty Data)-contains $_.PNPDeviceId}|"
                     "select -First 1|"
                     "%{$p=New-Object "
                     "System.IO.Ports.SerialPort($_.DeviceID);$p.DtrEnable=1;$p."
                     "RtsEnable=1;$p.ReadTimeout=1000;$p};"
                     "$p.Open();"
                     "$p.WriteLine(\"companion\");$p.readLine();"
                     "$r=\"\";"
                     "try{while($p.IsOpen){$r+=$p.ReadLine()+\"`r`n\"}}catch{$p.close()}"
                     "$f=(New-TemporaryFile).FullName+\".ps1\";"
                     "$r|Out-File $f -Encoding ascii;"
                     "start powershell -WindowStyle Hidden -ArgumentList "
                     "\"-ExecutionPolicy Bypass $f $($p.portName)\";");
}

__asm__(".section .rodata\n"
        ".balign 4\n"
        ".global companion_app\n"
        "companion_app:\n"
        ".incbin \"quantizer_companion.ps1\"\n"
        ".balign 1\n"
        "_end_companion_app:\n"
        ".byte 0\n" // null termination
        ".balign 4\n"
        "sizeof_companion_app:"
        ".int _end_companion_app - companion_app\n"
        ".section .text\n");

extern const size_t companion_app;
extern const size_t sizeof_companion_app;
const uint8_t      *p_companion_app = (const uint8_t *)&companion_app;

void print_companion_app(void) {
    printf("%s\n", p_companion_app);
}

void send_config_file(void) {
    const config_t *p_send_config = p_config;
    if (p_send_config != config_rom && is_config_rom_valid_crc()) {
        p_send_config = config_rom;
    }

    for (int idx = 0; idx < p_send_config->p_yaml + p_send_config->yaml_len - (uint8_t *)p_send_config; idx++) {
        virtser_send(((const uint8_t *)p_send_config)[idx]);
    }
}

bool process_record_dynamic_config(uint16_t keycode, keyrecord_t *record) {
    dynamic_tap_dance_task();

    bool res = true;
    res &= process_record_dynamic_leader(keycode, record);
    res &= process_record_dynamic_config_macro(keycode, record);

    return true && res;
}
