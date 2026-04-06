// SPDX-License-Identifier: GPL-2.0-or-later
// QAP'LA: macOS Protocol Layer
//
// TWO APPROACHES (compile-time selection):
//
// 1. CGEventTap (default, no driver needed)
//    - Creates a Quartz event tap that intercepts keyboard events
//    - Posts processed events via CGEventPost
//    - Requires "Input Monitoring" or "Accessibility" permission in
//      System Preferences → Security & Privacy → Privacy
//    - Won't work until user grants permission (macOS shows a dialog)
//    - Runs in a CFRunLoop (like Windows message pump)
//
// 2. Karabiner VirtualHIDDevice (optional, if installed)
//    - Uses Karabiner's DriverKit extension for a proper virtual keyboard
//    - Grabs real keyboard via IOHIDManager with kIOHIDOptionsTypeSeizeDevice
//    - Posts to Karabiner's virtual HID device via IOKit
//    - Better: real device grab (like evdev EVIOCGRAB), invisible to other apps
//    - Requires Karabiner-DriverKit-VirtualHIDDevice installed
//
// This file implements approach 1 (CGEventTap). Approach 2 is TODO.
//
// MACOS EVENT SYSTEM OVERVIEW:
//   macOS uses Quartz (Core Graphics) for the event pipeline:
//     Hardware → IOKit HID → WindowServer → CGEventTap → Application
//   A CGEventTap at kCGHIDEventTap level intercepts events as they
//   leave the HID system, before any application sees them.
//   We can modify, suppress, or inject events at this point.
//
// CGEventTap + CFRunLoop:
//   Like Windows LLHOOK + message pump, macOS requires a run loop
//   for the event tap to fire. We integrate this with QMK's main
//   loop by running CFRunLoopRunInMode with a short timeout in
//   protocol_post_task.

#include <stdio.h>
#include <string.h>
#include <unistd.h>

// QMK headers first
#include "host.h"
#include "host_driver.h"
#include "report.h"
#include "timer.h"
#include "platform.h"

// macOS headers
#include <CoreGraphics/CoreGraphics.h>
#include <Carbon/Carbon.h>  // for kVK_* virtual key codes

// ---- macOS Virtual Key → USB HID keycode mapping ----
// macOS uses kVK_* constants (Carbon virtual key codes).
// These are NOT the same as Windows VK codes or evdev codes.
// The mapping is fixed by Apple and documented in Events.h.

