/* Copyright 2019
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

//#define ADC_PIN F6

//#define QMK_WAITING_TEST_BUSY_PIN F6
//#define QMK_WAITING_TEST_YIELD_PIN F7

#define EXTERNAL_EEPROM_SPI_SLAVE_SELECT_PIN F7
#define EXTERNAL_EEPROM_BYTE_COUNT 16384
#define EXTERNAL_EEPROM_PAGE_SIZE 64
#define EXTERNAL_EEPROM_ADDRESS_SIZE 2
#define EXTERNAL_EEPROM_WP_PIN B6
#define EXTERNAL_EEPROM_SPI_CLOC_DIVISOR 32

#define DEBUG_EEPROM_OUTPUT
