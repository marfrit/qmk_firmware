/*
Copyright 2019,2020 Jun Wako <wakojun@gmail.com>
Copyright 2021, Markus Fritsche <fritsche.markus@gmail.com>

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

#include "wait.h"
#include "timer.h"
#include "ibmpc_usb.h"
#include "ringbuf.h"

/* buffer for data received from device */
#define RINGBUF_SIZE    16
static uint8_t rbuf[RINGBUF_SIZE];
static ringbuf_t rb = {};

/* internal state of receiving data */
static volatile uint16_t isr_state = 0x8000;
static uint8_t timer_start = 0;

#define WAITPS2(stat, us, err) do { \
    if (!wait_##stat(us)) { \
        ibmpc_error = err; \
        goto ERROR; \
    } \
} while (0)

volatile uint16_t ibmpc_isr_debug = 0;
volatile uint8_t ibmpc_protocol = IBMPC_PROTOCOL_NO;
volatile uint8_t ibmpc_error = IBMPC_ERR_NONE;

int16_t read_wait(uint16_t wait_ms)
{
    uint16_t start = timer_read();
    int16_t code;
    while ((code = ibmpc_host_recv()) == -1 && timer_elapsed(start) < wait_ms);
    return code;
}

void ibmpc_host_init(void)
{
    // initialize reset pin to HiZ
    PS2_RST_HIZ();
    inhibit();
    PS2_INT_INIT();
    PS2_INT_OFF();
    ringbuf_init(&rb, rbuf, RINGBUF_SIZE);
}

void ibmpc_host_enable(void)
{
    PS2_INT_ON();
    idle();
}

void ibmpc_host_disable(void)
{
    PS2_INT_OFF();
    inhibit();
}

int16_t ibmpc_host_send(uint8_t data)
{
    bool parity = true;
    ibmpc_error = IBMPC_ERR_NONE;
    uint8_t retry = 0;

    dprintf("w%02X ", data);

    // Not receiving data
    if (isr_state != 0x8000) dprintf("isr:%04X ", isr_state);
    while (isr_state != 0x8000) ;

    // Not clock Lo
    if (!clock_in()) dprintf("c:%u ", wait_clock_hi(1000));

    // Not data Lo
    if (!data_in()) dprintf("d:%u ", wait_data_hi(1000));

    PS2_INT_OFF();

RETRY:
    /* terminate a transmission if we have */
    inhibit();
    wait_us(200);    // [5]p.54

    /* 'Request to Send' and Start bit */
    data_lo();
    wait_us(200);
    clock_hi();     // [5]p.54 [clock low]>100us [5]p.50
    WAITPS2(clock_lo, 10000, 1);   // [5]p.53, -10ms [5]p.50

    /* Data bit[2-9] */
    for (uint8_t i = 0; i < 8; i++) {
        wait_us(15);
        if (data&(1<<i)) {
            parity = !parity;
            data_hi();
        } else {
            data_lo();
        }
        WAITPS2(clock_hi, 50, 2);
        WAITPS2(clock_lo, 50, 3);
    }

    /* Parity bit */
    wait_us(15);
    if (parity) { data_hi(); } else { data_lo(); }
    WAITPS2(clock_hi, 50, 4);
    WAITPS2(clock_lo, 50, 5);

    /* Stop bit */
    wait_us(15);
    data_hi();

    /* Ack */
    WAITPS2(data_lo, 300, 6);
    WAITPS2(data_hi, 300, 7);
    WAITPS2(clock_hi, 300, 8);

    // clear buffer to get response correctly
    ibmpc_host_isr_clear();

    idle();
    PS2_INT_ON();
    return ibmpc_host_recv_response();
ERROR:
    // Retry for Z-150 AT start bit error
    if (ibmpc_error == 1 && retry++ < 10) {
        ibmpc_error = IBMPC_ERR_NONE;
        dprintf("R ");
        goto RETRY;
    }

    ibmpc_error |= IBMPC_ERR_SEND;
    inhibit();
    wait_ms(2);
    idle();
    PS2_INT_ON();
    return -1;
}

/*
 * Receive data from keyboard
 */
int16_t ibmpc_host_recv(void)
{
    int16_t ret = -1;

    // Enable ISR if buffer was full
    if (ringbuf_is_full(&rb)) {
        ibmpc_host_isr_clear();
        PS2_INT_ON();
        idle();
    }

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        ret = ringbuf_get(&rb);
    }
    if (ret != -1) dprintf("r%02X ", ret&0xFF);
    return ret;
}

