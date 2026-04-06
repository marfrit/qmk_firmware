// SPDX-License-Identifier: GPL-2.0-or-later
// No GPIO on Linux userspace — custom matrix handles input
#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef uint32_t pin_t;

#define gpio_set_pin_input(pin)
#define gpio_set_pin_input_high(pin)
#define gpio_set_pin_input_low(pin)
#define gpio_set_pin_output(pin)
#define gpio_set_pin_output_push_pull(pin)
#define gpio_set_pin_output_open_drain(pin)
#define gpio_write_pin_high(pin)
#define gpio_write_pin_low(pin)
#define gpio_write_pin(pin, level)
#define gpio_read_pin(pin) 0
