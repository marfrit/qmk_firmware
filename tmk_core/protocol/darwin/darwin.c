// SPDX-License-Identifier: GPL-2.0-or-later
// QAP'LA: macOS Protocol Layer
//
// TWO BACKENDS (runtime selection):
//
// 1. IOHIDManager + Karabiner VirtualHIDDevice (preferred)
//    Input:  IOHIDDeviceOpen with kIOHIDOptionsTypeSeizeDevice
//            True exclusive grab — events never reach macOS
//    Output: Karabiner's DriverKit virtual keyboard daemon
//            Creates a real HID device visible to all apps
//    Pros:   Proper grab (like evdev EVIOCGRAB), pre-Fn-key intercept
//    Cons:   Requires Karabiner-DriverKit-VirtualHIDDevice installed
//    Ref:    kmonad's mac/keyio_mac.hpp, mac/dext.cpp
//
// 2. CGEventTap (fallback, no driver dependency)
//    Input:  CGEventTapCreate at kCGHIDEventTap level
//    Output: CGEventPost with injection tag to prevent feedback
//    Pros:   No driver needed, just Input Monitoring permission
//    Cons:   Not a true grab (some macOS internals still see events),
//            post-Fn-key translation, CGEventPost limitations
//
// RUNTIME SELECTION:
//   We try Karabiner first. If the daemon socket doesn't exist
//   (~Library/Application Support/org.pqrs/tmp/rootonly/vhidd_server/),
//   fall back to CGEventTap.
//
// INSTALLING THE KARABINER DRIVER:
//   The easiest way is to install Karabiner-Elements:
//     brew install --cask karabiner-elements
//   This installs the DriverKit virtual HID device and its daemon.
//   You don't need to use Karabiner-Elements itself — just the driver.
//
//   Alternatively, install just the driver (manual):
//     1. Download Karabiner-DriverKit-VirtualHIDDevice from:
//        https://github.com/pqrs-org/Karabiner-DriverKit-VirtualHIDDevice/releases
//     2. Run the installer .pkg
//     3. Approve the System Extension in System Preferences → Security
//     4. Start the daemon:
//        sudo launchctl load /Library/LaunchDaemons/org.pqrs.Karabiner-DriverKit-VirtualHIDDevice-Daemon.plist
//     5. Verify:
//        ls /Library/Application\ Support/org.pqrs/tmp/rootonly/vhidd_server/
//
//   kmonad users will already have this installed.
//
// ARCHITECTURE:
//
//   Backend 1 (Karabiner):
//   ┌──────────────┐         ┌───────────────────┐
//   │ IOHIDManager │         │ Karabiner dext     │
//   │ (seize real   │         │ virtual keyboard   │
//   │  keyboard)    │───QMK──▸│ via Unix socket    │
//   └──────────────┘         └───────────────────┘
//
//   Backend 2 (CGEventTap):
//   ┌──────────────┐         ┌───────────────────┐
//   │ CGEventTap   │         │ CGEventPost       │
//   │ (intercept +  │───QMK──▸│ (inject with tag) │
//   │  suppress)    │         └───────────────────┘
//   └──────────────┘

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

// QMK headers
#include "host.h"
#include "host_driver.h"
#include "report.h"
#include "timer.h"
#include "platform.h"

// macOS headers
#include <CoreGraphics/CoreGraphics.h>
#include <Carbon/Carbon.h>  // for kVK_* virtual key codes
#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDLib.h>

// ============================================================================
// SHARED: macOS VK → USB HID mapping
// ============================================================================

