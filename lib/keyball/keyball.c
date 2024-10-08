/*
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include QMK_KEYBOARD_H

#ifdef SPLIT_KEYBOARD
#    include "transactions.h"
#endif

#include "lib/keyball/keyball.h"

const uint16_t CPI_DEFAULT   = KEYBALL_CPI_DEFAULT;
const uint16_t CPI_MAX       = 12000;
const uint8_t SCROLL_DIV_MAX = 7;

static const char BL = '\xB0'; // Blank indicator character
static const char LFSTR_ON[] PROGMEM = "\xB2\xB3";
static const char LFSTR_OFF[] PROGMEM = "\xB4\xB5";

uint16_t horizontal_flag = 0;

keyball_t keyball = {
    .this_have_ball = false,
    .that_enable    = false,
    .that_have_ball = false,

    .cpi_value   = 0,
    .cpi_changed = false,

    .scroll_mode = false,
    .scroll_div  = 0,

    .pressing_keys = { BL, BL, BL, BL, BL, BL, 0 },

    .scroll_buffer = {0},
};

//////////////////////////////////////////////////////////////////////////////
// Hook points

__attribute__((weak)) void keyball_on_adjust_layout(keyball_adjust_t v) {}

//////////////////////////////////////////////////////////////////////////////
// Static utilities

// clip2int8 clips an integer fit into int8_t.
static inline int8_t clip2int8(int16_t v) {
    return (v) < -127 ? -127 : (v) > 127 ? 127 : (int8_t)v;
}

#ifdef OLED_ENABLE
static const char *format_4d(int8_t d) {
    static char buf[5] = {0}; // max width (4) + NUL (1)
    char        lead   = ' ';
    if (d < 0) {
        d    = -d;
        lead = '-';
    }
    buf[3] = (d % 10) + '0';
    d /= 10;
    if (d == 0) {
        buf[2] = lead;
        lead   = ' ';
    } else {
        buf[2] = (d % 10) + '0';
        d /= 10;
    }
    if (d == 0) {
        buf[1] = lead;
        lead   = ' ';
    } else {
        buf[1] = (d % 10) + '0';
        d /= 10;
    }
    buf[0] = lead;
    return buf;
}

static char to_1x(uint8_t x) {
    x &= 0x0f;
    return x < 10 ? x + '0' : x + 'a' - 10;
}
#endif

static void add_cpi(int8_t delta) {
    int16_t v = keyball_get_cpi() + delta;
    keyball_set_cpi(v < 1 ? 1 : v);
}

static void add_scroll_div(int8_t delta) {
    int8_t v = keyball_get_scroll_div() + delta;
    keyball_set_scroll_div(v < 1 ? 1 : v);
}

//////////////////////////////////////////////////////////////////////////////
// Pointing device driver

void pointing_device_init_kb(void) {
    keyball.this_have_ball = pmw33xx_init(0);

    if (keyball.this_have_ball) {
        pmw33xx_set_cpi(0, CPI_DEFAULT);
        keyball_set_scroll_div(KEYBALL_SCROLL_DIV_DEFAULT);

        pointing_device_init_user();
    }
}

static void adjust_mouse_speed (report_mouse_t *m) {
    int16_t movement_size = abs(m->x) + abs(m->y);
    float speed_multiplier = 1.0; // EDGE

    if (movement_size > 30){
        speed_multiplier = 3.0;
    } else if (movement_size > 15) {
        speed_multiplier = 2.5;
    } else if (movement_size > 8) {
        speed_multiplier = 2.0;
    } else if (movement_size > 5) {
        speed_multiplier = 1.5;
    } else if (movement_size > 3) {
        speed_multiplier = 0.7;
    } else if (movement_size > 1) {
        speed_multiplier = 0.5;
    }
    m->x = clip2int8((int16_t) (m->x * speed_multiplier));
    m->y = clip2int8((int16_t) (m->y * speed_multiplier));
}

static void motion_to_mouse_move(report_mouse_t *r, bool is_left) {
#if KEYBALL_MODEL == 61 || KEYBALL_MODEL == 39 || KEYBALL_MODEL == 147 || KEYBALL_MODEL == 44
    adjust_mouse_speed (r);

    r->x = clip2int8(r->x);
    r->y = clip2int8(r->y);
    if (is_left) {
        r->x = -r->x;
        r->y = -r->y;
    }
#else
    #    error("unknown Keyball model")
#endif
}

static void motion_to_mouse_scroll(report_mouse_t *r, bool is_left) {
    // consume motion of trackball.
    int16_t div = 1 << (keyball_get_scroll_div() - 1);

    keyball.scroll_buffer.h += (float)r->x / div;
    keyball.scroll_buffer.v -= (float)r->y / div;

    // apply to mouse report.
#if KEYBALL_MODEL == 61 || KEYBALL_MODEL == 39 || KEYBALL_MODEL == 147 || KEYBALL_MODEL == 44
    r->h = clip2int8(keyball.scroll_buffer.h);
    r->v = clip2int8(keyball.scroll_buffer.v);
    if (is_left) {
        r->h = -r->h;
        r->v = -r->v;
    }
    if (horizontal_flag == 1) {
        r->h = 0;
    } else if (horizontal_flag == 2) {
        r->v = 0;
    }

    keyball.scroll_buffer.h -= clip2int8(keyball.scroll_buffer.h);
    keyball.scroll_buffer.v -= clip2int8(keyball.scroll_buffer.v);

    r->x = 0;
    r->y = 0;
#else
    #    error("unknown Keyball model")
#endif
}

static void motion_to_mouse(report_mouse_t *r, bool is_left, bool as_scroll) {
    if (as_scroll) {
        motion_to_mouse_scroll(r, is_left);
    } else {
        motion_to_mouse_move(r, is_left);
    }
}

report_mouse_t pointing_device_task_kb(report_mouse_t rep) {
    // report mouse event, if keyboard is primary.
    if (is_keyboard_master()) {
        // modify mouse report by PMW3360 motion.
        motion_to_mouse(&rep, is_keyboard_left(), keyball.scroll_mode);
        // store mouse report for OLED.
        /* keyball.last_mouse = rep; */
    }
    return pointing_device_task_user(rep);
}

