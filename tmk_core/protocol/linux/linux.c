// SPDX-License-Identifier: GPL-2.0-or-later
// QAP'LA: QMK As Process, Linux Adaptation
// Protocol layer: evdev input, uinput output

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <linux/uhid.h>

#include "host.h"
#include "host_driver.h"
#include "report.h"
#include "timer.h"
#include "platform.h"

#ifdef RAW_ENABLE
void raw_hid_receive(uint8_t *data, uint8_t length);
#endif

// Evdev → USB HID keycode mapping (Linux input.h KEY_* → USB HID usage)
// Only the keys that differ from identity or need explicit mapping
static const uint8_t evdev_to_hid[256] = {
    [KEY_RESERVED]   = 0x00,
    [KEY_ESC]        = 0x29,
    [KEY_1]          = 0x1E, [KEY_2] = 0x1F, [KEY_3] = 0x20, [KEY_4] = 0x21,
    [KEY_5]          = 0x22, [KEY_6] = 0x23, [KEY_7] = 0x24, [KEY_8] = 0x25,
    [KEY_9]          = 0x26, [KEY_0] = 0x27,
    [KEY_MINUS]      = 0x2D, [KEY_EQUAL] = 0x2E,
    [KEY_BACKSPACE]  = 0x2A,
    [KEY_TAB]        = 0x2B,
    [KEY_Q]          = 0x14, [KEY_W] = 0x1A, [KEY_E] = 0x08, [KEY_R] = 0x15,
    [KEY_T]          = 0x17, [KEY_Y] = 0x1C, [KEY_U] = 0x18, [KEY_I] = 0x0C,
    [KEY_O]          = 0x12, [KEY_P] = 0x13,
    [KEY_LEFTBRACE]  = 0x2F, [KEY_RIGHTBRACE] = 0x30,
    [KEY_ENTER]      = 0x28,
    [KEY_LEFTCTRL]   = 0xE0,
    [KEY_A]          = 0x04, [KEY_S] = 0x16, [KEY_D] = 0x07, [KEY_F] = 0x09,
    [KEY_G]          = 0x0A, [KEY_H] = 0x0B, [KEY_J] = 0x0D, [KEY_K] = 0x0E,
    [KEY_L]          = 0x0F,
    [KEY_SEMICOLON]  = 0x33, [KEY_APOSTROPHE] = 0x34,
    [KEY_GRAVE]      = 0x35,
    [KEY_LEFTSHIFT]  = 0xE1,
    [KEY_BACKSLASH]  = 0x31,
    [KEY_Z]          = 0x1D, [KEY_X] = 0x1B, [KEY_C] = 0x06, [KEY_V] = 0x19,
    [KEY_B]          = 0x05, [KEY_N] = 0x11, [KEY_M] = 0x10,
    [KEY_COMMA]      = 0x36, [KEY_DOT] = 0x37, [KEY_SLASH] = 0x38,
    [KEY_RIGHTSHIFT] = 0xE5,
    [KEY_KPASTERISK] = 0x55,
    [KEY_LEFTALT]    = 0xE2,
    [KEY_SPACE]      = 0x2C,
    [KEY_CAPSLOCK]   = 0x39,
    [KEY_F1]         = 0x3A, [KEY_F2] = 0x3B, [KEY_F3] = 0x3C, [KEY_F4] = 0x3D,
    [KEY_F5]         = 0x3E, [KEY_F6] = 0x3F, [KEY_F7] = 0x40, [KEY_F8] = 0x41,
    [KEY_F9]         = 0x42, [KEY_F10] = 0x43,
    [KEY_NUMLOCK]    = 0x53, [KEY_SCROLLLOCK] = 0x47,
    [KEY_KP7]        = 0x5F, [KEY_KP8] = 0x60, [KEY_KP9] = 0x61,
    [KEY_KPMINUS]    = 0x56,
    [KEY_KP4]        = 0x5C, [KEY_KP5] = 0x5D, [KEY_KP6] = 0x5E,
    [KEY_KPPLUS]     = 0x57,
    [KEY_KP1]        = 0x59, [KEY_KP2] = 0x5A, [KEY_KP3] = 0x5B,
    [KEY_KP0]        = 0x62,
    [KEY_KPDOT]      = 0x63,
    [KEY_102ND]      = 0x64, // non-US backslash
    [KEY_F11]        = 0x44, [KEY_F12] = 0x45,
    [KEY_KPENTER]    = 0x58,
    [KEY_RIGHTCTRL]  = 0xE4,
    [KEY_KPSLASH]    = 0x54,
    [KEY_SYSRQ]      = 0x46,
    [KEY_RIGHTALT]   = 0xE6,
    [KEY_HOME]       = 0x4A, [KEY_UP] = 0x52,
    [KEY_PAGEUP]     = 0x4B,
    [KEY_LEFT]       = 0x50, [KEY_RIGHT] = 0x4F,
    [KEY_END]        = 0x4D, [KEY_DOWN] = 0x51,
    [KEY_PAGEDOWN]   = 0x4E,
    [KEY_INSERT]     = 0x49, [KEY_DELETE] = 0x4C,
    [KEY_LEFTMETA]   = 0xE3, [KEY_RIGHTMETA] = 0xE7,
    [KEY_COMPOSE]    = 0x65,
    [KEY_PAUSE]      = 0x48,
};

