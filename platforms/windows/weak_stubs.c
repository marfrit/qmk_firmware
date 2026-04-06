// SPDX-License-Identifier: GPL-2.0-or-later
// QAP'LA Windows: Strong stubs for weak symbols that mingw can't resolve
//
// mingw PE format represents __attribute__((weak)) as .weak.funcname.caller
// mangled sections that don't resolve across translation units like ELF does.
// We provide strong definitions here. Use -Wl,--allow-multiple-definition
// so the linker accepts both our strong version and the weak .weak.* one.

#include <stdint.h>
#include <stdbool.h>

// Keyboard-level callbacks
void suspend_power_down_kb(void) {}
void suspend_wakeup_init_kb(void) {}
void led_init_ports(void) {}
void led_set(uint8_t usb_led) { (void)usb_led; }
uint32_t layer_state_set_kb(uint32_t state) { return state; }
int8_t sendchar(uint8_t c) { (void)c; return 0; }
void eeprom_driver_format(void) {}
uint16_t keycode_config(uint16_t keycode) { return keycode; }
uint8_t mod_config(uint8_t mod) { return mod; }

// action.c weak functions — the most critical ones for the mingw port.
// On ELF these resolve from action.o's weak definitions. On PE they don't.
// We forward to the actual implementations via action_util.h functions.
#include "action_util.h"
#include "host.h"
#include "wait.h"

#ifndef TAP_CODE_DELAY
#define TAP_CODE_DELAY 10
#endif

void register_code(uint8_t code) {
    if (code >= 0xE0 && code <= 0xE7) {
        add_mods(1 << (code & 0x07));
    } else {
        add_key(code);
    }
    send_keyboard_report();
}

void unregister_code(uint8_t code) {
    if (code >= 0xE0 && code <= 0xE7) {
        del_mods(1 << (code & 0x07));
    } else {
        del_key(code);
    }
    send_keyboard_report();
}

void register_mods(uint8_t mods) {
    add_mods(mods);
    send_keyboard_report();
}

void unregister_mods(uint8_t mods) {
    del_mods(mods);
    send_keyboard_report();
}

void register_weak_mods(uint8_t mods) {
    add_weak_mods(mods);
    send_keyboard_report();
}

void unregister_weak_mods(uint8_t mods) {
    del_weak_mods(mods);
    send_keyboard_report();
}

void tap_code(uint8_t code) {
    register_code(code);
    wait_ms(TAP_CODE_DELAY);
    unregister_code(code);
}

uint16_t tap_code_delay = TAP_CODE_DELAY;

// keymap_key_to_keycode — normally weak in keymap_common.c
#include "keymap_introspection.h"
#undef KEY_EVENT
#include "keyboard.h"
uint16_t keymap_key_to_keycode(uint8_t layer, keypos_t key) {
    if (key.row < 16 && key.col < 16) {
        return keycode_at_keymap_location(layer, key.row, key.col);
    }
    return 0;
}

// keycode_at_keymap_location — normally weak in keymap_introspection.c
uint16_t keycode_at_keymap_location(uint8_t layer_num, uint8_t row, uint8_t column) {
    return keycode_at_keymap_location_raw(layer_num, row, column);
}

// has_mouse_report_changed — from report.c
#include "report.h"
bool has_mouse_report_changed(report_mouse_t *new_report, report_mouse_t *old_report) {
    return memcmp(new_report, old_report, sizeof(report_mouse_t)) != 0;
}

// No WinMain needed — QMK's main() is strong on Windows (not weak),
// and we build with -mconsole so the CRT calls main() directly.
