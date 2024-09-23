#include <string.h>

#define ko_make_with_custom_action(trigger_mods_, trigger_key, replacement_key, custom_action_)  \
    ((const key_override_t){ \
        .trigger_mods      = (trigger_mods_), \
        .layers            = ~0, \
        .suppressed_mods   = (trigger_mods_), \
        .options           = ko_option_activation_trigger_down, \
        .negative_mod_mask = (uint8_t) ~(trigger_mods_), \
        .custom_action     = (custom_action_), \
        .context           = NULL, \
        .trigger           = (trigger_key), \
        .replacement       = (replacement_key), \
        .enabled           = NULL \
 })

bool ctrl_k(bool key_down, void *context) {
    if (key_down) {
        register_code16(KC_LSFT);
        tap_code16(KC_END);
        tap_code16(KC_BSPC);
    } else {
        unregister_code16(KC_LSFT);
    }

    return false;
}

bool ctrl_u(bool key_down, void *context) {
    if (key_down) {
        register_code16(KC_LSFT);
        tap_code16(KC_HOME);
        tap_code16(KC_BSPC);
    } else {
        unregister_code16(KC_LSFT);
    }

    return false;
}

// linux
const key_override_t ko_layer_rofi_override_1 = ko_make_basic(0, KC_MUTE, C(KC_TAB));

const key_override_t ko_layer_copyq_override_1 = ko_make_basic(MOD_MASK_CTRL, KC_M, KC_ENT);

const key_override_t ko_layer_gui_override_1 = ko_make_basic(MOD_MASK_CTRL, KC_B, KC_LEFT);
const key_override_t ko_layer_gui_override_2 = ko_make_basic(MOD_MASK_CTRL, LT(UT, KC_F), KC_RIGHT);
const key_override_t ko_layer_gui_override_3 = ko_make_basic(MOD_MASK_CTRL, KC_P, KC_UP);
const key_override_t ko_layer_gui_override_4 = ko_make_basic(MOD_MASK_CTRL, KC_N, KC_DOWN);
const key_override_t ko_layer_gui_override_5 = ko_make_basic(MOD_MASK_CTRL, KC_H, KC_BSPC);
const key_override_t ko_layer_gui_override_6 = ko_make_basic(MOD_MASK_CTRL, KC_W, C(KC_BSPC));
const key_override_t ko_layer_gui_override_7 = ko_make_basic(MOD_MASK_CTRL, KC_A, KC_HOME);
const key_override_t ko_layer_gui_override_8 = ko_make_basic(MOD_MASK_CTRL, KC_E, KC_END);
const key_override_t ko_layer_gui_override_9 = ko_make_basic(MOD_MASK_CTRL, KC_M, KC_ENTER);
const key_override_t ko_layer_gui_override_10 = ko_make_basic(MOD_MASK_CTRL, KC_D, KC_DELETE);
const key_override_t ko_layer_gui_override_11 = ko_make_with_custom_action(MOD_MASK_CTRL, KC_K, KC_NO, ctrl_k);
const key_override_t ko_layer_gui_override_12 = ko_make_with_custom_action(MOD_MASK_CTRL, KC_U, KC_NO, ctrl_u);
const key_override_t ko_layer_gui_override_13 = ko_make_basic(MOD_MASK_CTRL, KC_Y, S(KC_F10));
const key_override_t ko_layer_gui_override_14 = ko_make_basic(MOD_MASK_CTRL, KC_LBRC, KC_ESC);
const key_override_t ko_layer_gui_override_15 = ko_make_basic(MOD_MASK_CS, KC_A, S(KC_HOME));
const key_override_t ko_layer_gui_override_16 = ko_make_basic(MOD_MASK_CS, KC_E, S(KC_END));
const key_override_t ko_layer_gui_override_17 = ko_make_basic(MOD_MASK_CTRL, KC_I, KC_TAB);
const key_override_t ko_layer_gui_override_18 = ko_make_basic(MOD_MASK_CS, KC_I, S(KC_TAB));
const key_override_t ko_layer_gui_override_19 = ko_make_basic(0, G(KC_V), C(KC_V));

const key_override_t ko_layer_debug_override_1 = ko_make_basic(MOD_MASK_CTRL, KC_C, C(KC_A));