// ---- Evdev state ----
// Support multiple evdev devices for keyboards with multiple interfaces
// (e.g. AMIRA USB keyboard exposes event1 and event8 as identical
// keyboard interfaces — keystrokes may come from either one)
#define EVDEV_MAX_DEVICES 8
static int evdev_fds[EVDEV_MAX_DEVICES];
static int evdev_count = 0;

// Track raw key states from evdev (for the custom matrix to read)
#define EVDEV_KEY_MAX 256
static uint8_t evdev_key_state[EVDEV_KEY_MAX];

int qapla_evdev_fd(void) { return evdev_count > 0 ? evdev_fds[0] : -1; }

uint8_t qapla_hid_for_evdev(uint16_t code) {
    if (code >= EVDEV_KEY_MAX) return 0;
    return evdev_to_hid[code];
}

uint8_t qapla_evdev_key_state(uint16_t code) {
    if (code >= EVDEV_KEY_MAX) return 0;
    return evdev_key_state[code];
}

// ---- UHID state (mughwI' — Vial raw HID channel) ----
static int uhid_fd = -1;

// HID report descriptor for Vial raw HID (Usage Page 0xFF60, Usage 0x61)
static const uint8_t raw_hid_report_desc[] = {
    0x06, 0x60, 0xFF,  // Usage Page (Vendor Defined 0xFF60)
    0x09, 0x61,        // Usage (0x61)
    0xA1, 0x01,        // Collection (Application)
    0x09, 0x62,        //   Usage (0x62)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x95, 0x20,        //   Report Count (32)
    0x75, 0x08,        //   Report Size (8)
    0x81, 0x02,        //   Input (Data, Variable, Absolute)
    0x09, 0x63,        //   Usage (0x63)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x95, 0x20,        //   Report Count (32)
    0x75, 0x08,        //   Report Size (8)
    0x91, 0x02,        //   Output (Data, Variable, Absolute)
    0xC0               // End Collection
};

static int open_uhid(void) {
    int fd = open("/dev/uhid", O_RDWR | O_NONBLOCK);
    if (fd < 0) {
        fprintf(stderr, "qapla: could not open /dev/uhid: %s (Vial disabled)\n", strerror(errno));
        return -1;
    }

    struct uhid_event ev = {0};
    ev.type = UHID_CREATE2;
    snprintf((char *)ev.u.create2.name, sizeof(ev.u.create2.name),
             "QAP'LA mughwI'");
    ev.u.create2.bus     = BUS_USB;
    ev.u.create2.vendor  = 0x1209;
    ev.u.create2.product = 0x4B42;
    ev.u.create2.version = 1;
    // Vial GUI discovery key: serial must contain "vial:f64c2b3c"
    snprintf((char *)ev.u.create2.uniq, sizeof(ev.u.create2.uniq),
             "vial:f64c2b3c");
    ev.u.create2.rd_size = sizeof(raw_hid_report_desc);
    memcpy(ev.u.create2.rd_data, raw_hid_report_desc, sizeof(raw_hid_report_desc));

    if (write(fd, &ev, sizeof(ev)) < 0) {
        fprintf(stderr, "qapla: uhid create failed: %s\n", strerror(errno));
        close(fd);
        return -1;
    }

    // Wait for kernel to create the hidraw device
    usleep(100000);

    fprintf(stderr, "qapla: mughwI' (Vial raw HID) device created\n");
    return fd;
}

