// SPDX-License-Identifier: GPL-2.0-or-later
// QAP'LA: Windows Protocol Layer
//
// Keyboard interception via Low-Level Keyboard Hook (WH_KEYBOARD_LL)
// Keystroke injection via SendInput
//
// ARCHITECTURE:
//   Windows LLHOOK requires a message pump (GetMessage/DispatchMessage).
//   QMK requires a tight scan loop (keyboard_task called repeatedly).
//   We reconcile these by using PeekMessage with a short timeout in
//   protocol_post_task, which processes Windows messages without blocking
//   indefinitely.
//
// HOW LLHOOK WORKS:
//   SetWindowsHookEx(WH_KEYBOARD_LL, callback, NULL, 0) installs a
//   system-wide keyboard hook. Every keystroke passes through our
//   callback BEFORE reaching any application. The callback can:
//     - Return 0: block the keystroke (we consumed it)
//     - Call CallNextHookEx: pass it through
//   We block all real keystrokes and re-inject processed ones via
//   SendInput. Injected keystrokes have the LLKHF_INJECTED flag,
//   which we check in the callback to avoid re-intercepting our own output.
//
// LIMITATIONS:
//   - Won't intercept keystrokes in elevated (admin) windows unless
//     QAP'LA itself runs as admin
//   - Won't work on the login screen (use Interception driver for that)
//   - Some anti-cheat software blocks keyboard hooks

// QMK headers MUST come before windows.h because windows.h defines
// KEY_EVENT as 0x1 (from wincon.h), which collides with QMK's
// keyevent_type_t enum. We include QMK first, then undef, then windows.
#include "host.h"
#include "host_driver.h"
#include "report.h"
#include "timer.h"
#include "platform.h"

// Undo QMK's KEY_EVENT before windows.h redefines it
#undef KEY_EVENT
#include <windows.h>
#include <stdio.h>
#include <string.h>
// Both QMK and Windows define KEY_EVENT = 1. Lucky coincidence.

// ---- Windows VK → evdev-like scancode mapping ----
// We reuse the same matrix layout as Linux (16x16, code/16=row, code%16=col)
// but index by Windows virtual key codes instead of evdev codes.
// The evdev_to_hid table from Linux works here too since VK codes
// and evdev codes differ, but the HID codes are the same.