int16_t ibmpc_host_recv_response(void)
{
    // Command may take 25ms/20ms at most([5]p.46, [3]p.21)
    uint8_t retry = 25;
    int16_t data = -1;
    while (retry-- && (data = ibmpc_host_recv()) == -1) {
        wait_ms(1);
    }
    return data;
}

void ibmpc_host_isr_clear(void)
{
    ibmpc_isr_debug = 0;
    ibmpc_protocol = 0;
    ibmpc_error = 0;
    isr_state = 0x8000;
    ringbuf_reset(&rb);
}


// NOTE: With this ISR data line should be read within 5us after clock falling edge.
// Confirmed that ATmega32u4 can read data line in 2.5us from interrupt after
// ISR prologue pushs r18, r19, r20, r21, r24, r25 r30 and r31 with GCC 5.4.0
ISR(PS2_INT_VECT)
{
    uint8_t dbit;
    dbit = readPin(PS2_DATA_PIN);

    // Timeout check
    uint8_t t;
    // use only the least byte of millisecond timer
    asm("lds %0, %1" : "=r" (t) : "p" (&timer_count));
    //t = (uint8_t)timer_count;    // compiler uses four registers instead of one
    if (isr_state == 0x8000) {
        timer_start = t;
    } else {
        // This gives 2.0ms at least before timeout
        if ((uint8_t)(t - timer_start) >= 3) {
            ibmpc_isr_debug = isr_state;
            ibmpc_error = IBMPC_ERR_TIMEOUT;
            goto ERROR;

            // timeout error recovery - start receiving new data
            // it seems to work somehow but may not under unstable situation
            //timer_start = t;
            //isr_state = 0x8000;
        }
    }

    isr_state = isr_state>>1;
    if (dbit) isr_state |= 0x8000;

    // isr_state: state of receiving data from keyboard
    //
    // This should be initialized with 0x8000 before receiving data and
    // the MSB '*1' works as marker to discrimitate between protocols.
    // It stores sampled bit at MSB after right shift on each clock falling edge.
    //
    // XT protocol has two variants of signaling; XT_IBM and XT_Clone.
    // XT_IBM uses two start bits 0 and 1 while XT_Clone uses just start bit 1.
    // https://github.com/tmk/tmk_keyboard/wiki/IBM-PC-XT-Keyboard-Protocol
    //
    //      15 14 13 12   11 10  9  8    7  6  5  4    3  2  1  0
    //      -----------------------------------------------------
    //      *1  0  0  0    0  0  0  0 |  0  0  0  0    0  0  0  0     Initial state(0x8000)
    //
    //       x  x  x  x    x  x  x  x |  0  0  0  0    0  0  0  0     midway(0-7 bits received)
    //       x  x  x  x    x  x  x  x | *1  0  0  0    0  0  0  0     midway(8 bits received)
    //      b6 b5 b4 b3   b2 b1 b0  1 |  0 *1  0  0    0  0  0  0     XT_IBM-midway ^1
    //      b7 b6 b5 b4   b3 b2 b1 b0 |  0 *1  0  0    0  0  0  0     AT-midway ^1
    //      b7 b6 b5 b4   b3 b2 b1 b0 |  1 *1  0  0    0  0  0  0     XT_Clone-done ^3
    //      b6 b5 b4 b3   b2 b1 b0  1 |  1 *1  0  0    0  0  0  0     XT_IBM-error ^3
    //      pr b7 b6 b5   b4 b3 b2 b1 |  0  0 *1  0    0  0  0  0     AT-midway[b0=0]
    //      b7 b6 b5 b4   b3 b2 b1 b0 |  1  0 *1  0    0  0  0  0     XT_IBM-done ^2
    //      pr b7 b6 b5   b4 b3 b2 b1 |  1  0 *1  0    0  0  0  0     AT-midway[b0=1] ^2
    //      b7 b6 b5 b4   b3 b2 b1 b0 |  1  1 *1  0    0  0  0  0     XT_IBM-error-done
    //       x  x  x  x    x  x  x  x |  x  1  1  0    0  0  0  0     illegal
    //      st pr b7 b6   b5 b4 b3 b2 | b1 b0  0 *1    0  0  0  0     AT-done
    //       x  x  x  x    x  x  x  x |  x  x  1 *1    0  0  0  0     illegal
    //                                all other states than above     illegal
    //
    // ^1: AT and XT_IBM takes same state.
    // ^2: AT and XT_IBM takes same state in case that AT b0 is 1,
    // we have to check AT stop bit to discriminate between the two protocol.
    switch (isr_state & 0xFF) {
        case 0b00000000:
        case 0b10000000:
        case 0b01000000:    // ^1
        case 0b00100000:
            // midway
            goto NEXT;
            break;
        case 0b11000000:    // ^3
            {
                uint8_t us = 100;
                // wait for rising and falling edge of b7 of XT_IBM
                while (!readPin(PS2_CLOCK_PIN) && us) { wait_us(1); us--; }
                while ( readPin(PS2_CLOCK_PIN)  && us) { wait_us(1); us--; }

                if (us) {
                    // XT_IBM-error: read start(0) as 1
                    goto NEXT;
                } else {
                    // XT_Clone-done
                    ibmpc_isr_debug = isr_state;
                    isr_state = isr_state>>8;
                    ibmpc_protocol = IBMPC_PROTOCOL_XT_CLONE;
                    goto DONE;
                }
            }
            break;
        case 0b11100000:
            // XT_IBM-error-done
            ibmpc_isr_debug = isr_state;
            isr_state = isr_state>>8;
            ibmpc_protocol = IBMPC_PROTOCOL_XT_ERROR;
            goto DONE;
            break;
        case 0b10100000:    // ^2
            {
                uint8_t us = 100;
                // wait for rising and falling edge of AT stop bit to discriminate between XT and AT
                while (!readPin(PS2_CLOCK_PIN) && us) { wait_us(1); us--; }
                while ( readPin(PS2_CLOCK_PIN)  && us) { wait_us(1); us--; }

                if (us) {
                    // found stop bit: AT-midway - process the stop bit in next ISR
                    goto NEXT;
                } else {
                    // no stop bit: XT_IBM-done
                    ibmpc_isr_debug = isr_state;
                    isr_state = isr_state>>8;
                    ibmpc_protocol = IBMPC_PROTOCOL_XT_IBM;
                    goto DONE;
                }
             }
            break;
        case 0b00010000:
        case 0b10010000:
        case 0b01010000:
        case 0b11010000:
            // AT-done
            // TODO: parity check?
            ibmpc_isr_debug = isr_state;
            // stop bit check
            if (isr_state & 0x8000) {
                ibmpc_protocol = IBMPC_PROTOCOL_AT;
            } else {
                // Zenith Z-150 AT(beige/white lable) asserts stop bit as low
                // https://github.com/tmk/tmk_keyboard/wiki/IBM-PC-AT-Keyboard-Protocol#zenith-z-150-beige
                ibmpc_protocol = IBMPC_PROTOCOL_AT_Z150;
            }
            isr_state = isr_state>>6;
            goto DONE;
            break;
        case 0b01100000:
        case 0b00110000:
        case 0b10110000:
        case 0b01110000:
        case 0b11110000:
        default:            // xxxx_oooo(any 1 in low nibble)
            // Illegal
            ibmpc_isr_debug = isr_state;
            ibmpc_error = IBMPC_ERR_ILLEGAL;
            goto ERROR;
            break;
    }

DONE:
    // store data
    ringbuf_push(&rb, isr_state & 0xFF);
    if (ringbuf_is_full(&rb)) {
        // just became full
        // Disable ISR if buffer is full
        PS2_INT_OFF();
        // inhibit: clock_lo
        clock_lo();
    }
    if (ringbuf_is_empty(&rb)) {
        // buffer overflow
        ibmpc_error = IBMPC_ERR_FULL;
    }
ERROR:
    // clear for next data
    isr_state = 0x8000;
NEXT:
    return;
}