static void uhid_send_input(const uint8_t *data, uint8_t length) {
    if (uhid_fd < 0) return;

    struct uhid_event ev = {0};
    ev.type = UHID_INPUT2;
    ev.u.input2.size = length;
    memcpy(ev.u.input2.data, data, length);

    if (write(uhid_fd, &ev, sizeof(ev)) < 0) {
        if (errno != EAGAIN)
            fprintf(stderr, "qapla: uhid send failed: %s\n", strerror(errno));
    }
}

static void uhid_poll(void) {
    if (uhid_fd < 0) return;

    struct uhid_event ev;
    while (read(uhid_fd, &ev, sizeof(ev)) > 0) {
        if (ev.type == UHID_OUTPUT) {
#ifdef RAW_ENABLE
            // Vial GUI sends 33 bytes (report ID 0 + 32 data bytes)
            // UHID strips the report ID, so we get the raw 32 bytes
            uint8_t buf[32];
            uint8_t len = ev.u.output.size;
            if (len > 32) len = 32;
            memcpy(buf, ev.u.output.data, len);
            raw_hid_receive(buf, len);
#endif
        }
    }
}

// ---- Uinput state ----
static int uinput_fd = -1;

static void uinput_emit(int type, int code, int value) {
    struct input_event ev = {0};
    ev.type  = type;
    ev.code  = code;
    ev.value = value;
    write(uinput_fd, &ev, sizeof(ev));
}

static void uinput_sync(void) {
    uinput_emit(EV_SYN, SYN_REPORT, 0);
}

// ---- HID → evdev output mapping (reverse of input) ----
static const uint16_t hid_to_evdev[256] = {
    [0x04] = KEY_A, [0x05] = KEY_B, [0x06] = KEY_C, [0x07] = KEY_D,
    [0x08] = KEY_E, [0x09] = KEY_F, [0x0A] = KEY_G, [0x0B] = KEY_H,
    [0x0C] = KEY_I, [0x0D] = KEY_J, [0x0E] = KEY_K, [0x0F] = KEY_L,
    [0x10] = KEY_M, [0x11] = KEY_N, [0x12] = KEY_O, [0x13] = KEY_P,
    [0x14] = KEY_Q, [0x15] = KEY_R, [0x16] = KEY_S, [0x17] = KEY_T,
    [0x18] = KEY_U, [0x19] = KEY_V, [0x1A] = KEY_W, [0x1B] = KEY_X,
    [0x1C] = KEY_Y, [0x1D] = KEY_Z,
    [0x1E] = KEY_1, [0x1F] = KEY_2, [0x20] = KEY_3, [0x21] = KEY_4,
    [0x22] = KEY_5, [0x23] = KEY_6, [0x24] = KEY_7, [0x25] = KEY_8,
    [0x26] = KEY_9, [0x27] = KEY_0,
    [0x28] = KEY_ENTER, [0x29] = KEY_ESC, [0x2A] = KEY_BACKSPACE,
    [0x2B] = KEY_TAB, [0x2C] = KEY_SPACE,
    [0x2D] = KEY_MINUS, [0x2E] = KEY_EQUAL,
    [0x2F] = KEY_LEFTBRACE, [0x30] = KEY_RIGHTBRACE,
    [0x31] = KEY_BACKSLASH, [0x33] = KEY_SEMICOLON, [0x34] = KEY_APOSTROPHE,
    [0x35] = KEY_GRAVE, [0x36] = KEY_COMMA, [0x37] = KEY_DOT, [0x38] = KEY_SLASH,
    [0x39] = KEY_CAPSLOCK,
    [0x3A] = KEY_F1, [0x3B] = KEY_F2, [0x3C] = KEY_F3, [0x3D] = KEY_F4,
    [0x3E] = KEY_F5, [0x3F] = KEY_F6, [0x40] = KEY_F7, [0x41] = KEY_F8,
    [0x42] = KEY_F9, [0x43] = KEY_F10, [0x44] = KEY_F11, [0x45] = KEY_F12,
    [0x46] = KEY_SYSRQ, [0x47] = KEY_SCROLLLOCK, [0x48] = KEY_PAUSE,
    [0x49] = KEY_INSERT, [0x4A] = KEY_HOME, [0x4B] = KEY_PAGEUP,
    [0x4C] = KEY_DELETE, [0x4D] = KEY_END, [0x4E] = KEY_PAGEDOWN,
    [0x4F] = KEY_RIGHT, [0x50] = KEY_LEFT, [0x51] = KEY_DOWN, [0x52] = KEY_UP,
    [0x53] = KEY_NUMLOCK,
    [0x54] = KEY_KPSLASH, [0x55] = KEY_KPASTERISK, [0x56] = KEY_KPMINUS,
    [0x57] = KEY_KPPLUS, [0x58] = KEY_KPENTER,
    [0x59] = KEY_KP1, [0x5A] = KEY_KP2, [0x5B] = KEY_KP3,
    [0x5C] = KEY_KP4, [0x5D] = KEY_KP5, [0x5E] = KEY_KP6,
    [0x5F] = KEY_KP7, [0x60] = KEY_KP8, [0x61] = KEY_KP9,
    [0x62] = KEY_KP0, [0x63] = KEY_KPDOT,
    [0x64] = KEY_102ND, [0x65] = KEY_COMPOSE,
    // Modifiers
    [0xE0] = KEY_LEFTCTRL, [0xE1] = KEY_LEFTSHIFT, [0xE2] = KEY_LEFTALT,
    [0xE3] = KEY_LEFTMETA, [0xE4] = KEY_RIGHTCTRL, [0xE5] = KEY_RIGHTSHIFT,
    [0xE6] = KEY_RIGHTALT, [0xE7] = KEY_RIGHTMETA,
};

