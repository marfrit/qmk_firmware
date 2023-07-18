// Copyright 2023 sekigon-gonnoc
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

typedef union {
    uint32_t raw;
    struct {
        uint8_t keyboard_lang_us_or_jp : 1;
        uint8_t os_lang_us_or_jp : 1;
    };
} user_config_t;
extern user_config_t user_config;