static const uint8_t vk_to_hid[128] = {
    [kVK_ANSI_A]         = 0x04, [kVK_ANSI_S]         = 0x16,
    [kVK_ANSI_D]         = 0x07, [kVK_ANSI_F]         = 0x09,
    [kVK_ANSI_H]         = 0x0B, [kVK_ANSI_G]         = 0x0A,
    [kVK_ANSI_Z]         = 0x1D, [kVK_ANSI_X]         = 0x1B,
    [kVK_ANSI_C]         = 0x06, [kVK_ANSI_V]         = 0x19,
    [kVK_ANSI_B]         = 0x05, [kVK_ANSI_Q]         = 0x14,
    [kVK_ANSI_W]         = 0x1A, [kVK_ANSI_E]         = 0x08,
    [kVK_ANSI_R]         = 0x15, [kVK_ANSI_Y]         = 0x1C,
    [kVK_ANSI_T]         = 0x17, [kVK_ANSI_1]         = 0x1E,
    [kVK_ANSI_2]         = 0x1F, [kVK_ANSI_3]         = 0x20,
    [kVK_ANSI_4]         = 0x21, [kVK_ANSI_6]         = 0x23,
    [kVK_ANSI_5]         = 0x22, [kVK_ANSI_Equal]     = 0x2E,
    [kVK_ANSI_9]         = 0x26, [kVK_ANSI_7]         = 0x24,
    [kVK_ANSI_Minus]     = 0x2D, [kVK_ANSI_8]         = 0x25,
    [kVK_ANSI_0]         = 0x27, [kVK_ANSI_RightBracket] = 0x30,
    [kVK_ANSI_O]         = 0x12, [kVK_ANSI_U]         = 0x18,
    [kVK_ANSI_LeftBracket]= 0x2F,[kVK_ANSI_I]         = 0x0C,
    [kVK_ANSI_P]         = 0x13, [kVK_ANSI_L]         = 0x0F,
    [kVK_ANSI_J]         = 0x0D, [kVK_ANSI_Quote]     = 0x34,
    [kVK_ANSI_K]         = 0x0E, [kVK_ANSI_Semicolon] = 0x33,
    [kVK_ANSI_Backslash] = 0x31, [kVK_ANSI_Comma]     = 0x36,
    [kVK_ANSI_Slash]     = 0x38, [kVK_ANSI_N]         = 0x11,
    [kVK_ANSI_M]         = 0x10, [kVK_ANSI_Period]    = 0x37,
    [kVK_ANSI_Grave]     = 0x35,
    [kVK_ANSI_KeypadDecimal]  = 0x63, [kVK_ANSI_KeypadMultiply] = 0x55,
    [kVK_ANSI_KeypadPlus]     = 0x57, [kVK_ANSI_KeypadClear]    = 0x53,
    [kVK_ANSI_KeypadDivide]   = 0x54, [kVK_ANSI_KeypadEnter]    = 0x58,
    [kVK_ANSI_KeypadMinus]    = 0x56, [kVK_ANSI_KeypadEquals]   = 0x67,
    [kVK_ANSI_Keypad0] = 0x62, [kVK_ANSI_Keypad1] = 0x59,
    [kVK_ANSI_Keypad2] = 0x5A, [kVK_ANSI_Keypad3] = 0x5B,
    [kVK_ANSI_Keypad4] = 0x5C, [kVK_ANSI_Keypad5] = 0x5D,
    [kVK_ANSI_Keypad6] = 0x5E, [kVK_ANSI_Keypad7] = 0x5F,
    [kVK_ANSI_Keypad8] = 0x60, [kVK_ANSI_Keypad9] = 0x61,
    [kVK_Return]     = 0x28, [kVK_Tab]       = 0x2B,
    [kVK_Space]      = 0x2C, [kVK_Delete]    = 0x2A,  // backspace
    [kVK_Escape]     = 0x29, [kVK_CapsLock]  = 0x39,
    [kVK_F1]  = 0x3A, [kVK_F2]  = 0x3B, [kVK_F3]  = 0x3C,
    [kVK_F4]  = 0x3D, [kVK_F5]  = 0x3E, [kVK_F6]  = 0x3F,
    [kVK_F7]  = 0x40, [kVK_F8]  = 0x41, [kVK_F9]  = 0x42,
    [kVK_F10] = 0x43, [kVK_F11] = 0x44, [kVK_F12] = 0x45,
    [kVK_Home]     = 0x4A, [kVK_PageUp]     = 0x4B,
    [kVK_ForwardDelete] = 0x4C, [kVK_End]   = 0x4D,
    [kVK_PageDown] = 0x4E,
    [kVK_RightArrow] = 0x4F, [kVK_LeftArrow]  = 0x50,
    [kVK_DownArrow]  = 0x51, [kVK_UpArrow]    = 0x52,
    // Modifiers (flag-based on macOS, but also have VK codes)
    [kVK_Command]      = 0xE3,  // Left GUI
    [kVK_Shift]        = 0xE1,  // Left Shift
    [kVK_Option]       = 0xE2,  // Left Alt
    [kVK_Control]      = 0xE0,  // Left Control
    [kVK_RightCommand] = 0xE7,
    [kVK_RightShift]   = 0xE5,
    [kVK_RightOption]  = 0xE6,
    [kVK_RightControl] = 0xE4,
};

// Reverse: HID → macOS VK for CGEventPost output
static uint16_t hid_to_vk[256];  // populated at init from vk_to_hid

// ---- Key state tracking ----
#define KEY_STATE_MAX 128
static uint8_t key_state[KEY_STATE_MAX];
static CFMachPortRef event_tap = NULL;
static CFRunLoopSourceRef run_loop_source = NULL;