// Track what we've sent so we can diff
static uint8_t prev_keys[KEYBOARD_REPORT_KEYS];
static uint8_t prev_mods;

// ---- Host driver callbacks ----

static uint8_t linux_keyboard_leds(void) {
    // Could read LED state from evdev, but not critical
    return 0;
}

static void linux_send_keyboard(report_keyboard_t *report) {
    if (uinput_fd < 0) return;

    // Handle modifier changes
    uint8_t mod_hid_codes[] = {0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7};
    for (int i = 0; i < 8; i++) {
        uint8_t old_bit = (prev_mods >> i) & 1;
        uint8_t new_bit = (report->mods >> i) & 1;
        if (old_bit != new_bit) {
            uint16_t evcode = hid_to_evdev[mod_hid_codes[i]];
            if (evcode) uinput_emit(EV_KEY, evcode, new_bit);
        }
    }

    // Handle key changes — diff old and new report
    // Release keys no longer in report
    for (int i = 0; i < KEYBOARD_REPORT_KEYS; i++) {
        uint8_t old_key = prev_keys[i];
        if (old_key == 0) continue;
        int found = 0;
        for (int j = 0; j < KEYBOARD_REPORT_KEYS; j++) {
            if (report->keys[j] == old_key) { found = 1; break; }
        }
        if (!found) {
            uint16_t evcode = hid_to_evdev[old_key];
            if (evcode) uinput_emit(EV_KEY, evcode, 0);
        }
    }
    // Press keys newly in report
    for (int i = 0; i < KEYBOARD_REPORT_KEYS; i++) {
        uint8_t new_key = report->keys[i];
        if (new_key == 0) continue;
        int found = 0;
        for (int j = 0; j < KEYBOARD_REPORT_KEYS; j++) {
            if (prev_keys[j] == new_key) { found = 1; break; }
        }
        if (!found) {
            uint16_t evcode = hid_to_evdev[new_key];
            if (evcode) uinput_emit(EV_KEY, evcode, 1);
        }
    }

    uinput_sync();
    memcpy(prev_keys, report->keys, sizeof(prev_keys));
    prev_mods = report->mods;
}

static void linux_send_nkro(report_nkro_t *report) {
    // NKRO: iterate all bits
    // For now, fall through to basic keyboard handling
    (void)report;
}

static void linux_send_mouse(report_mouse_t *report) {
    if (uinput_fd < 0) return;

    if (report->x) uinput_emit(EV_REL, REL_X, report->x);
    if (report->y) uinput_emit(EV_REL, REL_Y, report->y);
    if (report->v) uinput_emit(EV_REL, REL_WHEEL, report->v);
    if (report->h) uinput_emit(EV_REL, REL_HWHEEL, report->h);

    // Mouse buttons
    static uint8_t prev_buttons = 0;
    uint8_t        changed      = report->buttons ^ prev_buttons;
    if (changed & 1) uinput_emit(EV_KEY, BTN_LEFT, (report->buttons >> 0) & 1);
    if (changed & 2) uinput_emit(EV_KEY, BTN_RIGHT, (report->buttons >> 1) & 1);
    if (changed & 4) uinput_emit(EV_KEY, BTN_MIDDLE, (report->buttons >> 2) & 1);

    if (report->x || report->y || report->v || report->h || changed)
        uinput_sync();
    prev_buttons = report->buttons;
}

