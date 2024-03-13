#include "matrix.h"
#include "debug.h"
#include "quantum.h"
#include "ch.h"
/* #include "hal.h"

#define BUFFER_SIZE 32
static input_buffers_queue_t               pio_msg_queue;
static __attribute__((aligned(4))) uint8_t pio_msg_buffer[BQ_BUFFER_SIZE(BUFFER_SIZE, sizeof(uint32_t))];

static THD_WORKING_AREA(waLEDThread, 128);
static THD_FUNCTION(LEDThread, arg) {
    (void)arg;
    uint32_t* frame_buffer;
    chRegSetThreadName("LEDThread");
    while (true) {
        osalSysLock();
        frame_buffer = (uint32_t*)ibqGetEmptyBufferI(&pio_msg_queue);
        *frame_buffer = port_get_core_id();
        ibqPostFullBufferI(&pio_msg_queue, sizeof(uint32_t));
        osalSysUnlock();
        wait_ms(2000);
    }
}

 */

void matrix_init_custom(void) {
    // TODO: initialize hardware here
    debug_enable = true;
    debug_keyboard = true;
}

int j = 0;

/* void keyboard_pre_init_kb(void) {
    ibqObjectInit(&pio_msg_queue, false, pio_msg_buffer, sizeof(uint32_t), BUFFER_SIZE, NULL, NULL);
    chThdCreateStatic(waLEDThread, sizeof(waLEDThread), NORMALPRIO - 16, LEDThread, NULL);
    keyboard_pre_init_user();
}

 */

bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    bool             matrix_has_changed = false;
    static u_int32_t count              = 0;
 /*    uint32_t         frame              = 0; */

    count++;
    if (count == 625000) {
        count = 0;
        j++;
        chThdSleepMilliseconds(2000);
        dprintf("Main (C0) running on core %d\n", port_get_core_id());
        /*         ibqReadTimeout(&pio_msg_queue, (uint8_t*)&frame, sizeof(uint32_t), TIME_MS2I(500));
         */
    }

    return matrix_has_changed;
}
