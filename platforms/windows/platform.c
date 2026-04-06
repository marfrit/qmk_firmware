// SPDX-License-Identifier: GPL-2.0-or-later
// QAP'LA: Windows platform setup

#include "platform_deps.h"
#include <windows.h>
#include <stdio.h>

static volatile int running = 1;

static BOOL WINAPI console_handler(DWORD event) {
    if (event == CTRL_C_EVENT || event == CTRL_CLOSE_EVENT) {
        running = 0;
        return TRUE;
    }
    return FALSE;
}

int qapla_running(void) {
    return running;
}

void platform_setup(void) {
    SetConsoleCtrlHandler(console_handler, TRUE);
}