static void linux_send_extra(report_extra_t *report) {
    if (uinput_fd < 0) return;
    // Consumer control / system control
    // Map common usages
    if (report->report_id == 2) { // Consumer
        // Simplified: emit KEY_* for common media keys
        // Full mapping would be extensive
    }
    (void)report;
}

#ifdef RAW_ENABLE
static void linux_send_raw_hid(uint8_t *data, uint8_t length) {
    uhid_send_input(data, length);
}
#endif

static host_driver_t linux_driver = {
    .keyboard_leds = linux_keyboard_leds,
    .send_keyboard = linux_send_keyboard,
    .send_nkro     = linux_send_nkro,
    .send_mouse    = linux_send_mouse,
    .send_extra    = linux_send_extra,
#ifdef RAW_ENABLE
    .send_raw_hid  = linux_send_raw_hid,
#endif
};

// ---- Evdev setup ----

static char *evdev_device = NULL;

static int grab_one_evdev(const char *path) {
    int fd = open(path, O_RDONLY | O_NONBLOCK);
    if (fd < 0) return -1;

    char name[256] = "Unknown";
    ioctl(fd, EVIOCGNAME(sizeof(name)), name);

    // Skip our own uinput device
    if (strstr(name, "QAP'LA")) { close(fd); return -1; }

    if (ioctl(fd, EVIOCGRAB, 1) < 0) {
        fprintf(stderr, "qapla: warning: could not grab %s: %s\n", path, strerror(errno));
    }
    fprintf(stderr, "qapla: grabbed input device: %s (%s)\n", name, path);
    return fd;
}

static bool evdev_is_keyboard(int fd) {
    unsigned long evbits[2] = {0};
    ioctl(fd, EVIOCGBIT(0, sizeof(evbits)), evbits);
    if (!(evbits[0] & (1 << EV_KEY))) return false;

    unsigned long keybits[8] = {0};
    ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keybits)), keybits);
    // Check for KEY_A (30)
    return (keybits[30 / (sizeof(long) * 8)] & (1UL << (30 % (sizeof(long) * 8)))) != 0;
}

static void open_evdev(void) {
    evdev_count = 0;
    const char *env_path = evdev_device;
    if (!env_path) env_path = getenv("QAPLA_EVDEV");

    // Explicit device(s): QAPLA_EVDEV can be comma-separated
    // e.g. QAPLA_EVDEV=/dev/input/event1,/dev/input/event8
    if (env_path) {
        char buf[512];
        snprintf(buf, sizeof(buf), "%s", env_path);
        char *tok = strtok(buf, ",");
        while (tok && evdev_count < EVDEV_MAX_DEVICES) {
            // Trim whitespace
            while (*tok == ' ') tok++;
            int fd = grab_one_evdev(tok);
            if (fd >= 0) {
                evdev_fds[evdev_count++] = fd;
            }
            tok = strtok(NULL, ",");
        }
        if (evdev_count == 0) {
            fprintf(stderr, "qapla: could not open any device from QAPLA_EVDEV=%s\n", env_path);
        }
        return;
    }

    // Auto-detect: grab ALL keyboards with KEY_A capability
    // Some keyboards (e.g. AMIRA) expose multiple evdev interfaces
    // where keystrokes may arrive on any of them.
    fprintf(stderr, "qapla: scanning for keyboard devices...\n");
    for (int i = 0; i < 32 && evdev_count < EVDEV_MAX_DEVICES; i++) {
        char dev_path[64];
        snprintf(dev_path, sizeof(dev_path), "/dev/input/event%d", i);
        int fd = open(dev_path, O_RDONLY | O_NONBLOCK);
        if (fd < 0) continue;

        if (!evdev_is_keyboard(fd)) { close(fd); continue; }

        char name[256] = "Unknown";
        ioctl(fd, EVIOCGNAME(sizeof(name)), name);

        // Skip our own uinput device
        if (strstr(name, "QAP'LA")) { close(fd); continue; }

        if (ioctl(fd, EVIOCGRAB, 1) < 0) {
            fprintf(stderr, "qapla: warning: could not grab %s\n", dev_path);
            close(fd);
            continue;
        }
        fprintf(stderr, "qapla: grabbed keyboard: %s (%s)\n", name, dev_path);
        evdev_fds[evdev_count++] = fd;
    }

    if (evdev_count == 0) {
        fprintf(stderr, "qapla: no keyboard device found. Set QAPLA_EVDEV=/dev/input/eventN\n");
    } else {
        fprintf(stderr, "qapla: grabbed %d keyboard device(s)\n", evdev_count);
    }
}