/* send LED state to keyboard */
void ibmpc_host_set_led(uint8_t led)
{
    if (0xFA == ibmpc_host_send(0xED)) {
        ibmpc_host_send(led);
    }
}

const uint8_t map_cs1[MATRIX_ROWS][MATRIX_COLS] = {
    { XXX , 0x1a, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, \
    0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x29, 0x33 }, /* 08-0F */
    { 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, \
    0x3c, 0x3d, 0x3e, 0x3f, 0x57, 0x76, 0x4b, 0x4c }, /* 18-1F */
    { 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53, 0x54, \
    0x55, 0x1b, 0x5f, 0x56, 0x61, 0x62, 0x63, 0x64 }, /* 28-2F */
    { 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6c, 0x2f, \
    0x77, 0x78, 0x4a, 0x0d, 0x0e, 0x0f, 0x10, 0x11 }, /* 38-3F */
    { 0x12, 0x13, 0x14, 0x15, 0x16, 0x2d, 0x31, 0x44, \
    0x45, 0x46, 0x30, 0x59, 0x5a, 0x5b, 0x47, 0x70 }, /* 48-4F */
    { 0x71, 0x72, 0x7d, 0x7e, 0x5e, 0x48, 0x60, 0x17, \
    0x18, XXX , 0x75, 0x74, 0x5d, XXX , XXX , XXX  }, /* 58-5F */
    { 0x58, 0x6d, 0x7b, 0x6f, 0x01, 0x02, 0x03, 0x04, \
    0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x73 }, /* 68-6F */
    { XXX , 0x2a, 0x41, XXX , 0x2b, 0x42, 0x0c, 0x2c, \
    0x43, XXX , 0x7a, XXX , 0x79, XXX,  0x5c, 0x2e }, /* 78-7F */
};