const key_override_t ko_layer_winterm_override_1 = ko_make_basic(0, G(KC_V), S(KC_INSERT));
const key_override_t ko_layer_winterm_override_2 = ko_make_basic(MOD_MASK_CTRL, LT(SM, KC_SCLN), G(KC_V));

const key_override_t ko_layer_wingui_override_1 = ko_make_basic(MOD_MASK_CTRL, LT(SM, KC_SCLN), G(KC_V));

const key_override_t *linux_rofi_overrides[] = {
    &ko_layer_rofi_override_1,
    NULL
};

const key_override_t *linux_copyq_overrides[] = {
    &ko_layer_copyq_override_1,
    &ko_layer_gui_override_1,
    &ko_layer_gui_override_2,
    &ko_layer_gui_override_3,
    &ko_layer_gui_override_4,
    &ko_layer_gui_override_5,
    &ko_layer_gui_override_6,
    &ko_layer_gui_override_7,
    &ko_layer_gui_override_8,
    &ko_layer_gui_override_9,
    &ko_layer_gui_override_10,
    &ko_layer_gui_override_11,
    &ko_layer_gui_override_12,
    &ko_layer_gui_override_13,
    &ko_layer_gui_override_14,
    &ko_layer_gui_override_15,
    &ko_layer_gui_override_16,
    &ko_layer_gui_override_17,
    &ko_layer_gui_override_18,
    &ko_layer_gui_override_19,
    NULL
};

const key_override_t *linux_gui_overrides[] = {
    &ko_layer_gui_override_1,
    &ko_layer_gui_override_2,
    &ko_layer_gui_override_3,
    &ko_layer_gui_override_4,
    &ko_layer_gui_override_5,
    &ko_layer_gui_override_6,
    &ko_layer_gui_override_7,
    &ko_layer_gui_override_8,
    &ko_layer_gui_override_9,
    &ko_layer_gui_override_10,
    &ko_layer_gui_override_11,
    &ko_layer_gui_override_12,
    &ko_layer_gui_override_13,
    &ko_layer_gui_override_14,
    &ko_layer_gui_override_15,
    &ko_layer_gui_override_16,
    &ko_layer_gui_override_17,
    &ko_layer_gui_override_18,
    &ko_layer_gui_override_19,
    NULL
};

const key_override_t *win_gui_overrides[] = {
    &ko_layer_wingui_override_1,
    &ko_layer_gui_override_1,
    &ko_layer_gui_override_2,
    &ko_layer_gui_override_3,
    &ko_layer_gui_override_4,
    &ko_layer_gui_override_5,
    &ko_layer_gui_override_6,
    &ko_layer_gui_override_7,
    &ko_layer_gui_override_8,
    &ko_layer_gui_override_9,
    &ko_layer_gui_override_10,
    &ko_layer_gui_override_11,
    &ko_layer_gui_override_12,
    &ko_layer_gui_override_13,
    &ko_layer_gui_override_14,
    &ko_layer_gui_override_15,
    &ko_layer_gui_override_16,
    &ko_layer_gui_override_17,
    &ko_layer_gui_override_18,
    &ko_layer_gui_override_19,
    NULL
};

const key_override_t *win_terminal_overrides[] = {
    &ko_layer_winterm_override_1,
    &ko_layer_winterm_override_2,
    NULL
};

const key_override_t *linux_debug_overrides[] = {
    &ko_layer_debug_override_1,
    NULL
};

const key_override_t **key_overrides = NULL;  // Default, no overrides

void set_key_overrides(const char *application_name) {
    if (strcmp(application_name, "Rofi") == 0) {
        key_overrides = linux_rofi_overrides;
    } else if (strcmp(application_name, "copyq") == 0) {
        key_overrides = linux_copyq_overrides;
    } else if (strcmp(application_name, "debug") == 0) {
        key_overrides = linux_debug_overrides;
    } else if (strcmp(application_name, "WindowsTerminal") == 0) {
        key_overrides = win_terminal_overrides;
    } else if (strcmp(application_name, "kitty") == 0) {
        key_overrides = NULL;
    } else if (os_name == OS_WINDOWS) {
        key_overrides = win_gui_overrides;
    } else {
        key_overrides = linux_gui_overrides;
    }
}
