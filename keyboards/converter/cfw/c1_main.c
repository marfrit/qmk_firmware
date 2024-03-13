#include "ch.h"
#include "hal.h"
#include "debug.h"

extern int j;

__attribute__((weak)) void c1_main(void) {
    chSysWaitSystemState(ch_sys_running);
    chInstanceObjectInit(&ch1, &ch_core1_cfg);
    chSysUnlock();

    while (true) {
        chThdSleepMilliseconds(1493);
        dprintf("Running on core %d %d\n", port_get_core_id(), j);
    }
}
