#include QMK_KEYBOARD_H
#include "mouse_buttons.h"

void update_mouse_button1(bool pressed) {
    report_mouse_t current_report = pointing_device_get_report();

    if (pressed) {
        current_report.buttons |= MOUSE_BTN1;
    } else {
        current_report.buttons &= ~MOUSE_BTN1;
    }

    pointing_device_set_report(current_report);
    pointing_device_send();
}

void click_mouse_button1(void) {
    update_mouse_button1(true);
}

void release_mouse_button1(void) {
    update_mouse_button1(false);
}
