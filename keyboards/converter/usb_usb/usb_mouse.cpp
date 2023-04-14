// USB HID host
#include "Usb.h"
#include "usbhub.h"
#include "hid.h"
#include "hidboot.h"
#include "parser.h"
#include "debug.h"

extern "C" {
#include "quantum.h"
#include "pointing_device.h"
}

extern USB usb_host;
extern KBDReportParser kbd_parser1;
report_mouse_t prevReport;
uint16_t prev_time_stamp;
// uint16_t cpisetting;
// bool scrolling;

class MouseReportParser2 : public MouseReportParser {
public:
    report_mouse_t report;
    uint16_t time_stamp;
    virtual void Parse(HID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf);
};

void MouseReportParser2::Parse(HID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf) {
    struct MOUSEINFO *msg = (struct MOUSEINFO*) buf;
    time_stamp = timer_read();
    // report.buttons = msg->bmLeftButton | msg->bmRightButton << 1 | msg->bmMiddleButton << 2;
    report.buttons = msg->bmLeftButton | msg->bmRightButton << 1;
    // scrolling = msg->bmMiddleButton;
    if(!msg->bmMiddleButton) {
        // cpisetting = pointing_device_get_cpi();
        // pointing_device_set_cpi(pointing_device_get_cpi() >> 4);
        // dprintf("CPI: %d\n", cpisetting);
        report.x = msg->dX;
        report.y = msg->dY;
        report.h = 0;
        report.v = 0;
    } else {
        report.x = 0;
        report.y = 0;
        report.h = - msg->dX;
        report.v = - msg->dY;
        dprintf("%d %d\n", msg->dX, msg->dY);
    }

    // int i;
    // for (i = 0; i < len; i++)
    // {
    //     if (i > 0) printf(":");
    //     dprintf("%02X", buf[i]);
    // }
    // dprintf("\n");
}

HIDBoot<HID_PROTOCOL_KEYBOARD | HID_PROTOCOL_MOUSE> hidcomposite(&usb_host);
HIDBoot<HID_PROTOCOL_MOUSE> mouse1(&usb_host);
MouseReportParser2 mouse_parser1;

extern "C" {
    void pointing_device_init_kb(void) {
        pointing_device_init_user();
    }

    void keyboard_post_init_kb(void) {
        debug_enable   = true;
        debug_matrix   = true;
        debug_keyboard = true;
        debug_mouse    = true;

        hidcomposite.SetReportParser(0, &kbd_parser1);
        hidcomposite.SetReportParser(1, &mouse_parser1);
        mouse1.SetReportParser(0, &mouse_parser1);
    }

    report_mouse_t pointing_device_task_kb(report_mouse_t mouse_report) {
        report_mouse_t local_mouse_report;
        uint16_t now = millis();
        local_mouse_report = pointing_device_get_report();
        local_mouse_report.x = mouse_parser1.report.x;
        local_mouse_report.y = mouse_parser1.report.y;
        local_mouse_report.h = mouse_parser1.report.h;
        local_mouse_report.v = mouse_parser1.report.v;
        if(timer_elapsed(mouse_parser1.time_stamp)) {
            mouse_parser1.report.x = 0;
            mouse_parser1.report.y = 0;
            mouse_parser1.report.h = 0;
            mouse_parser1.report.v = 0;
        }
        local_mouse_report.buttons = mouse_parser1.report.buttons;
        pointing_device_set_report(local_mouse_report);
        usb_host.Task();
        return local_mouse_report;
    }
}