static const uint8_t vk_to_hid[128] = {
    [kVK_ANSI_A] = 0x04, [kVK_ANSI_S] = 0x16, [kVK_ANSI_D] = 0x07,
    [kVK_ANSI_F] = 0x09, [kVK_ANSI_H] = 0x0B, [kVK_ANSI_G] = 0x0A,
    [kVK_ANSI_Z] = 0x1D, [kVK_ANSI_X] = 0x1B, [kVK_ANSI_C] = 0x06,
    [kVK_ANSI_V] = 0x19, [kVK_ANSI_B] = 0x05, [kVK_ANSI_Q] = 0x14,
    [kVK_ANSI_W] = 0x1A, [kVK_ANSI_E] = 0x08, [kVK_ANSI_R] = 0x15,
    [kVK_ANSI_Y] = 0x1C, [kVK_ANSI_T] = 0x17, [kVK_ANSI_1] = 0x1E,
    [kVK_ANSI_2] = 0x1F, [kVK_ANSI_3] = 0x20, [kVK_ANSI_4] = 0x21,
    [kVK_ANSI_6] = 0x23, [kVK_ANSI_5] = 0x22, [kVK_ANSI_Equal] = 0x2E,
    [kVK_ANSI_9] = 0x26, [kVK_ANSI_7] = 0x24, [kVK_ANSI_Minus] = 0x2D,
    [kVK_ANSI_8] = 0x25, [kVK_ANSI_0] = 0x27,
    [kVK_ANSI_RightBracket] = 0x30, [kVK_ANSI_O] = 0x12,
    [kVK_ANSI_U] = 0x18, [kVK_ANSI_LeftBracket] = 0x2F,
    [kVK_ANSI_I] = 0x0C, [kVK_ANSI_P] = 0x13, [kVK_ANSI_L] = 0x0F,
    [kVK_ANSI_J] = 0x0D, [kVK_ANSI_Quote] = 0x34, [kVK_ANSI_K] = 0x0E,
    [kVK_ANSI_Semicolon] = 0x33, [kVK_ANSI_Backslash] = 0x31,
    [kVK_ANSI_Comma] = 0x36, [kVK_ANSI_Slash] = 0x38,
    [kVK_ANSI_N] = 0x11, [kVK_ANSI_M] = 0x10, [kVK_ANSI_Period] = 0x37,
    [kVK_ANSI_Grave] = 0x35,
    [kVK_ANSI_KeypadDecimal] = 0x63, [kVK_ANSI_KeypadMultiply] = 0x55,
    [kVK_ANSI_KeypadPlus] = 0x57, [kVK_ANSI_KeypadClear] = 0x53,
    [kVK_ANSI_KeypadDivide] = 0x54, [kVK_ANSI_KeypadEnter] = 0x58,
    [kVK_ANSI_KeypadMinus] = 0x56, [kVK_ANSI_KeypadEquals] = 0x67,
    [kVK_ANSI_Keypad0] = 0x62, [kVK_ANSI_Keypad1] = 0x59,
    [kVK_ANSI_Keypad2] = 0x5A, [kVK_ANSI_Keypad3] = 0x5B,
    [kVK_ANSI_Keypad4] = 0x5C, [kVK_ANSI_Keypad5] = 0x5D,
    [kVK_ANSI_Keypad6] = 0x5E, [kVK_ANSI_Keypad7] = 0x5F,
    [kVK_ANSI_Keypad8] = 0x60, [kVK_ANSI_Keypad9] = 0x61,
    [kVK_Return] = 0x28, [kVK_Tab] = 0x2B, [kVK_Space] = 0x2C,
    [kVK_Delete] = 0x2A, [kVK_Escape] = 0x29, [kVK_CapsLock] = 0x39,
    [kVK_F1] = 0x3A, [kVK_F2] = 0x3B, [kVK_F3] = 0x3C,
    [kVK_F4] = 0x3D, [kVK_F5] = 0x3E, [kVK_F6] = 0x3F,
    [kVK_F7] = 0x40, [kVK_F8] = 0x41, [kVK_F9] = 0x42,
    [kVK_F10] = 0x43, [kVK_F11] = 0x44, [kVK_F12] = 0x45,
    [kVK_Home] = 0x4A, [kVK_PageUp] = 0x4B, [kVK_ForwardDelete] = 0x4C,
    [kVK_End] = 0x4D, [kVK_PageDown] = 0x4E,
    [kVK_RightArrow] = 0x4F, [kVK_LeftArrow] = 0x50,
    [kVK_DownArrow] = 0x51, [kVK_UpArrow] = 0x52,
    [kVK_Command] = 0xE3, [kVK_Shift] = 0xE1,
    [kVK_Option] = 0xE2, [kVK_Control] = 0xE0,
    [kVK_RightCommand] = 0xE7, [kVK_RightShift] = 0xE5,
    [kVK_RightOption] = 0xE6, [kVK_RightControl] = 0xE4,
};

