// Copyright 2023 sekigon-gonnoc
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

void post_process_record_mouse(uint16_t keycode, keyrecord_t* record);
bool process_record_mouse(uint16_t keycode, keyrecord_t* record);
void set_mouse_gesture_threshold(uint16_t val);
bool pre_process_record_mouse(uint16_t keycode, keyrecord_t *record);