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

#include "drivers/pmw3360/pmw3360.h"

const uint8_t CPI_DEFAULT    = KEYBALL_CPI_DEFAULT / 100;
const uint8_t CPI_MAX        = pmw3360_MAXCPI + 1;
const uint8_t SCROLL_DIV_MAX = 7;

static const char BL = '\xB0'; // Blank indicator character
static const char LFSTR_ON[] PROGMEM = "\xB2\xB3";
static const char LFSTR_OFF[] PROGMEM = "\xB4\xB5";

uint16_t horizontal_flag = 0;

keyball_t keyball = {
    .this_have_ball = false,
    .that_enable    = false,
    .that_have_ball = false,

    .this_motion = {0},
    .that_motion = {0},

    .cpi_value   = 0,
    .cpi_changed = false,

    .scroll_mode = false,
    .scroll_div  = 0,

    .pressing_keys = { BL, BL, BL, BL, BL, BL, 0 },
};

//////////////////////////////////////////////////////////////////////////////
// Hook points

__attribute__((weak)) void keyball_on_adjust_layout(keyball_adjust_t v) {}

__attribute__((weak)) uint16_t keyball_get_scroll_resolution(void) {
#ifdef POINTING_DEVICE_HIRES_SCROLL_ENABLE
    return pointing_device_get_hires_scroll_resolution();
#else
    return 1;
#endif
}

//////////////////////////////////////////////////////////////////////////////
// Static utilities

typedef struct {
    int32_t  remainder_x;
    int32_t  remainder_y;
    uint32_t speed_x100;
    uint32_t last_update;
    int8_t   sign_x;
    int8_t   sign_y;
} keyball_motion_filter_t;

typedef struct {
    int32_t remainder_x;
    int32_t remainder_y;
    uint32_t last_motion;
    uint8_t  axis_lock;
} keyball_scroll_filter_t;

static keyball_motion_filter_t this_motion_filter = {0};
static keyball_motion_filter_t that_motion_filter = {0};
static keyball_scroll_filter_t this_scroll_filter = {0};
static keyball_scroll_filter_t that_scroll_filter = {0};

// add16 adds two int16_t with clipping.
static int16_t add16(int16_t a, int16_t b) {
    int32_t r = (int32_t)a + b;
    return r < INT16_MIN ? INT16_MIN : r > INT16_MAX ? INT16_MAX : (int16_t)r;
}

static uint32_t isqrt32(uint32_t n) {
    uint32_t result = 0;
    uint32_t bit    = 1UL << 30;

    while (bit > n) {
        bit >>= 2;
    }
    while (bit != 0) {
        if (n >= result + bit) {
            n -= result + bit;
            result = (result >> 1) + bit;
        } else {
            result >>= 1;
        }
        bit >>= 2;
    }
    return result;
}

static int8_t sign16(int32_t v) {
    return v < 0 ? -1 : v > 0 ? 1 : 0;
}

static int32_t constrain_mouse_xy(int32_t value) {
    return CONSTRAIN_HID_XY(value);
}

static int32_t constrain_mouse_hv(int32_t value) {
    return value < MOUSE_REPORT_HV_MIN ? MOUSE_REPORT_HV_MIN : value > MOUSE_REPORT_HV_MAX ? MOUSE_REPORT_HV_MAX : value;
}

static uint16_t acceleration_gain(uint32_t speed_x100) {
#if KEYBALL_ACCEL_ENABLE
    if (speed_x100 <= KEYBALL_ACCEL_START_X100) {
        return KEYBALL_ACCEL_MIN_GAIN_X100;
    }
    if (speed_x100 >= KEYBALL_ACCEL_END_X100) {
        return KEYBALL_ACCEL_MAX_GAIN_X100;
    }

    // Smoothstep interpolation avoids the hard jumps of the old buckets.
    uint32_t t = (speed_x100 - KEYBALL_ACCEL_START_X100) * 1000U / (KEYBALL_ACCEL_END_X100 - KEYBALL_ACCEL_START_X100);
    uint32_t s = t * t * (3000U - 2U * t) / 1000000U;
    return KEYBALL_ACCEL_MIN_GAIN_X100 + ((KEYBALL_ACCEL_MAX_GAIN_X100 - KEYBALL_ACCEL_MIN_GAIN_X100) * s) / 1000U;
#else
    (void)speed_x100;
    return 100;
#endif
}