static uint16_t hid_to_vk[256]; // reverse, populated at init

// ---- Key state (shared between backends) ----
#define KEY_STATE_MAX 128
static uint8_t key_state[KEY_STATE_MAX];

uint8_t qapla_key_state(uint16_t code) {
    if (code >= KEY_STATE_MAX) return 0;
    return key_state[code];
}


// ============================================================================
// BACKEND 1: IOHIDManager + Karabiner dext
// ============================================================================

#ifdef QAPLA_KARABINER_BACKEND

// Input: IOHIDManager seizes all keyboard devices
static IOHIDManagerRef hid_manager = NULL;
static int hid_pipe[2] = {-1, -1}; // pipe for HID callback → main loop

typedef struct {
    uint64_t type;   // 0 = keydown, 1 = keyup
    uint32_t page;   // HID usage page
    uint32_t usage;  // HID usage (keycode)
} hid_key_event_t;

static void hid_input_callback(void *ctx, IOReturn result, void *sender,
                                IOHIDValueRef value) {
    (void)ctx; (void)result; (void)sender;

    IOHIDElementRef elem = IOHIDValueGetElement(value);
    uint32_t page  = IOHIDElementGetUsagePage(elem);
    uint32_t usage = IOHIDElementGetUsage(elem);
    CFIndex  pressed = IOHIDValueGetIntegerValue(value);

    // Only keyboard/keypad page
    if (page != kHIDPage_KeyboardOrKeypad) return;
    if (usage < 4 || usage > 231) return;  // skip non-key usages

    hid_key_event_t ev;
    ev.type  = pressed ? 0 : 1;
    ev.page  = page;
    ev.usage = usage;

    // Write to pipe (non-blocking, drop if full)
    write(hid_pipe[1], &ev, sizeof(ev));
}

static void hid_device_added(void *ctx, IOReturn result, void *sender,
                              IOHIDDeviceRef device) {
    (void)ctx; (void)result; (void)sender;

    // Skip Karabiner's own virtual devices
    CFStringRef product = IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductKey));
    if (product) {
        char name[256];
        CFStringGetCString(product, name, sizeof(name), kCFStringEncodingUTF8);
        if (strstr(name, "Karabiner") || strstr(name, "QAP")) {
            fprintf(stderr, "qapla: skipping virtual device: %s\n", name);
            return;
        }
        fprintf(stderr, "qapla: seizing keyboard: %s\n", name);
    }

    IOHIDDeviceOpen(device, kIOHIDOptionsTypeSeizeDevice);
    IOHIDDeviceRegisterInputValueCallback(device, hid_input_callback, NULL);
}

static bool init_hid_manager(void) {
    if (pipe(hid_pipe) < 0) return false;

    hid_manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
    if (!hid_manager) return false;

    // Match keyboard devices
    CFMutableDictionaryRef match = CFDictionaryCreateMutable(
        kCFAllocatorDefault, 0,
        &kCFTypeDictionaryKeyCallBacks,
        &kCFTypeDictionaryValueCallBacks);

    int page = kHIDPage_GenericDesktop;
    int usage = kHIDUsage_GD_Keyboard;
    CFNumberRef pageRef  = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &page);
    CFNumberRef usageRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &usage);
    CFDictionarySetValue(match, CFSTR(kIOHIDDeviceUsagePageKey), pageRef);
    CFDictionarySetValue(match, CFSTR(kIOHIDDeviceUsageKey), usageRef);
    CFRelease(pageRef);
    CFRelease(usageRef);

    IOHIDManagerSetDeviceMatching(hid_manager, match);
    CFRelease(match);

    IOHIDManagerRegisterDeviceMatchingCallback(hid_manager, hid_device_added, NULL);
    IOHIDManagerScheduleWithRunLoop(hid_manager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);

    IOReturn ret = IOHIDManagerOpen(hid_manager, kIOHIDOptionsTypeSeizeDevice);
    if (ret != kIOReturnSuccess) {
        fprintf(stderr, "qapla: IOHIDManagerOpen failed: 0x%x\n", ret);
        fprintf(stderr, "qapla: Run with sudo, or grant Input Monitoring permission\n");
        return false;
    }

    return true;
}

