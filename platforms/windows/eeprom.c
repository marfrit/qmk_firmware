// SPDX-License-Identifier: GPL-2.0-or-later
// QAP'LA: File-backed EEPROM for Windows
// Stores in %APPDATA%\qapla\eeprom.bin

#include "eeprom_driver.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

static uint8_t buffer[EEPROM_SIZE];
static char eeprom_path[MAX_PATH];

static void resolve_path(void) {
    if (eeprom_path[0]) return;
    const char *appdata = getenv("APPDATA");
    if (appdata) {
        snprintf(eeprom_path, sizeof(eeprom_path), "%s\\qapla\\eeprom.bin", appdata);
        // Create directory
        char dir[MAX_PATH];
        snprintf(dir, sizeof(dir), "%s\\qapla", appdata);
        CreateDirectoryA(dir, NULL);
    } else {
        snprintf(eeprom_path, sizeof(eeprom_path), "C:\\qapla_eeprom.bin");
    }
}

static void load_eeprom(void) {
    resolve_path();
    FILE *f = fopen(eeprom_path, "rb");
    if (f) { fread(buffer, 1, sizeof(buffer), f); fclose(f); }
}

static void save_eeprom(void) {
    resolve_path();
    FILE *f = fopen(eeprom_path, "wb");
    if (f) { fwrite(buffer, 1, sizeof(buffer), f); fclose(f); }
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
    if (offset + len > sizeof(buffer)) { memset(buf, 0xFF, len); return; }
    memcpy(buf, &buffer[offset], len);
}

void eeprom_write_block(const void *buf, void *addr, size_t len) {
    uintptr_t offset = (uintptr_t)addr;
    if (offset + len > sizeof(buffer)) return;
    memcpy(&buffer[offset], buf, len);
    save_eeprom();
}