static int32_t apply_gain(int32_t value, uint16_t gain_x100, int32_t *remainder) {
    int64_t scaled = ((int64_t)value * gain_x100 * 65536) / 100 + *remainder;
    int32_t output = (int32_t)(scaled / 65536);
    *remainder     = (int32_t)(scaled - (int64_t)output * 65536);

    int32_t limited = constrain_mouse_xy(output);
    if (limited != output) {
        int64_t retained = (int64_t)*remainder + ((int64_t)output - limited) * 65536;
        *remainder       = retained < INT32_MIN ? INT32_MIN : retained > INT32_MAX ? INT32_MAX : (int32_t)retained;
    }
    return limited;
}

static void reset_motion_filter(keyball_motion_filter_t *filter) {
    filter->remainder_x = 0;
    filter->remainder_y = 0;
    filter->speed_x100  = 0;
    filter->last_update = 0;
    filter->sign_x      = 0;
    filter->sign_y      = 0;
}

static void adjust_mouse_speed(keyball_motion_t *m, keyball_motion_filter_t *filter, uint32_t now) {
    uint32_t elapsed = filter->last_update == 0 ? KEYBALL_REPORTMOUSE_INTERVAL : TIMER_DIFF_32(now, filter->last_update);
    filter->last_update = now;

    if (elapsed == 0) {
        elapsed = 1;
    }
    if (elapsed >= KEYBALL_ACCEL_RESET_TIMER) {
        filter->speed_x100 = 0;
        filter->remainder_x = 0;
        filter->remainder_y = 0;
        filter->sign_x = 0;
        filter->sign_y = 0;
    }
    if (m->x == 0 && m->y == 0) {
        return;
    }

    int8_t sx = sign16(m->x);
    int8_t sy = sign16(m->y);
    bool   direction_changed = (sx != 0 && filter->sign_x != 0 && sx != filter->sign_x) || (sy != 0 && filter->sign_y != 0 && sy != filter->sign_y);
    if (direction_changed) {
        filter->speed_x100 = 0;
        if (sx != 0 && sx != filter->sign_x) {
            filter->remainder_x = 0;
        }
        if (sy != 0 && sy != filter->sign_y) {
            filter->remainder_y = 0;
        }
    }
    if (sx != 0) {
        filter->sign_x = sx;
    }
    if (sy != 0) {
        filter->sign_y = sy;
    }

    int32_t  x          = m->x;
    int32_t  y          = m->y;
    uint32_t magnitude = isqrt32((uint32_t)((uint64_t)x * x + (uint64_t)y * y));
    uint32_t cpi        = (uint32_t)keyball_get_cpi() * 100U;
    uint32_t speed_x100 = (uint32_t)(((uint64_t)magnitude * 100000U) / ((uint64_t)cpi * elapsed));

    if (filter->speed_x100 == 0 || direction_changed) {
        filter->speed_x100 = speed_x100;
    } else {
        // Scalar EMA only; filtering X/Y themselves would add visible lag.
        filter->speed_x100 = (filter->speed_x100 + speed_x100) / 2U;
    }

    uint16_t gain = acceleration_gain(filter->speed_x100);
    m->x          = (int16_t)apply_gain(m->x, gain, &filter->remainder_x);
    m->y          = (int16_t)apply_gain(m->y, gain, &filter->remainder_y);
}

static int32_t apply_scroll_axis(int16_t *value, int32_t *remainder, int16_t divisor, uint16_t resolution) {
    int64_t scaled = (int64_t)*value * resolution + *remainder;
    int32_t output = (int32_t)(scaled / divisor);
    *remainder     = (int32_t)(scaled - (int64_t)output * divisor);
    *value         = 0;

    int32_t limited = constrain_mouse_hv(output);
    if (limited != output) {
        int64_t retained = (int64_t)*remainder + ((int64_t)output - limited) * divisor;
        *remainder       = retained < INT32_MIN ? INT32_MIN : retained > INT32_MAX ? INT32_MAX : (int32_t)retained;
    }
    return limited;
}