//////////////////////////////////////////////////////////////////////////////
// Split RPC

#ifdef SPLIT_KEYBOARD

static void rpc_get_info_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data) {
    keyball_info_t info = {
        .ballcnt = keyball.this_have_ball ? 1 : 0,
    };
    *(keyball_info_t *)out_data = info;
    keyball_on_adjust_layout(KEYBALL_ADJUST_SECONDARY);
}

static void rpc_get_info_invoke(void) {
    static bool     negotiated = false;
    static uint32_t last_sync  = 0;
    static int      round      = 0;
    uint32_t        now        = timer_read32();
    if (negotiated || TIMER_DIFF_32(now, last_sync) < KEYBALL_TX_GETINFO_INTERVAL) {
        return;
    }
    last_sync = now;
    round++;
    keyball_info_t recv = {0};
    if (!transaction_rpc_exec(KEYBALL_GET_INFO, 0, NULL, sizeof(recv), &recv)) {
        if (round < KEYBALL_TX_GETINFO_MAXTRY) {
            dprintf("keyball:rpc_get_info_invoke: missed #%d\n", round);
            return;
        }
    }
    negotiated             = true;
    keyball.that_enable    = true;
    keyball.that_have_ball = recv.ballcnt > 0;
    dprintf("keyball:rpc_get_info_invoke: negotiated #%d %d\n", round, keyball.that_have_ball);

    // split keyboard negotiation completed.

#    ifdef VIA_ENABLE
// adjust VIA layout options according to current combination.
uint8_t  layouts = (keyball.this_have_ball ? (is_keyboard_left() ? 0x02 : 0x01) : 0x00) | (keyball.that_have_ball ? (is_keyboard_left() ? 0x01 : 0x02) : 0x00);
uint32_t curr    = via_get_layout_options();
uint32_t next    = (curr & ~0x3) | layouts;
if (next != curr) {
    via_set_layout_options(next);
}
#    endif

    keyball_on_adjust_layout(KEYBALL_ADJUST_PRIMARY);
}

static void rpc_get_motion_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data) {
    report_mouse_t r = pointing_device_get_report();
    *(report_mouse_t *)out_data = r;
    // clear motion
    r.x = 0;
    r.y = 0;
    pointing_device_set_report(r);
}

static void rpc_set_cpi_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data) {
    keyball_set_cpi(*(keyball_cpi_t *)in_data);
}

static void rpc_set_cpi_invoke(void) {
    if (!keyball.cpi_changed) {
        return;
    }
    keyball_cpi_t req = keyball.cpi_value;
    if (!transaction_rpc_send(KEYBALL_SET_CPI, sizeof(req), &req)) {
        return;
    }
    keyball.cpi_changed = false;
}

