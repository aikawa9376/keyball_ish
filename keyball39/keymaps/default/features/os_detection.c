#include QMK_KEYBOARD_H

#include "lib/keyball/keyball.h"
#include "lib/naginata/naginata.h"
#include "os_detection.h"

os_variant_t os_name; // +1 for null terminator

uint16_t keyball_get_scroll_resolution(void) {
#ifdef POINTING_DEVICE_HIRES_SCROLL_ENABLE
    // macOS/iOS do not interpret QMK's HID Resolution Multiplier like Linux,
    // so sending the usual 120x high-resolution units is excessively fast.
    if (os_name != OS_MACOS) {
        return pointing_device_get_hires_scroll_resolution();
    }
#endif
    return 1;
}

bool process_detected_host_os_kb(os_variant_t detected_os) {
    if (!process_detected_host_os_user(detected_os)) {
        return false;
    }
    switch (detected_os) {
        case OS_MACOS:
        case OS_IOS:
            switchOS(NG_MAC);
            os_name = OS_MACOS;
            break;
        case OS_WINDOWS:
            switchOS(NG_WIN);
            os_name = OS_WINDOWS;
            break;
        case OS_LINUX:
            switchOS(NG_LINUX);
            os_name = OS_LINUX;
            break;
        case OS_UNSURE:
            switchOS(NG_LINUX);
            os_name = OS_UNSURE;
            break;
    }
    // Discard any fractional scroll state accumulated using the old host scale.
    keyball_set_scroll_div(keyball_get_scroll_div());

    return true;
}