// ---- CGEventTap Callback ----
// Intercepts all keyboard events before applications see them.
//
// PERMISSIONS:
//   macOS requires the app to be in System Preferences → Security &
//   Privacy → Privacy → Input Monitoring (or Accessibility).
//   Without permission, CGEventTapCreate returns NULL.
//   The user will see a system dialog asking for permission.
//   After granting, the app needs to be restarted.
//
// EVENT FLOW:
//   1. User presses key on physical keyboard
//   2. IOKit HID driver creates HID event
//   3. WindowServer picks it up
//   4. Our CGEventTap fires (kCGHIDEventTap level)
//   5. We read the keycode, update key_state[]
//   6. Return NULL to suppress the original event
//   7. QMK processes the key in its scan loop
//   8. QMK calls send_keyboard() which calls CGEventPost

static CGEventRef event_tap_callback(
    CGEventTapProxy proxy,
    CGEventType type,
    CGEventRef event,
    void *user_info
) {
    (void)proxy;
    (void)user_info;

    // Re-enable if the tap gets disabled (macOS disables taps that
    // take too long to process — shouldn't happen at keyboard speed)
    if (type == kCGEventTapDisabledByTimeout ||
        type == kCGEventTapDisabledByUserInput) {
        CGEventTapEnable(event_tap, true);
        return event;
    }

    // Only handle keyboard events
    if (type != kCGEventKeyDown && type != kCGEventKeyUp &&
        type != kCGEventFlagsChanged) {
        return event;
    }

    // Check if this is our own injected event (prevent feedback loop)
    // We tag injected events with a custom field
    int64_t tag = CGEventGetIntegerValueField(event, kCGEventSourceUserData);
    if (tag == 0x51415041) {  // "QAPA" in hex
        return event;  // pass through our own events
    }

    int64_t keycode = CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);

    if (keycode >= 0 && keycode < KEY_STATE_MAX) {
        if (type == kCGEventKeyDown) {
            key_state[keycode] = 1;
        } else if (type == kCGEventKeyUp) {
            key_state[keycode] = 0;
        } else if (type == kCGEventFlagsChanged) {
            // Modifier keys use flag changes, not keydown/keyup
            CGEventFlags flags = CGEventGetFlags(event);
            // Determine press/release from the flag state
            key_state[keycode] = (flags & (kCGEventFlagMaskShift |
                kCGEventFlagMaskControl | kCGEventFlagMaskAlternate |
                kCGEventFlagMaskCommand)) ? 1 : 0;
        }
    }

    // Suppress the original event — QMK will re-emit via CGEventPost
    return NULL;
}

// ---- Matrix access for QMK ----
uint8_t qapla_key_state(uint16_t code) {
    if (code >= KEY_STATE_MAX) return 0;
    return key_state[code];
}

// ---- CGEventPost output ----

static void post_key(uint16_t vk, int pressed) {
    CGEventRef event = CGEventCreateKeyboardEvent(NULL, (CGKeyCode)vk, pressed);
    if (event) {
        // Tag as our own to prevent re-interception
        CGEventSetIntegerValueField(event, kCGEventSourceUserData, 0x51415041);
        CGEventPost(kCGHIDEventTap, event);
        CFRelease(event);
    }
}

static uint8_t darwin_keyboard_leds(void) {
    // macOS doesn't expose LED state the same way.
    // CGEventSourceKeyState could work for CapsLock.
    return CGEventSourceKeyState(kCGEventSourceStateCombinedSessionState, kVK_CapsLock) ? 2 : 0;
}

static uint8_t prev_keys[6];
static uint8_t prev_mods;