// Windows VK → USB HID keycode (used by matrix for keycode lookup)
__attribute__((unused)) static const uint8_t vk_to_hid[256] = {
    [0x08] = 0x2A, // VK_BACK
    [0x09] = 0x2B, // VK_TAB
    [0x0D] = 0x28, // VK_RETURN
    [0x10] = 0xE1, // VK_SHIFT (left)
    [0x11] = 0xE0, // VK_CONTROL (left)
    [0x12] = 0xE2, // VK_MENU (alt, left)
    [0x13] = 0x48, // VK_PAUSE
    [0x14] = 0x39, // VK_CAPITAL
    [0x1B] = 0x29, // VK_ESCAPE
    [0x20] = 0x2C, // VK_SPACE
    [0x21] = 0x4B, // VK_PRIOR (page up)
    [0x22] = 0x4E, // VK_NEXT (page down)
    [0x23] = 0x4D, // VK_END
    [0x24] = 0x4A, // VK_HOME
    [0x25] = 0x50, // VK_LEFT
    [0x26] = 0x52, // VK_UP
    [0x27] = 0x4F, // VK_RIGHT
    [0x28] = 0x51, // VK_DOWN
    [0x2D] = 0x49, // VK_INSERT
    [0x2E] = 0x4C, // VK_DELETE
    // 0-9
    [0x30] = 0x27, [0x31] = 0x1E, [0x32] = 0x1F, [0x33] = 0x20,
    [0x34] = 0x21, [0x35] = 0x22, [0x36] = 0x23, [0x37] = 0x24,
    [0x38] = 0x25, [0x39] = 0x26,
    // A-Z
    [0x41] = 0x04, [0x42] = 0x05, [0x43] = 0x06, [0x44] = 0x07,
    [0x45] = 0x08, [0x46] = 0x09, [0x47] = 0x0A, [0x48] = 0x0B,
    [0x49] = 0x0C, [0x4A] = 0x0D, [0x4B] = 0x0E, [0x4C] = 0x0F,
    [0x4D] = 0x10, [0x4E] = 0x11, [0x4F] = 0x12, [0x50] = 0x13,
    [0x51] = 0x14, [0x52] = 0x15, [0x53] = 0x16, [0x54] = 0x17,
    [0x55] = 0x18, [0x56] = 0x19, [0x57] = 0x1A, [0x58] = 0x1B,
    [0x59] = 0x1C, [0x5A] = 0x1D,
    // Windows keys
    [0x5B] = 0xE3, // VK_LWIN
    [0x5C] = 0xE7, // VK_RWIN
    [0x5D] = 0x65, // VK_APPS
    // Numpad
    [0x60] = 0x62, [0x61] = 0x59, [0x62] = 0x5A, [0x63] = 0x5B,
    [0x64] = 0x5C, [0x65] = 0x5D, [0x66] = 0x5E, [0x67] = 0x5F,
    [0x68] = 0x60, [0x69] = 0x61,
    [0x6A] = 0x55, // VK_MULTIPLY
    [0x6B] = 0x57, // VK_ADD
    [0x6D] = 0x56, // VK_SUBTRACT
    [0x6E] = 0x63, // VK_DECIMAL
    [0x6F] = 0x54, // VK_DIVIDE
    // F1-F12
    [0x70] = 0x3A, [0x71] = 0x3B, [0x72] = 0x3C, [0x73] = 0x3D,
    [0x74] = 0x3E, [0x75] = 0x3F, [0x76] = 0x40, [0x77] = 0x41,
    [0x78] = 0x42, [0x79] = 0x43, [0x7A] = 0x44, [0x7B] = 0x45,
    // Locks
    [0x90] = 0x53, // VK_NUMLOCK
    [0x91] = 0x47, // VK_SCROLL
    // Modifier variants (extended)
    [0xA0] = 0xE1, // VK_LSHIFT
    [0xA1] = 0xE5, // VK_RSHIFT
    [0xA2] = 0xE0, // VK_LCONTROL
    [0xA3] = 0xE4, // VK_RCONTROL
    [0xA4] = 0xE2, // VK_LMENU
    [0xA5] = 0xE6, // VK_RMENU
    // OEM keys (US layout)
    [0xBA] = 0x33, // VK_OEM_1 (;:)
    [0xBB] = 0x2E, // VK_OEM_PLUS (=+)
    [0xBC] = 0x36, // VK_OEM_COMMA
    [0xBD] = 0x2D, // VK_OEM_MINUS
    [0xBE] = 0x37, // VK_OEM_PERIOD
    [0xBF] = 0x38, // VK_OEM_2 (/?)
    [0xC0] = 0x35, // VK_OEM_3 (`~)
    [0xDB] = 0x2F, // VK_OEM_4 ([{)
    [0xDC] = 0x31, // VK_OEM_5 (\|)
    [0xDD] = 0x30, // VK_OEM_6 (]})
    [0xDE] = 0x34, // VK_OEM_7 ('")
    [0xE2] = 0x64, // VK_OEM_102 (<> on 102-key)
};

