// SPDX-License-Identifier: GPL-2.0-or-later
// QAP'LA: macOS platform setup

#include "platform_deps.h"
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

static volatile int running = 1;

static void signal_handler(int sig) {
    (void)sig;
    running = 0;
}

int qapla_running(void) {
    return running;
}

void platform_setup(void) {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
}
