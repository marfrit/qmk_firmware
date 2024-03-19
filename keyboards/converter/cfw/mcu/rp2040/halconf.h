// Copyright 2022 Stefan Kerkmann
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#define HAL_USE_I2C TRUE
#define HAL_USE_PWM TRUE
#define HAL_USE_ADC TRUE
#define RP_PAL_EVENT_CORE_AFFINITY 1
#define PAL_USE_CALLBACKS TRUE

#include_next <halconf.h>
