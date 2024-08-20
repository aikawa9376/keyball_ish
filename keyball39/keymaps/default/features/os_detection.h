bool process_detected_host_os_kb(os_variant_t detected_os) {
    if (!process_detected_host_os_user(detected_os)) {
        return false;
    }
    switch (detected_os) {
        case OS_MACOS:
        case OS_IOS:
            switchOS(NG_MAC);
            break;
        case OS_WINDOWS:
            switchOS(NG_WIN);
            break;
        case OS_LINUX:
            switchOS(NG_LINUX);
            break;
        case OS_UNSURE:
            switchOS(NG_LINUX);
            break;
    }

    return true;
}