// ---- Uinput setup ----

static int open_uinput(void) {
    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
        fprintf(stderr, "qapla: could not open /dev/uinput: %s\n", strerror(errno));
        return -1;
    }

    // Enable event types
    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    ioctl(fd, UI_SET_EVBIT, EV_REL);
    ioctl(fd, UI_SET_EVBIT, EV_SYN);

    // Enable all standard keys
    for (int i = 1; i < 248; i++) {
        ioctl(fd, UI_SET_KEYBIT, i);
    }
    // Mouse buttons
    ioctl(fd, UI_SET_KEYBIT, BTN_LEFT);
    ioctl(fd, UI_SET_KEYBIT, BTN_RIGHT);
    ioctl(fd, UI_SET_KEYBIT, BTN_MIDDLE);

    // Relative axes for mouse
    ioctl(fd, UI_SET_RELBIT, REL_X);
    ioctl(fd, UI_SET_RELBIT, REL_Y);
    ioctl(fd, UI_SET_RELBIT, REL_WHEEL);
    ioctl(fd, UI_SET_RELBIT, REL_HWHEEL);

    struct uinput_setup usetup = {0};
    snprintf(usetup.name, UINPUT_MAX_NAME_SIZE, "QAP'LA Virtual Keyboard");
    usetup.id.bustype = BUS_VIRTUAL;
    usetup.id.vendor  = 0x1209; // pid.codes test VID
    usetup.id.product = 0x4B42; // "KB"
    usetup.id.version = 1;

    ioctl(fd, UI_DEV_SETUP, &usetup);
    ioctl(fd, UI_DEV_CREATE);

    // Small delay for uinput device to register
    usleep(100000);

    fprintf(stderr, "qapla: virtual keyboard created\n");
    return fd;
}

// ---- Protocol interface ----

void protocol_setup(void) {
    timer_init();
}

void protocol_pre_init(void) {
    uinput_fd = open_uinput();
    open_evdev();
    if (evdev_count == 0) {
        fprintf(stderr, "qapla: running without input device (output-only mode)\n");
    }
#ifdef RAW_ENABLE
    uhid_fd = open_uhid();
#endif
}

void protocol_post_init(void) {
    host_set_driver(&linux_driver);
    fprintf(stderr, "qapla: QMK initialized, processing keys\n");
}

void protocol_pre_task(void) {
    if (!qapla_running()) {
        // Clean up and exit
        for (int i = 0; i < evdev_count; i++) {
            ioctl(evdev_fds[i], EVIOCGRAB, 0); // ungrab
            close(evdev_fds[i]);
        }
        if (uinput_fd >= 0) {
            ioctl(uinput_fd, UI_DEV_DESTROY);
            close(uinput_fd);
        }
        if (uhid_fd >= 0) {
            struct uhid_event ev = {0};
            ev.type = UHID_DESTROY;
            write(uhid_fd, &ev, sizeof(ev));
            close(uhid_fd);
        }
        fprintf(stderr, "qapla: shutting down\n");
        exit(0);
    }

    // Poll uhid for Vial commands
#ifdef RAW_ENABLE
    uhid_poll();
#endif

    // Read all pending evdev events from all grabbed devices
    for (int d = 0; d < evdev_count; d++) {
        struct input_event ev;
        while (read(evdev_fds[d], &ev, sizeof(ev)) == sizeof(ev)) {
            if (ev.type == EV_KEY && ev.code < EVDEV_KEY_MAX) {
                // value: 0=release, 1=press, 2=repeat (treat repeat as held)
                evdev_key_state[ev.code] = (ev.value > 0) ? 1 : 0;
            }
        }
    }
}

void protocol_post_task(void) {
    // Yield CPU time — no need to burn 100% spinning
    usleep(1000); // 1ms = ~1000Hz scan rate, plenty fast
}

#ifdef RAW_ENABLE
// raw_hid_task is called from main loop — we handle it in protocol_pre_task instead
void raw_hid_task(void) {}
#endif

// ---- Public API for setting evdev device path ----

void qapla_set_evdev_device(const char *path) {
    evdev_device = (char *)path;
}
