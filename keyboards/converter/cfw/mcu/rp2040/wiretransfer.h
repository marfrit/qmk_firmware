#pragma once

#include "gpio.h"

/* iomode_t pin_mode = PAL_RP_PAD_IE |
                    PAL_RP_GPIO_OE |
                    PAL_RP_PAD_SLEWFAST |
                    PAL_RP_PAD_DRIVE2; */

/*--------------------------------------------------------------------
 * static functions
 *------------------------------------------------------------------*/
/*
 * Clock
 */
static inline void clock_lo(void)
{
    gpio_set_pin_output(KEYBOARD_CLOCK_PIN);
    gpio_write_pin_low(KEYBOARD_CLOCK_PIN);
}

static inline void clock_hi(void)
{
    gpio_set_pin_output(KEYBOARD_CLOCK_PIN);
    gpio_write_pin_high(KEYBOARD_CLOCK_PIN);
}

static inline bool clock_in(void)
{
    gpio_set_pin_input(KEYBOARD_CLOCK_PIN);
    wait_us(1);
    return palReadLine(KEYBOARD_CLOCK_PIN);
}

/*
 * Data
 */
static inline void data_lo(void)
{
    gpio_set_pin_output(KEYBOARD_DATA_PIN);
    gpio_write_pin_low(KEYBOARD_DATA_PIN);
}

static inline void data_hi(void)
{
    /* input with pull up */
    gpio_set_pin_output(KEYBOARD_DATA_PIN);
    gpio_write_pin_high(KEYBOARD_DATA_PIN);
}

static inline bool data_in(void)
{
    gpio_set_pin_input(KEYBOARD_DATA_PIN);
    wait_us(1);
    return palReadLine(KEYBOARD_DATA_PIN);
}

static inline uint16_t wait_clock_lo(uint16_t us)
{
    while (clock_in()  && us) { asm(""); wait_us(1); us--; }
    return us;
}
static inline uint16_t wait_clock_hi(uint16_t us)
{
    while (!clock_in() && us) { asm(""); wait_us(1); us--; }
    return us;
}
static inline uint16_t wait_data_lo(uint16_t us)
{
    while (data_in() && us)  { asm(""); wait_us(1); us--; }
    return us;
}
static inline uint16_t wait_data_hi(uint16_t us)
{
    while (!data_in() && us)  { asm(""); wait_us(1); us--; }
    return us;
}

/* idle state that device can send */
static inline void idle(void)
{
    clock_hi();
    data_hi();
}

/* inhibit device to send(AT), soft reset(XT) */
static inline void inhibit(void)
{
    clock_lo();
    data_hi();
}

/* inhibit device to send(XT) */
static inline void inhibit_xt(void)
{
    clock_hi();
    data_lo();
}

/* reset for XT Type-1 keyboard: low pulse for 500ms */
/* #define IBMPC_RST_HIZ() do { \
    writePinLow(IBMPC_RST_PIN0); \
    setPinInput(IBMPC_RST_PIN0); \
    writePinLow(IBMPC_RST_PIN1); \
    setPinInput(IBMPC_RST_PIN1); \
} while (0)

#define IBMPC_RST_LO() do { \
    writePinLow(IBMPC_RST_PIN0); \
    setPinOutput(IBMPC_RST_PIN0); \
    writePinLow(IBMPC_RST_PIN1); \
    setPinOutput(IBMPC_RST_PIN1); \
} while (0)

#define IBMPC_INT_INIT()                                 \
        { palSetLineMode(KEYBOARD_CLOCK_PIN, PAL_MODE_INPUT); } \
        while (0)
#define IBMPC_INT_ON()                                                    \
        {                                                                   \
            palEnableLineEvent(KEYBOARD_CLOCK_PIN, PAL_EVENT_MODE_FALLING_EDGE); \
            palSetLineCallback(KEYBOARD_CLOCK_PIN, palCallback, NULL);           \
        }                                                                   \
        while (0)
#define IBMPC_INT_OFF()                       \
        { palDisableLineEvent(KEYBOARD_CLOCK_PIN); } \
        while (0)
 */
