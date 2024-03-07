/*
Copyright 2024 Markus Fritsche <fritsche.markus@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#define POWERPIN GP16              /* GPIO pin for turning on PS/2 device power on Purdea Andrei's converter */
#define PS2_KEEB_CLOCK_PIN GP21    /* GPIO that is wired with the keyboard's clock signal */
#define PS2_KEEB_DATA_PIN GP20     /* GPIO that is wired with the keyboard's data signal */
/* Receive mouse reports only after polling. This corresponds well with QMK's ps2_mouse_task.
   In stream mode, the mouse would send movement data to a queue which would eventually be
   handled by the host or discarded. */