#endif

// Public API functions

bool keyball_get_scroll_mode(void) {
    return keyball.scroll_mode;
}

void keyball_set_scroll_mode(bool mode) {
    if (mode != keyball.scroll_mode) {
        keyball.scroll_mode_changed = timer_read32();
    }
    keyball.scroll_mode = mode;
}

uint8_t keyball_get_scroll_div(void) {
    return keyball.scroll_div == 0 ? KEYBALL_SCROLL_DIV_DEFAULT : keyball.scroll_div;
}

void keyball_set_scroll_div(uint8_t div) {
    keyball.scroll_div = div > SCROLL_DIV_MAX ? SCROLL_DIV_MAX : div;
}

uint8_t keyball_get_cpi(void) {
    return keyball.cpi_value == 0 ? CPI_DEFAULT : keyball.cpi_value;
}

void keyball_set_cpi(uint16_t cpi) {
    if (cpi > CPI_MAX) {
        cpi = CPI_MAX;
    }
    keyball.cpi_value   = cpi;
    keyball.cpi_changed = true;
    if (keyball.this_have_ball) {
        pmw33xx_set_cpi(0, cpi == 0 ? CPI_DEFAULT : cpi);
    }
}

//////////////////////////////////////////////////////////////////////////////
// Keyboard hooks

void keyboard_post_init_kb(void) {
#ifdef SPLIT_KEYBOARD
// register transaction handlers on secondary.
if (!is_keyboard_master()) {
    transaction_register_rpc(KEYBALL_GET_INFO, rpc_get_info_handler);
    transaction_register_rpc(KEYBALL_GET_MOTION, rpc_get_motion_handler);
    transaction_register_rpc(KEYBALL_SET_CPI, rpc_set_cpi_handler);
}
#endif

    // read keyball configuration from EEPROM
    if (eeconfig_is_enabled()) {
        keyball_config_t c = {.raw = eeconfig_read_kb()};
        keyball_set_cpi(c.cpi);
        keyball_set_scroll_div(c.sdiv);
    }

    keyball_on_adjust_layout(KEYBALL_ADJUST_PENDING);
    keyboard_post_init_user();
}

