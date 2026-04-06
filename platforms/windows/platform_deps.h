// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

// Windows headers define KEY_EVENT as 0x1 (wincon.h), which collides
// with QMK's keyevent_type_t enum in keyboard.h.
// Both happen to use value 1, so we undef the Windows macro and let
// QMK's enum definition take precedence.
#include <windows.h>
#undef KEY_EVENT
