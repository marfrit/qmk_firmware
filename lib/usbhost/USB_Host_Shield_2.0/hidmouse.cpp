#include "hidmouse.h"
#include "print.h"

void MouseReportParser::Parse(HID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf) {
    ::memcpy(&report, buf, sizeof(report_mouse_t));
    time_stamp = millis();

    dprintf("input %d: %02X %02X,%02X V%02X H%02X\n", hid->GetAddress(), report.buttons, report.x, report.y, report.v, report.h);
}