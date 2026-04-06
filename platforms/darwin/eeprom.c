// SPDX-License-Identifier: GPL-2.0-or-later
// QAP'LA: File-backed EEPROM emulation
// Persists keymap state across runs in ~/.config/qapla/eeprom.bin

#include "eeprom_driver.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdint.h>

static uint8_t buffer[EEPROM_SIZE];
static char    eeprom_path[256];

static void ensure_dir(const char *path) {
    char tmp[256];
    snprintf(tmp, sizeof(tmp), "%s", path);
    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            mkdir(tmp, 0755);
            *p = '/';
        }
    }
}

static void resolve_path(void) {
    if (eeprom_path[0]) return;
    const char *home = getenv("HOME");
    if (home) {
        snprintf(eeprom_path, sizeof(eeprom_path), "%s/.config/qapla/eeprom.bin", home);
    } else {
        snprintf(eeprom_path, sizeof(eeprom_path), "/tmp/qapla_eeprom.bin");
    }
}

static void load_eeprom(void) {
    resolve_path();
    FILE *f = fopen(eeprom_path, "rb");
    if (f) {
        fread(buffer, 1, sizeof(buffer), f);
        fclose(f);
    }
}

static void save_eeprom(void) {
    resolve_path();
    ensure_dir(eeprom_path);
    FILE *f = fopen(eeprom_path, "wb");
    if (f) {
        fwrite(buffer, 1, sizeof(buffer), f);
        fclose(f);
    }
}

void eeprom_driver_init(void) {
    memset(buffer, 0xFF, sizeof(buffer));
    load_eeprom();
}

void eeprom_driver_erase(void) {
    memset(buffer, 0xFF, sizeof(buffer));
    save_eeprom();
}

void eeprom_read_block(void *buf, const void *addr, size_t len) {
    uintptr_t offset = (uintptr_t)addr;
    if (offset + len > sizeof(buffer)) {
        memset(buf, 0xFF, len);
        return;
    }
    memcpy(buf, &buffer[offset], len);
}

void eeprom_write_block(const void *buf, void *addr, size_t len) {
    uintptr_t offset = (uintptr_t)addr;
    if (offset + len > sizeof(buffer)) return;
    memcpy(&buffer[offset], buf, len);
    save_eeprom();
}
