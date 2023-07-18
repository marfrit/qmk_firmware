// copyright 2023 sekigon-gonnoc
// spdx-license-identifier: gpl-2.0-or-later

#pragma once

#include "quantum.h"

void dynamic_leader_task(void);
bool process_record_dynamic_leader(uint16_t keycode, keyrecord_t *record);
void process_dynamic_config_leader_task(void);