#if SPLIT_KEYBOARD
void housekeeping_task_kb(void) {
    if (is_keyboard_master()) {
        rpc_get_info_invoke();
        if (keyball.that_have_ball) {
            rpc_set_cpi_invoke();
        }
    }
}
#endif

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    // store last keycode, row, and col for OLED
    keyball.last_kc  = keycode;
    keyball.last_pos = record->event.key;

    if (!process_record_user(keycode, record)) {
        return false;
    }

    // strip QK_MODS part.
    if (keycode >= QK_MODS && keycode <= QK_MODS_MAX) {
        keycode &= 0xff;
    }

    switch (keycode) {
#ifndef MOUSEKEY_ENABLE
// process KC_MS_BTN1~8 by myself
// See process_action() in quantum/action.c for details.
case KC_MS_BTN1 ... KC_MS_BTN8: {
    extern void register_mouse(uint8_t mouse_keycode, bool pressed);
    register_mouse(keycode, record->event.pressed);
    // to apply QK_MODS actions, allow to process others.
    return true;
}
#endif

        case SCRL_MO:
            keyball_set_scroll_mode(record->event.pressed);
            return false;
    }

    // process events which works on pressed only.
    if (record->event.pressed) {
        switch (keycode) {
            case KBC_RST:
                keyball_set_cpi(0);
                keyball_set_scroll_div(0);
                break;
            case KBC_SAVE: {
                keyball_config_t c = {
                    .cpi  = keyball.cpi_value,
                    .sdiv = keyball.scroll_div,
                };
                eeconfig_update_kb(c.raw);
            } break;

            case CPI_I100:
                add_cpi(1);
                break;
            case CPI_D100:
                add_cpi(-1);
                break;
            case CPI_I1K:
                add_cpi(10);
                break;
            case CPI_D1K:
                add_cpi(-10);
                break;

            case SCRL_TO:
                keyball_set_scroll_mode(!keyball.scroll_mode);
                break;
            case SCRL_DVI:
                add_scroll_div(1);
                break;
            case SCRL_DVD:
                add_scroll_div(-1);
                break;

            default:
                return true;
        }
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////
// OLED utility

#ifdef OLED_ENABLE
// clang-format off
const char PROGMEM code_to_name[] = {
    'a', 'b', 'c', 'd', 'e', 'f',  'g', 'h', 'i',  'j',
    'k', 'l', 'm', 'n', 'o', 'p',  'q', 'r', 's',  't',
    'u', 'v', 'w', 'x', 'y', 'z',  '1', '2', '3',  '4',
    '5', '6', '7', '8', '9', '0',  'R', 'E', 'B',  'T',
    '_', '-', '=', '[', ']', '\\', '#', ';', '\'', '`',
    ',', '.', '/',
};
// clang-format on
#endif

void keyball_oled_render_ballinfo(void) {
#ifdef OLED_ENABLE
// Format: `Ball:{mouse x}{mouse y}{mouse h}{mouse v}`
//
// Output example:
//
//     Ball: -12  34   0   0

// 1st line, "Ball" label, mouse x, y, h, and v.
oled_write_P(PSTR("Ball\xB1"), false);
oled_write(format_4d(keyball.last_mouse.x), false);
oled_write(format_4d(keyball.last_mouse.y), false);
oled_write(format_4d(keyball.last_mouse.h), false);
oled_write(format_4d(keyball.last_mouse.v), false);

// 2nd line, empty label and CPI
oled_write_P(PSTR("    \xB1\xBC\xBD"), false);
oled_write(format_4d(keyball_get_cpi()) + 1, false);
oled_write_P(PSTR("00 "), false);

// indicate scroll mode: on/off
oled_write_P(PSTR("\xBE\xBF"), false);
if (keyball.scroll_mode) {
    oled_write_P(LFSTR_ON, false);
} else {
    oled_write_P(LFSTR_OFF, false);
}

// indicate scroll divider:
oled_write_P(PSTR(" \xC0\xC1"), false);
oled_write_char('0' + keyball_get_scroll_div(), false);
#endif
}

void keyball_oled_render_ballsubinfo(void) {
#ifdef OLED_ENABLE
#endif
}

void keyball_oled_render_keyinfo(void) {
#ifdef OLED_ENABLE
// Format: `Key :  R{row}  C{col} K{kc} {name}{name}{name}`
//
// Where `kc` is lower 8 bit of keycode.
// Where `name`s are readable labels for pressing keys, valid between 4 and 56.
//
// `row`, `col`, and `kc` indicates the last processed key,
// but `name`s indicate unreleased keys in best effort.
//
// It is aligned to fit with output of keyball_oled_render_ballinfo().
// For example:
//
//     Key :  R2  C3 K06 abc
//     Ball:   0   0   0   0

// "Key" Label
oled_write_P(PSTR("Key \xB1"), false);

// Row and column
oled_write_char('\xB8', false);
oled_write_char(to_1x(keyball.last_pos.row), false);
oled_write_char('\xB9', false);
oled_write_char(to_1x(keyball.last_pos.col), false);

// Keycode
oled_write_P(PSTR("\xBA\xBB"), false);
oled_write_char(to_1x(keyball.last_kc >> 4), false);
oled_write_char(to_1x(keyball.last_kc), false);

// Pressing keys
oled_write_P(PSTR("  "), false);
oled_write(keyball.pressing_keys, false);
#endif
}

void keyball_oled_render_layerinfo(void) {
#ifdef OLED_ENABLE
// Format: `Layer:{layer state}`
//
// Output example:
//
//     Layer:-23------------
//
oled_write_P(PSTR("L\xB6\xB7r\xB1"), false);
for (uint8_t i = 1; i < 8; i++) {
    oled_write_char((layer_state_is(i) ? to_1x(i) : BL), false);
}
oled_write_char(' ', false);

#    ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
oled_write_P(PSTR("\xC2\xC3"), false);
if (get_auto_mouse_enable()) {
    oled_write_P(LFSTR_ON, false);
} else {
    oled_write_P(LFSTR_OFF, false);
}

oled_write(format_4d(get_auto_mouse_timeout() / 10) + 1, false);
oled_write_char('0', false);
#    else
oled_write_P(PSTR("\xC2\xC3\xB4\xB5 ---"), false);
#    endif
#endif
}

char keyball_get_oled_layer_char(uint8_t layer) {
    // layer が有効なレイヤーであることを確認する
    if (layer < DYNAMIC_KEYMAP_LAYER_COUNT) {
        return layer_state_is(layer) ? to_1x(layer) : BL;
    } else {
        return BL; // 無効なレイヤーまたはエラー文字
    }
}

//////////////////////////////////////////////////////////////////////////////
