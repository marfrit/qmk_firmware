// USB HID host
#include "Usb.h"
#include "usbhub.h"
#include "hid.h"
#include "hidboot.h"
#include "parser.h"

extern "C" {
    #include "quantum.h"
    #include "pointing_device.h"
}

extern USB usb_host;

class MouseReportParser2 : public HIDReportParser {
public:
    report_mouse_t report;
    uint16_t time_stamp;
    virtual void Parse(HID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf);
};

void MouseReportParser2::Parse(HID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf) {
    ::memcpy(&report, buf, sizeof(report_mouse_t));
    time_stamp = millis();

    dprintf("input %d: %02X %02X,%02X V%02X H%02X\n", hid->GetAddress(), report.buttons, report.x, report.y, report.v, report.h);
}

HIDBoot<HID_PROTOCOL_MOUSE> mouse1(&usb_host);
MouseReportParser2 mouse_parser1;

extern "C" {
    void pointing_device_init_kb(void) {
        mouse1.SetReportParser(0, (HIDReportParser*)&mouse_parser1);
        pointing_device_init_user();
        dprintf("Pointing device initialized\n");
    }

    report_mouse_t pointing_device_task_kb(report_mouse_t mouse_report) {
        report_mouse_t local_mouse_report;
        local_mouse_report = pointing_device_get_report();

        local_mouse_report.x = mouse_parser1.report.x;
        local_mouse_report.y = mouse_parser1.report.y;
        local_mouse_report.h = mouse_parser1.report.h;
        local_mouse_report.v = mouse_parser1.report.v;
        local_mouse_report.buttons = mouse_parser1.report.buttons;
        pointing_device_set_report(local_mouse_report);
        return local_mouse_report;
    }
}