static void reset_scroll_filter(keyball_scroll_filter_t *filter) {
    filter->remainder_x = 0;
    filter->remainder_y = 0;
    filter->last_motion = 0;
    filter->axis_lock   = 0;
}

#ifdef OLED_ENABLE
static const char *format_4d(int16_t d) {
    static char buf[5] = {0}; // max width (4) + NUL (1)
    int32_t     value  = d;
    char        lead   = ' ';
    if (value < 0) {
        value = -value;
        lead = '-';
    }
    buf[3] = (value % 10) + '0';
    value /= 10;
    if (value == 0) {
        buf[2] = lead;
        lead   = ' ';
    } else {
        buf[2] = (value % 10) + '0';
        value /= 10;
    }
    if (value == 0) {
        buf[1] = lead;
        lead   = ' ';
    } else {
        buf[1] = (value % 10) + '0';
        value /= 10;
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

#if KEYBALL_MODEL == 46
void keyboard_pre_init_kb(void) {
    keyball.this_have_ball = pmw3360_init();
    keyboard_pre_init_user();
}
#endif

bool pointing_device_driver_init(void) {
#if KEYBALL_MODEL != 46
    keyball.this_have_ball = pmw3360_init();
#endif
    if (keyball.this_have_ball) {
#if defined(KEYBALL_PMW3360_UPLOAD_SROM_ID)
#    if KEYBALL_PMW3360_UPLOAD_SROM_ID == 0x04
        pmw3360_srom_upload(pmw3360_srom_0x04);
#    elif KEYBALL_PMW3360_UPLOAD_SROM_ID == 0x81
        pmw3360_srom_upload(pmw3360_srom_0x81);
#    else
#        error Invalid value for KEYBALL_PMW3360_UPLOAD_SROM_ID. Choose 0x04 or 0x81, or leave it undefined.
#    endif
        if (pmw3360_srom_id != KEYBALL_PMW3360_UPLOAD_SROM_ID) {
#if defined(CONSOLE_ENABLE)
            dprintf("pmw3360: SROM verification failed (expected 0x%02X, got 0x%02X)\n", KEYBALL_PMW3360_UPLOAD_SROM_ID, pmw3360_srom_id);
#endif
            return false;
        }
#endif
        pmw3360_cpi_set(CPI_DEFAULT - 1);
        pmw3360_reg_write(pmw3360_Angle_Tune, (uint8_t)KEYBALL_PMW3360_ANGLE_TUNE);
        pmw3360_reg_write(pmw3360_Motion_Burst, 0);
    }

    return true;
}

uint16_t pointing_device_driver_get_cpi(void) {
    return keyball_get_cpi();
}

void pointing_device_driver_set_cpi(uint16_t cpi) {
    keyball_set_cpi(cpi);
}

static void motion_to_mouse_move(keyball_motion_t *m, report_mouse_t *r, bool is_left, keyball_motion_filter_t *filter, uint32_t now) {
    adjust_mouse_speed(m, filter, now);
#if KEYBALL_MODEL == 61 || KEYBALL_MODEL == 39 || KEYBALL_MODEL == 147 || KEYBALL_MODEL == 44
    r->x = (mouse_xy_report_t)constrain_mouse_xy(m->y);
    r->y = (mouse_xy_report_t)constrain_mouse_xy(m->x);
    if (is_left) {
        r->x = -r->x;
        r->y = -r->y;
    }
#elif KEYBALL_MODEL == 46
    r->x = (mouse_xy_report_t)constrain_mouse_xy(m->x);
    r->y = (mouse_xy_report_t)-constrain_mouse_xy(m->y);
#else
#    error("unknown Keyball model")
#endif
    // clear motion
    m->x = 0;
    m->y = 0;
}

static void motion_to_mouse_scroll(keyball_motion_t *m, report_mouse_t *r, bool is_left, keyball_scroll_filter_t *filter, uint32_t now) {
    // Consume motion with signed division and retain the exact remainder.
    int16_t  divisor   = 1 << (keyball_get_scroll_div() - 1);
    uint16_t resolution = keyball_get_scroll_resolution();
#if KEYBALL_SCROLL_AXIS_LOCK_ENABLE
    int16_t  raw_x   = m->x;
    int16_t  raw_y   = m->y;
    uint32_t elapsed = filter->last_motion == 0 ? 0 : TIMER_DIFF_32(now, filter->last_motion);
    if (raw_x == 0 && raw_y == 0) {
        if (elapsed >= KEYBALL_SCROLL_AXIS_LOCK_RESET_TIMER) {
            filter->axis_lock = 0;
        }
    } else {
        filter->last_motion = now;
        if (filter->axis_lock == 0) {
            uint32_t ax = raw_x < 0 ? -(int32_t)raw_x : raw_x;
            uint32_t ay = raw_y < 0 ? -(int32_t)raw_y : raw_y;
            if (ay * 100U >= ax * KEYBALL_SCROLL_AXIS_LOCK_RATIO_X100) {
                filter->axis_lock = 1; // horizontal wheel (sensor Y)
            } else if (ax * 100U >= ay * KEYBALL_SCROLL_AXIS_LOCK_RATIO_X100) {
                filter->axis_lock = 2; // vertical wheel (sensor X)
            }
        }
    }
#endif
    int32_t x = apply_scroll_axis(&m->x, &filter->remainder_x, divisor, resolution);
    int32_t y = apply_scroll_axis(&m->y, &filter->remainder_y, divisor, resolution);

    // apply to mouse report.
#if KEYBALL_MODEL == 61 || KEYBALL_MODEL == 39 || KEYBALL_MODEL == 147 || KEYBALL_MODEL == 44
    r->h = (mouse_hv_report_t)y;
    r->v = (mouse_hv_report_t)-x;
    if (is_left) {
        r->h = -r->h;
        r->v = -r->v;
    }
    if (horizontal_flag == 1) {
        r->h = 0;
    } else if (horizontal_flag == 2) {
        r->v = 0;
    }
#if KEYBALL_SCROLL_AXIS_LOCK_ENABLE
    else if (filter->axis_lock == 1) {
        r->v = 0;
    } else if (filter->axis_lock == 2) {
        r->h = 0;
    }
#endif
#elif KEYBALL_MODEL == 46
    r->h = (mouse_hv_report_t)x;
    r->v = (mouse_hv_report_t)y;
#if KEYBALL_SCROLL_AXIS_LOCK_ENABLE
    if (filter->axis_lock == 1) {
        r->v = 0;
    } else if (filter->axis_lock == 2) {
        r->h = 0;
    }
#endif
#else
#    error("unknown Keyball model")
#endif

#if KEYBALL_SCROLLSNAP_ENABLE
    // scroll snap.
    if (r->h != 0 || r->v != 0) {
        keyball.scroll_snap_last = now;
    } else if (TIMER_DIFF_32(now, keyball.scroll_snap_last) >= KEYBALL_SCROLLSNAP_RESET_TIMER) {
        keyball.scroll_snap_tension_h = 0;
    }
    int32_t threshold = KEYBALL_SCROLLSNAP_TENSION_THRESHOLD;
#ifdef POINTING_DEVICE_HIRES_SCROLL_ENABLE
    threshold *= resolution;
#endif
    if (abs(keyball.scroll_snap_tension_h) < threshold) {
        int32_t tension = keyball.scroll_snap_tension_h + y;
        keyball.scroll_snap_tension_h = tension < INT16_MIN ? INT16_MIN : tension > INT16_MAX ? INT16_MAX : tension;
        r->h = 0;
    }
#endif
}

static void motion_to_mouse(keyball_motion_t *m, report_mouse_t *r, bool is_left, bool as_scroll, keyball_motion_filter_t *motion_filter, keyball_scroll_filter_t *scroll_filter, uint32_t now) {
    if (as_scroll) {
        motion_to_mouse_scroll(m, r, is_left, scroll_filter, now);
    } else {
        motion_to_mouse_move(m, r, is_left, motion_filter, now);
    }
}

static inline bool should_report(uint32_t now) {
#if defined(KEYBALL_REPORTMOUSE_INTERVAL) && KEYBALL_REPORTMOUSE_INTERVAL > 0
    // throttling mouse report rate.
    static uint32_t last = 0;
    if (TIMER_DIFF_32(now, last) < KEYBALL_REPORTMOUSE_INTERVAL) {
        return false;
    }
    last = now;
#endif
#if defined(KEYBALL_SCROLLBALL_INHIVITOR) && KEYBALL_SCROLLBALL_INHIVITOR > 0
    if (TIMER_DIFF_32(now, keyball.scroll_mode_changed) < KEYBALL_SCROLLBALL_INHIVITOR) {
        keyball.this_motion.x = 0;
        keyball.this_motion.y = 0;
        keyball.that_motion.x = 0;
        keyball.that_motion.y = 0;
    }
#endif
    return true;
}

report_mouse_t pointing_device_driver_get_report(report_mouse_t rep) {
    uint32_t now = timer_read32();
    // fetch from optical sensor.
    if (keyball.this_have_ball) {
        pmw3360_motion_t d = {0};
        if (pmw3360_motion_burst(&d)) {
            ATOMIC_BLOCK_FORCEON {
                keyball.this_motion.x = add16(keyball.this_motion.x, d.x);
                keyball.this_motion.y = add16(keyball.this_motion.y, d.y);
            }
        }
    }
    // report mouse event, if keyboard is primary.
    if (is_keyboard_master() && should_report(now)) {
        // modify mouse report by PMW3360 motion.
        motion_to_mouse(&keyball.this_motion, &rep, is_keyboard_left(), keyball.scroll_mode, &this_motion_filter, &this_scroll_filter, now);
        motion_to_mouse(&keyball.that_motion, &rep, !is_keyboard_left(), keyball.scroll_mode ^ keyball.this_have_ball, &that_motion_filter, &that_scroll_filter, now);
        // store mouse report for OLED.
        keyball.last_mouse = rep;
    }
    return rep;
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
    *(keyball_motion_t *)out_data = keyball.this_motion;
    // clear motion
    keyball.this_motion.x = 0;
    keyball.this_motion.y = 0;
}

static void rpc_get_motion_invoke(void) {
    static uint32_t last_sync = 0;
    uint32_t        now       = timer_read32();
    if (TIMER_DIFF_32(now, last_sync) < KEYBALL_TX_GETMOTION_INTERVAL) {
        return;
    }
    keyball_motion_t recv = {0};
    if (transaction_rpc_exec(KEYBALL_GET_MOTION, 0, NULL, sizeof(recv), &recv)) {
        keyball.that_motion.x = add16(keyball.that_motion.x, recv.x);
        keyball.that_motion.y = add16(keyball.that_motion.y, recv.y);
    }
    last_sync = now;
    return;
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
// Public API functions

bool keyball_get_scroll_mode(void) {
    return keyball.scroll_mode;
}

void keyball_set_scroll_mode(bool mode) {
    if (mode != keyball.scroll_mode) {
        keyball.scroll_mode_changed = timer_read32();
        reset_motion_filter(&this_motion_filter);
        reset_motion_filter(&that_motion_filter);
        reset_scroll_filter(&this_scroll_filter);
        reset_scroll_filter(&that_scroll_filter);
    }
    keyball.scroll_mode = mode;
}

uint8_t keyball_get_scroll_div(void) {
    return keyball.scroll_div == 0 ? KEYBALL_SCROLL_DIV_DEFAULT : keyball.scroll_div;
}

void keyball_set_scroll_div(uint8_t div) {
    keyball.scroll_div = div > SCROLL_DIV_MAX ? SCROLL_DIV_MAX : div;
    reset_scroll_filter(&this_scroll_filter);
    reset_scroll_filter(&that_scroll_filter);
}

uint8_t keyball_get_cpi(void) {
    return keyball.cpi_value == 0 ? CPI_DEFAULT : keyball.cpi_value;
}

void keyball_set_cpi(uint8_t cpi) {
    if (cpi > CPI_MAX) {
        cpi = CPI_MAX;
    }
    keyball.cpi_value   = cpi;
    keyball.cpi_changed = true;
    reset_motion_filter(&this_motion_filter);
    reset_motion_filter(&that_motion_filter);
    if (keyball.this_have_ball) {
        pmw3360_cpi_set(cpi == 0 ? CPI_DEFAULT - 1 : cpi - 1);
        pmw3360_reg_write(pmw3360_Motion_Burst, 0);
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
            rpc_get_motion_invoke();
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
        // process MS_BTN1~8 by myself
        // See process_action() in quantum/action.c for details.
        case MS_BTN1 ... MS_BTN8: {
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
