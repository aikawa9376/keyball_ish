#include QMK_KEYBOARD_H

#include "auto_click_layer.h"
#include "macro_keys.h"
#include "mouse_buttons.h"
#include "selection_mode.h"

static selection_mode_t current_selection_mode = SELECTION_MODE_NONE;

static bool is_keyboard_selection_navigation(uint16_t keycode) {
    uint8_t mods = get_mods();

    if (IS_MODIFIER_KEYCODE(keycode) || keycode == MC_ESC || keycode == MC_LSFT || keycode == MC_RSFT) {
        return true;
    }

    if (IS_QK_MOMENTARY(keycode) || IS_QK_ONE_SHOT_LAYER(keycode) || IS_QK_TOGGLE_LAYER(keycode)) {
        return true;
    }

    if (IS_QK_MODS(keycode)) {
        mods |= QK_MODS_GET_MODS(keycode);
        keycode = QK_MODS_GET_BASIC_KEYCODE(keycode);
    } else if (IS_QK_LAYER_TAP(keycode)) {
        keycode = QK_LAYER_TAP_GET_TAP_KEYCODE(keycode);
    }

    switch (keycode) {
        case KC_LEFT:
        case KC_RIGHT:
        case KC_UP:
        case KC_DOWN:
        case KC_HOME:
        case KC_END:
        case KC_PAGE_UP:
        case KC_PAGE_DOWN:
            return true;
    }

    if (mods & MOD_MASK_CTRL) {
        switch (keycode) {
            case KC_A:
            case KC_B:
            case KC_E:
            case KC_F:
            case KC_N:
            case KC_P:
                return true;
        }
    }

    return false;
}

selection_mode_t selection_mode_get(void) {
    return current_selection_mode;
}

bool selection_mode_is_active(void) {
    return current_selection_mode != SELECTION_MODE_NONE;
}

bool selection_mode_should_stop(uint16_t keycode) {
    switch (current_selection_mode) {
        case SELECTION_MODE_KEYBOARD:
            return !is_keyboard_selection_navigation(keycode);
        case SELECTION_MODE_MOUSE:
            return true;
        case SELECTION_MODE_NONE:
            return false;
    }

    return false;
}

void selection_mode_start(bool mouse_mode) {
    if (selection_mode_is_active()) {
        return;
    }

    if (mouse_mode) {
        click_mouse_button1();
        current_selection_mode = SELECTION_MODE_MOUSE;
        state                  = CLICKING;
    } else {
        register_weak_mods(MOD_BIT(KC_LSFT));
        current_selection_mode = SELECTION_MODE_KEYBOARD;
    }
}

void selection_mode_stop(void) {
    switch (current_selection_mode) {
        case SELECTION_MODE_KEYBOARD:
            unregister_weak_mods(MOD_BIT(KC_LSFT));
            break;
        case SELECTION_MODE_MOUSE:
            release_mouse_button1();
            if (state == CLICKING) {
                state = CLICKED;
            }
            break;
        case SELECTION_MODE_NONE:
            return;
    }

    current_selection_mode = SELECTION_MODE_NONE;
}

void selection_mode_toggle(bool mouse_mode) {
    if (selection_mode_is_active()) {
        selection_mode_stop();
    } else {
        selection_mode_start(mouse_mode);
    }
}
