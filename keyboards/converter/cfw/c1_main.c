#include "ch.h"
#include "hal.h"
#include "debug.h"
#include "quantum.h"
#include "protocoladaptor.h"

extern u_int16_t startup_timer;

__attribute__((weak)) void c1_main(void) {
    chSysWaitSystemState(ch_sys_running);
    chInstanceObjectInit(&ch1, &ch_core1_cfg);
    chSysUnlock();

    dprintf("Time since start: %d\n", timer_elapsed(startup_timer));

    while (true) {
        chThdSleepMilliseconds(1493);
        dprintf("Running on core %d\n", port_get_core_id());
        dprintf("Time since start: %d\n", timer_elapsed(startup_timer));
        dprintf("Clock pin: %s data pin: %s\n", clock_in()?"true":"false", data_in()?"true":"false");
        clock_in()?clock_lo():clock_hi();
    }
}