// Output: Karabiner DriverKit virtual keyboard
// TODO: Implement Karabiner dext client (Unix socket communication)
// For now, fall back to CGEventPost even in Karabiner mode.
// The input side (IOHIDManager seize) is the valuable part.

#endif // QAPLA_KARABINER_BACKEND


// ============================================================================
// BACKEND 2: CGEventTap (fallback)
// ============================================================================

static CFMachPortRef event_tap = NULL;
static CFRunLoopSourceRef run_loop_source = NULL;

static CGEventRef event_tap_callback(
    CGEventTapProxy proxy, CGEventType type,
    CGEventRef event, void *user_info
) {
    (void)proxy; (void)user_info;

    if (type == kCGEventTapDisabledByTimeout ||
        type == kCGEventTapDisabledByUserInput) {
        CGEventTapEnable(event_tap, true);
        return event;
    }

    if (type != kCGEventKeyDown && type != kCGEventKeyUp &&
        type != kCGEventFlagsChanged) {
        return event;
    }

    // Skip our own injected events
    if (CGEventGetIntegerValueField(event, kCGEventSourceUserData) == 0x51415041)
        return event;

    int64_t keycode = CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);
    if (keycode >= 0 && keycode < KEY_STATE_MAX) {
        if (type == kCGEventKeyDown)
            key_state[keycode] = 1;
        else if (type == kCGEventKeyUp)
            key_state[keycode] = 0;
        else if (type == kCGEventFlagsChanged) {
            // Toggle based on current modifier state
            CGEventFlags flags = CGEventGetFlags(event);
            key_state[keycode] = (flags != 0) ? 1 : 0;
        }
    }

    return NULL; // suppress original
}

static bool init_event_tap(void) {
    CGEventMask mask = (1 << kCGEventKeyDown) | (1 << kCGEventKeyUp) |
                       (1 << kCGEventFlagsChanged);

    event_tap = CGEventTapCreate(
        kCGHIDEventTap, kCGHeadInsertEventTap,
        kCGEventTapOptionDefault, mask,
        event_tap_callback, NULL);

    if (!event_tap) {
        fprintf(stderr,
            "qapla: CGEventTapCreate failed.\n"
            "qapla: Grant Input Monitoring permission in:\n"
            "qapla:   System Settings → Privacy & Security → Input Monitoring\n");
        return false;
    }

    run_loop_source = CFMachPortCreateRunLoopSource(NULL, event_tap, 0);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), run_loop_source, kCFRunLoopCommonModes);
    CGEventTapEnable(event_tap, true);
    return true;
}


// ============================================================================
// OUTPUT: CGEventPost (used by both backends for now)
// ============================================================================
// TODO: When Karabiner dext client is implemented, use
// async_post_report() for the Karabiner backend instead of CGEventPost.

static void post_key(uint16_t vk, int pressed) {
    CGEventRef event = CGEventCreateKeyboardEvent(NULL, (CGKeyCode)vk, pressed);
    if (event) {
        CGEventSetIntegerValueField(event, kCGEventSourceUserData, 0x51415041);
        CGEventPost(kCGHIDEventTap, event);
        CFRelease(event);
    }
}

static uint8_t darwin_keyboard_leds(void) {
    return CGEventSourceKeyState(kCGEventSourceStateCombinedSessionState, kVK_CapsLock) ? 2 : 0;
}

