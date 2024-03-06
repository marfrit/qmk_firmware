/*
Copyright 2023 Markus Fritsche <fritsche.markus@gmail.com>

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

#include <stdint.h>
#include "gpio.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "pio_atxt_dma.h"
#include "debug.h"

#define PS2_KEEB_TIMEOUT_NO_DATA 20 /* http://www-ug.eecg.utoronto.ca/desl/nios_devices_SoC/datasheets/PS2%20Protocol.htm */
#define PS2_KEEB_TIMEOUT_SEND 25    /* grant a little more time for replies for good measure */

#if !defined(MCU_RP)
#    error PIO Driver is only available for Raspberry Pi 2040 MCUs!
#endif

#ifdef DEBUG_FRAMES
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  ((byte) & 0x80 ? '1' : '0'), \
  ((byte) & 0x40 ? '1' : '0'), \
  ((byte) & 0x20 ? '1' : '0'), \
  ((byte) & 0x10 ? '1' : '0'), \
  ((byte) & 0x08 ? '1' : '0'), \
  ((byte) & 0x04 ? '1' : '0'), \
  ((byte) & 0x02 ? '1' : '0'), \
  ((byte) & 0x01 ? '1' : '0')
#endif

#if PS2_KEEB_DATA_PIN + 1 != PS2_KEEB_CLOCK_PIN
#    error PS/2 clock pin must be data pin + 1!
#endif

static inline void pio_serve_keeb_dma_interrupt(void);

uint8_t ps2_keeb_error = PS2_ERR_NONE;
uint dma_chan = 0;

static const PIO pio = pio1; /* Use PIO1 since PIO0 is busy with the PS/2 mouse */

OSAL_IRQ_HANDLER(RP_PIO1_IRQ_1_HANDLER) {
    OSAL_IRQ_PROLOGUE();
    pio_serve_keeb_dma_interrupt();
    OSAL_IRQ_EPILOGUE();
}

#define PS2_WRAP_TARGET 0
#define PS2_WRAP 22

// clang-format off
static const uint16_t ps2_program_instructions[] = {
            //     .wrap_target
    0x00c7, //  0: jmp    pin, 7
    0xe02a, //  1: set    x, 10
    0x2021, //  2: wait   0 pin, 1
    0x4001, //  3: in     pins, 1
    0x20a1, //  4: wait   1 pin, 1
    0x0042, //  5: jmp    x--, 2
    0x0000, //  6: jmp    0
    0x00e9, //  7: jmp    !osre, 9
    0x0000, //  8: jmp    0
    0xff81, //  9: set    pindirs, 1             [31]
    0xe280, // 10: set    pindirs, 0             [2]
    0xe082, // 11: set    pindirs, 2
    0x2021, // 12: wait   0 pin, 1
    0xe029, // 13: set    x, 9
    0x6081, // 14: out    pindirs, 1
    0x20a1, // 15: wait   1 pin, 1
    0x2021, // 16: wait   0 pin, 1
    0x004e, // 17: jmp    x--, 14
    0xe083, // 18: set    pindirs, 3
    0x2021, // 19: wait   0 pin, 1
    0x20a1, // 20: wait   1 pin, 1
    0xe041, // 21: set    y, 1          // Additional to the PS/2 code by gamelaster, tag direct replies (e.g. ACK, 0xFA)
    0xa0d2, // 22: mov    isr, ::y      // reverse y and put into the ISR in order to tag the msg received
            //     .wrap
};
// clang-format on

static const struct pio_program ps2_program = {
    .instructions = ps2_program_instructions,
    .length       = 23,
    .origin       = -1,
};

static int                state_machine = -1;

void pio_serve_keeb_dma_interrupt(void) {
}

void ps2_keeb_host_init(void) {
    uint pio_idx = pio_get_index(pio);

    hal_lld_peripheral_unreset(pio_idx == 0 ? RESETS_ALLREG_PIO0 : RESETS_ALLREG_PIO1);

    state_machine = pio_claim_unused_sm(pio, true);
    if (state_machine < 0) {
        dprintln("ERROR: Failed to acquire state machine for PS/2!");
        ps2_keeb_error = PS2_ERR_NODATA;
        return;
    }

    uint offset = pio_add_program(pio, &ps2_program);

    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset + PS2_WRAP_TARGET, offset + PS2_WRAP);

    // Set pindirs to input (output enable is inverted below)
    pio_sm_set_consecutive_pindirs(pio, state_machine, PS2_KEEB_DATA_PIN, 2, true);
    sm_config_set_clkdiv(&c, (float)clock_get_hz(clk_sys) / (200.0f * KHZ));
    sm_config_set_set_pins(&c, PS2_KEEB_DATA_PIN, 2);
    sm_config_set_out_pins(&c, PS2_KEEB_DATA_PIN, 1);
    sm_config_set_out_shift(&c, true, true, 10);
    sm_config_set_in_shift(&c, true, true, 11);
    sm_config_set_jmp_pin(&c, PS2_KEEB_CLOCK_PIN);
    sm_config_set_in_pins(&c, PS2_KEEB_DATA_PIN);

    // clang-format off
    iomode_t pin_mode = PAL_RP_PAD_IE |
                        PAL_RP_GPIO_OE |
                        // PAL_RP_PAD_DRIVE2 because being
                        // gentle to the level shifter.
                        PAL_RP_PAD_DRIVE2 |
                        // Invert output enable so that pindirs=1 means input
                        // and indirs=0 means output. This way, out pindirs
                        // works correctly with the open-drain PS/2 interface.
                        // Setting pindirs=1 effectively pulls the line high,
                        // due to the pull-up resistor, while pindirs=0 pulls
                        // the line low.
                        PAL_RP_IOCTRL_OEOVER_DRVINVPERI |
                        (pio_idx == 0 ? PAL_MODE_ALTERNATE_PIO0 : PAL_MODE_ALTERNATE_PIO1);
    // clang-format on

    palSetLineMode(PS2_KEEB_DATA_PIN, pin_mode);
    palSetLineMode(PS2_KEEB_CLOCK_PIN, pin_mode);

    dma_channel_configure();

    pio_set_irq1_source_enabled(pio, pis_sm0_rx_fifo_not_empty + state_machine, true);
    pio_sm_init(pio, state_machine, offset, &c);

    nvicEnableVector(RP_PIO1_IRQ_1_NUMBER, CORTEX_MAX_KERNEL_PRIORITY);

    pio_sm_set_enabled(pio, state_machine, true);
}
