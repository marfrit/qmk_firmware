// Copyright 2023 sekigon-gonnoc
// SPDX-License-Identifier: GPL-2.0-or-later

#define EMBEDDED_CLI_IMPL
#include "embedded_cli.h"

#include "debug.h"
#include "dynamic_config.h"
#include "bootloader.h"
#include "pio_usb_ll.h"

extern void tusb_print_debug_buffer(void);

#define CLI_BUFFER_SIZE 1024
static CLI_UINT           cliBuffer[BYTES_TO_CLI_UINTS(CLI_BUFFER_SIZE)];
static EmbeddedCli       *cli             = NULL;
static bool               cli_initialized = false;
static EmbeddedCliConfig *config;
static volatile bool      isLoadMode = false;

void virtser_recv(uint8_t c) {
    if (isLoadMode) {
        cdc_config_load_cb(&c, 1);
    } else {
        embeddedCliReceiveChar(cli, c);
    }
}

static void writeChar(EmbeddedCli *_, char c) { printf("%c", c); }

static void onVersion(EmbeddedCli *cli, char *args, void *context) {
    printf("firmware version: %s\n", STR(GIT_DESCRIBE));
}

static void onConfig(EmbeddedCli *cli, char *args, void *context) {
    print_config();
}

static void onApp(EmbeddedCli *cli, char *args, void *context) { print_app(); }

static void onCompanion(EmbeddedCli *cli, char *args, void *context) {
    print_companion_app();
}

static void onDfu(EmbeddedCli *cli, char *args, void *context) {
    bootloader_jump();
}

static void onDebug(EmbeddedCli *cli, char *args, void *context) {
    debug_enable = !debug_enable;
}

static void onActivateApps(EmbeddedCli *cli, char *args, void *context) {
    int     argc     = embeddedCliGetTokenCount(args);
    uint8_t apps[32] = {0};
    for (int pos = 0; pos < argc; pos++) {
        const char *arg = embeddedCliGetToken(args, pos + 1);
        uint32_t    val = strtoul(arg, NULL, 16);
        apps[pos]       = val <= 0xff ? val : 0;
    }

    set_active_apps(apps, argc);
}

static void onBackup(EmbeddedCli *cli, char *args, void *context) {
    send_config_file();
}

static void onLoad(EmbeddedCli *cli, char *args, void *context) {
    int argc = embeddedCliGetTokenCount(args);
    if (argc == 1) {
        const char *arg = embeddedCliGetToken(args, 1);
        uint32_t    val = strtoul(arg, NULL, 0);
        pre_load_config_file(val);
        isLoadMode = true;
        load_config_file();
        isLoadMode = false;
        post_load_config_file();
    } else {
        printf("Error: No file size info");
    }
}

static void onPioUsbStatus(EmbeddedCli *cli, char *args, void *context) {
    pio_port_t const *pp = PIO_USB_PIO_PORT(0);
    printf("error rate: %ld / %ld = %f%%\n", pp->total_error_count,
           pp->total_transaction_count,
           (float)pp->total_error_count / pp->total_transaction_count * 100);
    printf(
        "fatal error rate: %ld / %ld = %f%%\n", pp->total_fatal_error_count,
        pp->total_transaction_count,
        (float)pp->total_fatal_error_count / pp->total_transaction_count * 100);
}

void cli_init(void) {
    config                     = embeddedCliDefaultConfig();
    config->cliBuffer          = cliBuffer;
    config->cliBufferSize      = CLI_BUFFER_SIZE;
    config->enableAutoComplete = false;
    config->historyBufferSize  = 64;
    config->maxBindingCount    = 16;
    cli                        = embeddedCliNew(config);
    cli->writeChar             = writeChar;
    embeddedCliAddBinding(
        cli, (CliCommandBinding){"version", "Show version", false, NULL,
                                 onVersion});
    embeddedCliAddBinding(
        cli, (CliCommandBinding){"app", "Read application list", false, NULL,
                                 onApp});
    embeddedCliAddBinding(
        cli, (CliCommandBinding){"companion", "Print companion script", false,
                                 NULL, onCompanion});
    embeddedCliAddBinding(cli, (CliCommandBinding){"config", "Read config",
                                                   false, NULL, onConfig});
    embeddedCliAddBinding(
        cli, (CliCommandBinding){"piousb", "Pio usb status", false, NULL,
                                 onPioUsbStatus});
    embeddedCliAddBinding(cli, (CliCommandBinding){"dfu", "jump to bootloader",
                                                   false, NULL, onDfu});
    embeddedCliAddBinding(
        cli, (CliCommandBinding){"debug", "toggle debug option", false, NULL,
                                 onDebug});
    embeddedCliAddBinding(cli, (CliCommandBinding){"activate", "Activate apps",
                                                   true, NULL, onActivateApps});
    embeddedCliAddBinding(cli, (CliCommandBinding){"backup", "Backup config",
                                                   true, NULL, onBackup});
    embeddedCliAddBinding(cli, (CliCommandBinding){"load", "Load config",
                                                   true, NULL, onLoad});
    cli_initialized = true;
}

void cli_exec(void) {
    if (cli_initialized) embeddedCliProcess(cli);
    
    tusb_print_debug_buffer();
}