// Reverse: HID → VK for SendInput output
static const uint8_t hid_to_vk[256] = {
    [0x04] = 0x41, [0x05] = 0x42, [0x06] = 0x43, [0x07] = 0x44,
    [0x08] = 0x45, [0x09] = 0x46, [0x0A] = 0x47, [0x0B] = 0x48,
    [0x0C] = 0x49, [0x0D] = 0x4A, [0x0E] = 0x4B, [0x0F] = 0x4C,
    [0x10] = 0x4D, [0x11] = 0x4E, [0x12] = 0x4F, [0x13] = 0x50,
    [0x14] = 0x51, [0x15] = 0x52, [0x16] = 0x53, [0x17] = 0x54,
    [0x18] = 0x55, [0x19] = 0x56, [0x1A] = 0x57, [0x1B] = 0x58,
    [0x1C] = 0x59, [0x1D] = 0x5A,
    [0x1E] = 0x31, [0x1F] = 0x32, [0x20] = 0x33, [0x21] = 0x34,
    [0x22] = 0x35, [0x23] = 0x36, [0x24] = 0x37, [0x25] = 0x38,
    [0x26] = 0x39, [0x27] = 0x30,
    [0x28] = 0x0D, [0x29] = 0x1B, [0x2A] = 0x08, [0x2B] = 0x09,
    [0x2C] = 0x20,
    [0x2D] = 0xBD, [0x2E] = 0xBB, [0x2F] = 0xDB, [0x30] = 0xDD,
    [0x31] = 0xDC, [0x33] = 0xBA, [0x34] = 0xDE, [0x35] = 0xC0,
    [0x36] = 0xBC, [0x37] = 0xBE, [0x38] = 0xBF, [0x39] = 0x14,
    [0x3A] = 0x70, [0x3B] = 0x71, [0x3C] = 0x72, [0x3D] = 0x73,
    [0x3E] = 0x74, [0x3F] = 0x75, [0x40] = 0x76, [0x41] = 0x77,
    [0x42] = 0x78, [0x43] = 0x79, [0x44] = 0x7A, [0x45] = 0x7B,
    [0x46] = 0x2C, // PrintScreen → VK_SNAPSHOT
    [0x47] = 0x91, [0x48] = 0x13,
    [0x49] = 0x2D, [0x4A] = 0x24, [0x4B] = 0x21, [0x4C] = 0x2E,
    [0x4D] = 0x23, [0x4E] = 0x22, [0x4F] = 0x27, [0x50] = 0x25,
    [0x51] = 0x28, [0x52] = 0x26, [0x53] = 0x90,
    [0x54] = 0x6F, [0x55] = 0x6A, [0x56] = 0x6D, [0x57] = 0x6B,
    [0x58] = 0x0D, // KP Enter
    [0x59] = 0x61, [0x5A] = 0x62, [0x5B] = 0x63, [0x5C] = 0x64,
    [0x5D] = 0x65, [0x5E] = 0x66, [0x5F] = 0x67, [0x60] = 0x68,
    [0x61] = 0x69, [0x62] = 0x60, [0x63] = 0x6E,
    [0x64] = 0xE2, [0x65] = 0x5D,
    [0xE0] = 0xA2, [0xE1] = 0xA0, [0xE2] = 0xA4, [0xE3] = 0x5B,
    [0xE4] = 0xA3, [0xE5] = 0xA1, [0xE6] = 0xA5, [0xE7] = 0x5C,
};

// ---- Key state tracking ----
// We use Windows scan codes (vkCode) mapped to a flat array,
// same 16x16 matrix concept as Linux.
#define KEY_STATE_MAX 256
static uint8_t key_state[KEY_STATE_MAX];
static HHOOK keyboard_hook = NULL;

// ---- LLHOOK Callback ----
// Called for every keystroke system-wide. Runs in the thread that
// installed the hook (must have a message pump).

static LRESULT CALLBACK ll_keyboard_proc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode < 0) return CallNextHookEx(keyboard_hook, nCode, wParam, lParam);

    KBDLLHOOKSTRUCT *kb = (KBDLLHOOKSTRUCT *)lParam;

    // Skip our own injected keystrokes (LLKHF_INJECTED flag)
    if (kb->flags & LLKHF_INJECTED) {
        return CallNextHookEx(keyboard_hook, nCode, wParam, lParam);
    }

    // Map VK to our key state array
    uint8_t vk = (uint8_t)kb->vkCode;
    if (vk < KEY_STATE_MAX) {
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            key_state[vk] = 1;
        } else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
            key_state[vk] = 0;
        }
    }

    // Block the original keystroke — QMK will re-emit via SendInput
    return 1;
}

// ---- Matrix access for QMK ----
// The custom matrix reads from key_state[], using VK code as the index.
// Matrix position = (vk/16, vk%16), same scheme as Linux with evdev codes.

uint8_t qapla_key_state(uint16_t code) {
    if (code >= KEY_STATE_MAX) return 0;
    return key_state[code];
}

// ---- SendInput output ----
// Track previous report to diff

static uint8_t prev_keys[6];  // KEYBOARD_REPORT_KEYS default
static uint8_t prev_mods;

static void send_vk(uint8_t vk, int pressed) {
    INPUT input = {0};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = vk;
    input.ki.dwFlags = pressed ? 0 : KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));
}

