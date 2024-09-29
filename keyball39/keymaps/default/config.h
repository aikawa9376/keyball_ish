/*
This is the c configuration file for the keymap

Copyright 2022 @Yowkees
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

#define KEYBALL_SCROLLSNAP_ENABLE 0  // scrollsnapをオフにする
#define NO_ACTION_ONESHOT  // 392バイト

#define TAP_CODE_DELAY 5
#define DYNAMIC_KEYMAP_LAYER_COUNT 7

#define COMBO_COUNT 3
#define COMBO_MUST_PRESS_IN_ORDER_PER_COMBO
#define COMBO_MUST_HOLD_PER_COMBO

#define LEADER_TIMEOUT 120

#define KEYBALL_CPI_DEFAULT 500      // 光学センサーPMW3360DM の解像度 (CPI) の規定値
#define KEYBALL_SCROLL_DIV_DEFAULT 6  // スクロール速度の規定値

#define TAPPING_TERM 6000
#define QUICK_TAP_TERM 120

// #define RETRO_TAPPING
#define TAPPING_TERM_PER_KEY
#define COMBO_TERM_PER_COMBO
#define PERMISSIVE_HOLD_PER_KEY
#define HOLD_ON_OTHER_KEY_PRESS_PER_KEY
#define QUICK_TAP_TERM_PER_KEY

// #define UNICODE_TYPE_DELAY 32