const uint8_t map_cs2[MATRIX_ROWS][MATRIX_COLS] = {
    { 0x48, 0x15, 0x13, 0x11, 0x0F, 0x0D, 0x0E, 0x18, \
    0x01, 0x16, 0x14, 0x12, 0x10, 0x33, 0x1B, 0x79 }, /* 08-0F */
    { 0x02, 0x77, 0x5F, XXX , 0x76, 0x34, 0x1C, 0x7A, \
    0x03, 0x75, 0x61, 0x4c, 0x4b, 0x35, 0x1D, 0x74 }, /* 18-1F */
    { 0x04, 0x63, 0x62, 0x4d, 0x36, 0x1F, 0x1E, 0x42, \
    0x05, 0x78, 0x64, 0x4e, 0x38, 0x37, 0x20, 0x2B }, /* 28-2F */
    { 0x06, 0x66, 0x65, 0x50, 0x4f, 0x39, 0x21, 0x41, \
    0x07, 0x2A, 0x67, 0x51, 0x3A, 0x22, 0x23, 0x7b }, /* 38-3F */
    { 0x08, 0x68, 0x52, 0x3B, 0x3c, 0x25, 0x24, 0x6f, \
    0x09, 0x69, 0x6a, 0x53, 0x54, 0x3d, 0x26, 0x58 }, /* 48-4F */
    { 0x0A, XXX , 0x55, 0x6d, 0x3E, 0x27, 0x43, 0x0B, \
    0x4a, 0x6C, 0x57, 0x3F, 0x5d, 0x28, 0x2C, 0x0C }, /* 58-5F */
    { 0x2e, 0x60, 0x73, 0x7f, XXX , XXX , 0x29, XXX , \
    0x56, 0x70, XXX , 0x59, 0x44, 0x5c, XXX , XXX  }, /* 68-6F */
    { 0x7d, 0x7e, 0x71, 0x5a, 0x5b, 0x45, 0x1A, 0x2d, \
    0x17, 0x47, 0x72, 0x30, 0x2f, 0x46, 0x31, 0x5e }, /* 78-7F */
};

const uint8_t map_cs3[MATRIX_ROWS][MATRIX_COLS] = {
    { XXX , 0x74, 0x5e, 0x48, 0x31, 0x19, 0x1a, 0x0d, \
    0x01, 0x75, 0x5d, 0x49, 0x32, 0x33, 0x1b, 0x0e }, /* 08-0F */
    { 0x02, 0x76, 0x5f, 0x60, 0x4a, 0x34, 0x1c, 0x0f, \
    0x03, 0x77, 0x61, 0x4c, 0x4b, 0x35, 0x1d, 0x10 }, /* 18-1F */
    { 0x04, 0x63, 0x62, 0x4d, 0x36, 0x1f, 0x1e, 0x11, \
    0x05, 0x78, 0x64, 0x4e, 0x38, 0x37, 0x20, 0x12 }, /* 28-2F */
    { 0x06, 0x66, 0x65, 0x50, 0x4f, 0x39, 0x21, 0x13, \
    0x07, 0x79, 0x67, 0x51, 0x3a, 0x22, 0x23, 0x14 }, /* 38-3F */
    { 0x08, 0x68, 0x52, 0x3b, 0x3c, 0x25, 0x24, 0x15, \
    0x09, 0x69, 0x6a, 0x53, 0x54, 0x3d, 0x26, 0x16 }, /* 48-4F */
    { 0x0a, 0x6b, 0x55, 0x56, 0x3e, 0x27, 0x17, 0x0b, \
    0x7a, 0x6c, 0x57, 0x3f, 0x40, 0x28, 0x18, 0x0c }, /* 58-5F */
    { 0x7b, 0x6d, 0x2b, 0x58, 0x41, 0x42, 0x29, 0x2a, \
    0x5c, 0x70, 0x6f, 0x59, 0x44, 0x43, 0x2b, 0x2c }, /* 68-6F */
    { 0x7d, 0x7e, 0x71, 0x5a, 0x5b, 0x45, 0x2d, 0x2e, \
    0x7f, 0x73, 0x72, 0x5c, 0x47, 0x46, 0x2f, 0x30 }, /* 78-7F */
};