static uint8_t prev_keys[6];
static uint8_t prev_mods;

static void darwin_send_keyboard(report_keyboard_t *report) {
    uint8_t mod_hid[] = {0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7};
    for (int i = 0; i < 8; i++) {
        uint8_t old_bit = (prev_mods >> i) & 1;
        uint8_t new_bit = (report->mods >> i) & 1;
        if (old_bit != new_bit) {
            uint16_t vk = hid_to_vk[mod_hid[i]];
            if (vk < KEY_STATE_MAX) post_key(vk, new_bit);
        }
    }

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
static void darwin_send_mouse(report_mouse_t *report) { (void)report; }
static void darwin_send_extra(report_extra_t *report) { (void)report; }

static host_driver_t darwin_driver = {
    .keyboard_leds = darwin_keyboard_leds,
    .send_keyboard = darwin_send_keyboard,
    .send_nkro     = darwin_send_nkro,
    .send_mouse    = darwin_send_mouse,
    .send_extra    = darwin_send_extra,
};


// ============================================================================
// PROTOCOL INTERFACE
// ============================================================================

static enum { BACKEND_NONE, BACKEND_KARABINER, BACKEND_CGEVENTTAP } active_backend;

void protocol_setup(void) {
    timer_init();
    memset(hid_to_vk, 0xFF, sizeof(hid_to_vk));
    for (int vk = 0; vk < 128; vk++) {
        uint8_t hid = vk_to_hid[vk];
        if (hid) hid_to_vk[hid] = vk;
    }
}

void protocol_pre_init(void) {
    active_backend = BACKEND_NONE;

    // Try Karabiner first (check if daemon socket exists)
#ifdef QAPLA_KARABINER_BACKEND
    struct stat st;
    if (stat("/Library/Application Support/org.pqrs/tmp/rootonly/vhidd_server", &st) == 0) {
        fprintf(stderr, "qapla: Karabiner DriverKit daemon detected\n");
        if (init_hid_manager()) {
            active_backend = BACKEND_KARABINER;
            fprintf(stderr, "qapla: using IOHIDManager + Karabiner backend\n");
            fprintf(stderr, "qapla: NOTE: output still via CGEventPost (dext client TODO)\n");
        }
    }
#endif

    // Fall back to CGEventTap
    if (active_backend == BACKEND_NONE) {
        if (init_event_tap()) {
            active_backend = BACKEND_CGEVENTTAP;
            fprintf(stderr, "qapla: using CGEventTap backend (no Karabiner)\n");
        } else {
            fprintf(stderr, "qapla: FATAL: no backend available\n");
            exit(1);
        }
    }
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
        }
        if (run_loop_source) CFRelease(run_loop_source);
#ifdef QAPLA_KARABINER_BACKEND
        if (hid_manager) {
            IOHIDManagerClose(hid_manager, kIOHIDOptionsTypeNone);
            CFRelease(hid_manager);
        }
        if (hid_pipe[0] >= 0) { close(hid_pipe[0]); close(hid_pipe[1]); }
#endif
        fprintf(stderr, "qapla: shutting down\n");
        exit(0);
    }

#ifdef QAPLA_KARABINER_BACKEND
    // Read HID events from pipe (Karabiner backend)
    if (active_backend == BACKEND_KARABINER) {
        hid_key_event_t ev;
        while (read(hid_pipe[0], &ev, sizeof(ev)) == sizeof(ev)) {
            // HID usage → macOS VK → key_state
            // HID usages 0x04-0xE7 map to vk_to_hid in reverse
            // But we already have HID codes from IOKit, so we can
            // map directly to the matrix via usage code.
            // For now, find the VK that maps to this HID usage:
            if (ev.usage < 256) {
                uint16_t vk = hid_to_vk[ev.usage];
                if (vk < KEY_STATE_MAX) {
                    key_state[vk] = (ev.type == 0) ? 1 : 0;
                }
            }
        }
    }
#endif
}

void protocol_post_task(void) {
    // Process CFRunLoop events (both backends need this)
    CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.001, false);
}
