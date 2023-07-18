// copyright 2023 sekigon-gonnoc
// spdx-license-identifier: gpl-2.0-or-later

#include <stdint.h>
#include <stdio.h>
#include "ch.h"
#include "c1.h"
#include "dynamic_config.h"
#include "dynamic_config_def.h"

#include "timer.h"

#include "hardware/flash.h"

static uint32_t config_write_offset = 0;
static uint16_t last_write_time     = 0;
static uint8_t  data_chunk[256];
static uint16_t chunk_offset;

extern void virtser_task(void);

static void FLASH_Unlock(void) {
    c1_before_flash_operation();
    chSysLock();
}

static void FLASH_Lock(void) {
    c1_after_flash_operation();
    chSysUnlock();
}

void cdc_config_load_cb(const uint8_t *buf, uint32_t cnt) {
    memcpy(&data_chunk[chunk_offset], buf, cnt);
    chunk_offset += cnt;
    last_write_time = timer_read();
    if (chunk_offset % 64 == 0) {
        printf("Receive 64 bytes\n");
    }

    if (chunk_offset == 256) {
        printf("Load %d bytes to 0x%08lx\n", chunk_offset, config_write_offset);
        FLASH_Unlock();
        flash_range_program(CONFIG_ADDR - 0x10000000 + config_write_offset, data_chunk, sizeof(data_chunk));
        FLASH_Lock();
        config_write_offset += chunk_offset;
        chunk_offset = 0;
    }
}

void pre_load_config_file(uint32_t length) {
    FLASH_Unlock();
    uint32_t erase_bytes = (length + 4095) / 4096 * 4096;
    flash_range_erase(CONFIG_ADDR - 0x10000000, erase_bytes);
    FLASH_Lock();
    config_write_offset = 0;
    printf("Erase %ld bytes\n", erase_bytes);
}

void load_config_file(void) {
    last_write_time = timer_read();
    while (1) {
        virtser_task();
        if (timer_elapsed(last_write_time) > 2000) {
            break;
        }
    }
}

void post_load_config_file(void) {
    if (chunk_offset != 0) {
        printf("Load %d bytes to 0x%08lx\n", chunk_offset, config_write_offset);
        FLASH_Unlock();
        flash_range_program(CONFIG_ADDR - 0x10000000 + config_write_offset, data_chunk, sizeof(data_chunk));
        FLASH_Lock();
        config_write_offset += chunk_offset;
        chunk_offset = 0;
    }
    printf("Load complete. %ld bytes\n", config_write_offset);
    dynamic_config_init();
}