static void darwin_send_keyboard(report_keyboard_t *report) {
    // Modifiers
    uint8_t mod_hid[] = {0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7};
    for (int i = 0; i < 8; i++) {
        uint8_t old_bit = (prev_mods >> i) & 1;
        uint8_t new_bit = (report->mods >> i) & 1;
        if (old_bit != new_bit) {
            uint16_t vk = hid_to_vk[mod_hid[i]];
            if (vk < KEY_STATE_MAX) post_key(vk, new_bit);
        }
    }

    // Release old keys
    for (int i = 0; i < 6; i++) {
        uint8_t old_key = prev_keys[i];
        if (old_key == 0) continue;
        int found = 0;
        for (int j = 0; j < 6; j++)
            if (report->keys[j] == old_key) { found = 1; break; }
        if (!found) {
            uint16_t vk = hid_to_vk[old_key];
            if (vk < KEY_STATE_MAX) post_key(vk, 0);
        }
    }

    // Press new keys
    for (int i = 0; i < 6; i++) {
        uint8_t new_key = report->keys[i];
        if (new_key == 0) continue;
        int found = 0;
        for (int j = 0; j < 6; j++)
            if (prev_keys[j] == new_key) { found = 1; break; }
        if (!found) {
            uint16_t vk = hid_to_vk[new_key];
            if (vk < KEY_STATE_MAX) post_key(vk, 1);
        }
    }

    memcpy(prev_keys, report->keys, sizeof(prev_keys));
    prev_mods = report->mods;
}

static void darwin_send_nkro(report_nkro_t *report) { (void)report; }
static void darwin_send_mouse(report_mouse_t *report) { (void)report; /* TODO: CGEventPost mouse */ }
static void darwin_send_extra(report_extra_t *report) { (void)report; }

static host_driver_t darwin_driver = {
    .keyboard_leds = darwin_keyboard_leds,
    .send_keyboard = darwin_send_keyboard,
    .send_nkro     = darwin_send_nkro,
    .send_mouse    = darwin_send_mouse,
    .send_extra    = darwin_send_extra,
};

// ---- Protocol interface ----

void protocol_setup(void) {
    timer_init();

    // Build reverse lookup table
    memset(hid_to_vk, 0xFF, sizeof(hid_to_vk));
    for (int vk = 0; vk < 128; vk++) {
        uint8_t hid = vk_to_hid[vk];
        if (hid) hid_to_vk[hid] = vk;
    }
}

void protocol_pre_init(void) {
    // Create CGEventTap
    CGEventMask mask = (1 << kCGEventKeyDown) | (1 << kCGEventKeyUp) |
                       (1 << kCGEventFlagsChanged);

    event_tap = CGEventTapCreate(
        kCGHIDEventTap,           // tap at HID level (before apps)
        kCGHeadInsertEventTap,    // insert at head of event chain
        kCGEventTapOptionDefault, // active tap (can modify/suppress)
        mask,
        event_tap_callback,
        NULL
    );

    if (!event_tap) {
        fprintf(stderr,
            "qapla: FATAL: CGEventTapCreate failed.\n"
            "qapla: Grant Input Monitoring permission in:\n"
            "qapla:   System Preferences → Security & Privacy → Privacy → Input Monitoring\n"
            "qapla: Then restart qapla.\n");
        exit(1);
    }

    // Add to the current run loop
    run_loop_source = CFMachPortCreateRunLoopSource(NULL, event_tap, 0);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), run_loop_source, kCFRunLoopCommonModes);
    CGEventTapEnable(event_tap, true);

    fprintf(stderr, "qapla: keyboard event tap installed\n");
}

void protocol_post_init(void) {
    host_set_driver(&darwin_driver);
    fprintf(stderr, "qapla: QMK initialized, processing keys\n");
}

void protocol_pre_task(void) {
    if (!qapla_running()) {
        if (event_tap) {
            CGEventTapEnable(event_tap, false);
            CFRelease(event_tap);
            event_tap = NULL;
        }
        if (run_loop_source) {
            CFRelease(run_loop_source);
            run_loop_source = NULL;
        }
        fprintf(stderr, "qapla: shutting down\n");
        exit(0);
    }
}

void protocol_post_task(void) {
    // Run the CFRunLoop briefly to process pending events.
    // Like Windows PeekMessage — process queued events without blocking.
    // kCFRunLoopRunHandledSource = we processed something
    // kCFRunLoopRunTimedOut = nothing pending
    // 0.001 second timeout = 1ms, matches QMK's ~1kHz scan rate
    CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.001, false);
}
