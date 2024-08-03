/*
Copyright 2022 MURAOKA Taro (aka KoRoN, @kaoriya)

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

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Configurations

#ifndef KEYBALL_CPI_DEFAULT
#    define KEYBALL_CPI_DEFAULT 500
#endif

#ifndef KEYBALL_SCROLL_DIV_DEFAULT
#    define KEYBALL_SCROLL_DIV_DEFAULT 4 // 4: 1/8 (1/2^(n-1))
#endif

#ifndef KEYBALL_REPORTMOUSE_INTERVAL
#    define KEYBALL_REPORTMOUSE_INTERVAL 8 // mouse report rate: 125Hz
#endif

#ifndef KEYBALL_SCROLLBALL_INHIVITOR
#    define KEYBALL_SCROLLBALL_INHIVITOR 50
#endif

#ifndef KEYBALL_SCROLLSNAP_ENABLE
#    define KEYBALL_SCROLLSNAP_ENABLE 1
#endif

#ifndef KEYBALL_SCROLLSNAP_RESET_TIMER
#    define KEYBALL_SCROLLSNAP_RESET_TIMER 100
#endif

#ifndef KEYBALL_SCROLLSNAP_TENSION_THRESHOLD
#    define KEYBALL_SCROLLSNAP_TENSION_THRESHOLD 12
#endif

//////////////////////////////////////////////////////////////////////////////
// Constants

#define KEYBALL_TX_GETINFO_INTERVAL 500
#define KEYBALL_TX_GETINFO_MAXTRY 10
#define KEYBALL_TX_GETMOTION_INTERVAL 4

#if (PRODUCT_ID & 0xff00) == 0x0000
#    define KEYBALL_MODEL 46
#elif (PRODUCT_ID & 0xff00) == 0x0100
#    define KEYBALL_MODEL 61
#elif (PRODUCT_ID & 0xff00) == 0x0200
#    define KEYBALL_MODEL 39
#elif (PRODUCT_ID & 0xff00) == 0x0300
#    define KEYBALL_MODEL 147
#elif (PRODUCT_ID & 0xff00) == 0x0400
#    define KEYBALL_MODEL 44
#endif

#define KEYBALL_OLED_MAX_PRESSING_KEYCODES 6

// typedef enum {
//     pmw3360_Product_ID                 = 0x00,
//     pmw3360_Revision_ID                = 0x01,
//     pmw3360_Motion                     = 0x02,
//     pmw3360_Delta_X_L                  = 0x03,
//     pmw3360_Delta_X_H                  = 0x04,
//     pmw3360_Delta_Y_L                  = 0x05,
//     pmw3360_Delta_Y_H                  = 0x06,
//     pmw3360_SQUAL                      = 0x07,
//     pmw3360_Raw_Data_Sum               = 0x08,
//     pmw3360_Maximum_Raw_data           = 0x09,
//     pmw3360_Minimum_Raw_data           = 0x0A,
//     pmw3360_Shutter_Lower              = 0x0B,
//     pmw3360_Shutter_Upper              = 0x0C,
//     pmw3360_Control                    = 0x0D,
//     pmw3360_Config1                    = 0x0F,
//     pmw3360_Config2                    = 0x10,
//     pmw3360_Angle_Tune                 = 0x11,
//     pmw3360_Frame_Capture              = 0x12,
//     pmw3360_SROM_Enable                = 0x13,
//     pmw3360_Run_Downshift              = 0x14,
//     pmw3360_Rest1_Rate_Lower           = 0x15,
//     pmw3360_Rest1_Rate_Upper           = 0x16,
//     pmw3360_Rest1_Downshift            = 0x17,
//     pmw3360_Rest2_Rate_Lower           = 0x18,
//     pmw3360_Rest2_Rate_Upper           = 0x19,
//     pmw3360_Rest2_Downshift            = 0x1A,
//     pmw3360_Rest3_Rate_Lower           = 0x1B,
//     pmw3360_Rest3_Rate_Upper           = 0x1C,
//     pmw3360_Observation                = 0x24,
//     pmw3360_Data_Out_Lower             = 0x25,
//     pmw3360_Data_Out_Upper             = 0x26,
//     pmw3360_Raw_Data_Dump              = 0x29,
//     pmw3360_SROM_ID                    = 0x2A,
//     pmw3360_Min_SQ_Run                 = 0x2B,
//     pmw3360_Raw_Data_Threshold         = 0x2C,
//     pmw3360_Config5                    = 0x2F,
//     pmw3360_Power_Up_Reset             = 0x3A,
//     pmw3360_Shutdown                   = 0x3B,
//     pmw3360_Inverse_Product_ID         = 0x3F,
//     pmw3360_LiftCutoff_Tune3           = 0x41,
//     pmw3360_Angle_Snap                 = 0x42,
//     pmw3360_LiftCutoff_Tune1           = 0x4A,
//     pmw3360_Motion_Burst               = 0x50,
//     pmw3360_LiftCutoff_Tune_Timeout    = 0x58,
//     pmw3360_LiftCutoff_Tune_Min_Length = 0x5A,
//     pmw3360_SROM_Load_Burst            = 0x62,
//     pmw3360_Lift_Config                = 0x63,
//     pmw3360_Raw_Data_Burst             = 0x64,
//     pmw3360_LiftCutoff_Tune2           = 0x65,
// } pmw3360_reg_t;

//////////////////////////////////////////////////////////////////////////////
// Types

enum keyball_keycodes {
    KBC_RST  = QK_KB_0, // Keyball configuration: reset to default
    KBC_SAVE = QK_KB_1, // Keyball configuration: save to EEPROM

    CPI_I100 = QK_KB_2, // CPI +100 CPI
    CPI_D100 = QK_KB_3, // CPI -100 CPI
    CPI_I1K  = QK_KB_4, // CPI +1000 CPI
    CPI_D1K  = QK_KB_5, // CPI -1000 CPI

    // In scroll mode, motion from primary trackball is treated as scroll
    // wheel.
    SCRL_TO  = QK_KB_6, // Toggle scroll mode
    SCRL_MO  = QK_KB_7, // Momentary scroll mode
    SCRL_DVI = QK_KB_8, // Increment scroll divider
    SCRL_DVD = QK_KB_9, // Decrement scroll divider

    // User customizable 32 keycodes.
    KEYBALL_SAFE_RANGE = QK_USER_0,
};

typedef union {
    uint32_t raw;
    struct {
        uint8_t cpi : 7;
        uint8_t sdiv : 3; // scroll divider
    };
} keyball_config_t;

typedef struct {
    uint8_t ballcnt; // count of balls: support only 0 or 1, for now
} keyball_info_t;

typedef struct {
    float h;
    float v;
} keyball_scroll_b;

typedef uint8_t keyball_cpi_t;

typedef struct {
    bool this_have_ball;
    bool that_enable;
    bool that_have_ball;

    uint8_t cpi_value;
    bool    cpi_changed;

    bool     scroll_mode;
    uint32_t scroll_mode_changed;
    uint8_t  scroll_div;

    uint32_t scroll_snap_last;
    int8_t   scroll_snap_tension_h;

    uint16_t       last_kc;
    keypos_t       last_pos;
    report_mouse_t last_mouse;

    keyball_scroll_b scroll_buffer;

    // Buffer to indicate pressing keys.
    char pressing_keys[KEYBALL_OLED_MAX_PRESSING_KEYCODES + 1];
} keyball_t;

typedef enum {
    KEYBALL_ADJUST_PENDING   = 0,
    KEYBALL_ADJUST_PRIMARY   = 1,
    KEYBALL_ADJUST_SECONDARY = 2,
} keyball_adjust_t;

//////////////////////////////////////////////////////////////////////////////
// Exported values (touch carefully)

extern keyball_t keyball;

//////////////////////////////////////////////////////////////////////////////
// Public API functions

/// keyball_oled_render_ballinfo renders ball information to OLED.
/// It uses just 21 columns to show the info.
void keyball_oled_render_ballinfo(void);

/// keyball_oled_render_keyinfo renders last processed key information to OLED.
/// It shows column, row, key code, and key name (if available).
void keyball_oled_render_keyinfo(void);

/// keyball_oled_render_layerinfo renders current layer status information to
/// OLED.  It shows layer mask with number (1~f) for active layers and '_' for
/// inactive layers.
void keyball_oled_render_layerinfo(void);

/// keyball_get_scroll_mode gets current scroll mode.
bool keyball_get_scroll_mode(void);

/// keyball_set_scroll_mode modify scroll mode.
void keyball_set_scroll_mode(bool mode);

/// keyball_set_scroll_mode modify scroll mode.
char keyball_get_oled_layer_char(uint8_t layer);

// TODO: document
uint8_t keyball_get_scroll_div(void);

// TODO: document
void keyball_set_scroll_div(uint8_t div);

// TODO: document
uint8_t keyball_get_cpi(void);

// TODO: document
void keyball_set_cpi(uint16_t cpi);