static uint8_t win_keyboard_leds(void) {
    uint8_t leds = 0;
    if (GetKeyState(VK_NUMLOCK) & 1)    leds |= 1;
    if (GetKeyState(VK_CAPITAL) & 1)    leds |= 2;
    if (GetKeyState(VK_SCROLL) & 1)     leds |= 4;
    return leds;
}

static void win_send_keyboard(report_keyboard_t *report) {
    // Modifiers
    uint8_t mod_hid[] = {0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7};
    for (int i = 0; i < 8; i++) {
        uint8_t old_bit = (prev_mods >> i) & 1;
        uint8_t new_bit = (report->mods >> i) & 1;
        if (old_bit != new_bit) {
            uint8_t vk = hid_to_vk[mod_hid[i]];
            if (vk) send_vk(vk, new_bit);
        }
    }

    // Release old keys
    for (int i = 0; i < 6; i++) {
        uint8_t old_key = prev_keys[i];
        if (old_key == 0) continue;
        int found = 0;
        for (int j = 0; j < 6; j++) {
            if (report->keys[j] == old_key) { found = 1; break; }
        }
        if (!found) {
            uint8_t vk = hid_to_vk[old_key];
            if (vk) send_vk(vk, 0);
        }
    }

    // Press new keys
    for (int i = 0; i < 6; i++) {
        uint8_t new_key = report->keys[i];
        if (new_key == 0) continue;
        int found = 0;
        for (int j = 0; j < 6; j++) {
            if (prev_keys[j] == new_key) { found = 1; break; }
        }
        if (!found) {
            uint8_t vk = hid_to_vk[new_key];
            if (vk) send_vk(vk, 1);
        }
    }

    memcpy(prev_keys, report->keys, sizeof(prev_keys));
    prev_mods = report->mods;
}

static void win_send_nkro(report_nkro_t *report) { (void)report; }

static void win_send_mouse(report_mouse_t *report) {
    INPUT input = {0};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_MOVE;
    if (report->x) input.mi.dx = report->x;
    if (report->y) input.mi.dy = report->y;
    if (report->v) {
        input.mi.dwFlags |= MOUSEEVENTF_WHEEL;
        input.mi.mouseData = report->v * WHEEL_DELTA;
    }
    if (input.mi.dx || input.mi.dy || report->v)
        SendInput(1, &input, sizeof(INPUT));
}

static void win_send_extra(report_extra_t *report) { (void)report; }

static host_driver_t windows_driver = {
    .keyboard_leds = win_keyboard_leds,
    .send_keyboard = win_send_keyboard,
    .send_nkro     = win_send_nkro,
    .send_mouse    = win_send_mouse,
    .send_extra    = win_send_extra,
};

// ---- Protocol interface ----

void protocol_setup(void) {
    timer_init();
}

void protocol_pre_init(void) {
    // Install the low-level keyboard hook
    keyboard_hook = SetWindowsHookEx(WH_KEYBOARD_LL, ll_keyboard_proc, NULL, 0);
    if (!keyboard_hook) {
        fprintf(stderr, "qapla: FATAL: SetWindowsHookEx failed (error %lu)\n", GetLastError());
        fprintf(stderr, "qapla: Try running as Administrator\n");
        exit(1);
    }
    fprintf(stderr, "qapla: keyboard hook installed\n");
}

void protocol_post_init(void) {
    host_set_driver(&windows_driver);
    fprintf(stderr, "qapla: QMK initialized, processing keys\n");
}

void protocol_pre_task(void) {
    if (!qapla_running()) {
        if (keyboard_hook) {
            UnhookWindowsHookEx(keyboard_hook);
            keyboard_hook = NULL;
        }
        fprintf(stderr, "qapla: shutting down\n");
        exit(0);
    }
}

void protocol_post_task(void) {
    // Process Windows messages — REQUIRED for LLHOOK to work.
    // Without a message pump, the hook callback never fires.
    // PeekMessage with PM_REMOVE processes pending messages
    // without blocking, so QMK's scan loop stays responsive.
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            // Someone posted WM_QUIT — shut down gracefully
            if (keyboard_hook) {
                UnhookWindowsHookEx(keyboard_hook);
                keyboard_hook = NULL;
            }
            exit(0);
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    // Yield ~1ms to not burn 100% CPU
    Sleep(1);
}
