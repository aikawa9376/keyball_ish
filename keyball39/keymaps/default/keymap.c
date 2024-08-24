/*
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

#include QMK_KEYBOARD_H

#include "raw_hid.h"

#include "keymap.h"
#include "utils/functions.h"
#include "lib/naginata/naginata.h"
#include "features/auto_click_layer.h"
#include "features/one_tap_multi_click.h"
#include "features/macro_keys.h"
#include "features/combo.h"
#include "features/tap_dance.h"
#include "features/tap_hold.h"
#include "features/os_detection.h"

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    // keymap for default
    [_DEFAULT] = LAYOUT_universal(
        KC_Q       , KC_W    , KC_E    , KC_R        , KC_T    ,                       KC_Y   , KC_U     , KC_I    , KC_O    , KC_P    ,
        KC_A       , KC_S    , KC_D    , LT(UT, KC_F), KC_G    ,                       KC_H   , KC_J     , KC_K    , KC_L    , LT(SM, KC_SCLN) ,
        KC_Z       , KC_X    , KC_C    , KC_V        , KC_B    ,                       KC_N   , KC_M     , KC_COMM , KC_DOT  , KC_SLSH ,
        KC_LGUI    , KC_SPC  , KC_BSPC , KC_LALT     , MC_ESC  , MC_LSFT,     MC_RSFT, MC_ESC ,  _______ , _______ , _______ , KC_RSFT
    ),

    [_NAGINATA] = LAYOUT_universal(
        NG_Q     , NG_W     , NG_E     , NG_R    , NG_T      ,                            NG_Y    , NG_U     , NG_I     , NG_O    , NG_P     ,
        NG_A     , NG_S     , NG_D     , NG_F    , NG_G      ,                            NG_H    , NG_J     , NG_K     , NG_L    , NG_SCLN  ,
        NG_Z     , NG_X     , NG_C     , NG_V    , NG_B      ,                            NG_N    , NG_M     , NG_COMM  , NG_DOT  , NG_SLSH  ,
        _______  , _______  , _______  , _______ , _______   , NG_SHFT  ,      NG_SHFT  , NG_MOFF , _______  , _______  , _______ , _______
    ),

    [_SYMBOL] = LAYOUT_universal(
        KC_EXLM   , KC_AT   , KC_HASH  , KC_DLR  , KC_PERC    ,                            KC_CIRC  , KC_AMPR  , KC_UNDS  , KC_EQL  , KC_BSLS  ,
        KC_LPRN   , KC_RPRN , KC_LBRC  , KC_RBRC , KC_PIPE    ,                            KC_ASTR  , KC_SLSH  , KC_GRV   , KC_DQT  , KC_QUOT  ,
        KC_LCBR   , KC_RCBR , C(KC_C)  , G(KC_V) , G(KC_SPC)  ,                            KC_PLUS  , KC_MINS  , KC_PGDN  , KC_PGUP , KC_TILD  ,
        _______   , _______ , _______  , KC_PGUP , KC_PGDN    , _______  ,      KC_LCBR ,  KC_RCBR  , _______  , _______  , _______ , _______
    ),

    [_NUM] = LAYOUT_universal(
        KC_F1   , KC_F2    , KC_F3     , KC_F4    , KC_F5    ,                            KC_F6    , KC_F7    , KC_F8    , KC_F9    , KC_F10   ,
        KC_1    , KC_2     , KC_3      , KC_4     , KC_5     ,                            KC_6     , KC_7     , KC_8     , KC_9     , KC_0     ,
        KC_PLUS , KC_MINS  , KC_ASTR   , KC_SLSH  , KC_EQL   ,                            KC_SPC   , _______  , _______  , _______  , _______  ,
        _______ , _______  , _______   , KC_F11   , KC_F12   , _______  ,      _______ ,  _______  , _______  , _______  , _______  , QK_BOOT
    ),

    [_MOVE] = LAYOUT_universal(
        G(KC_Q)  , C(KC_F4) , _______  , _______  , _______   ,                            S(KC_LEFT)   , S(KC_DOWN)  , S(KC_UP)    , S(KC_RGHT)  , _______  ,
        G(KC_A)  , _______  , C(KC_UP) ,C(KC_DOWN), C(KC_PGDN),                            KC_LEFT      , KC_DOWN     , KC_UP       , KC_RGHT     , MC_TMUX  ,
        _______  , _______  , _______  , _______  , _______   ,                            G(KC_SCLN)   , G(KC_SLSH)  , G(KC_LBRC)  , G(KC_QUOT)  , ALT_TAB  ,
        _______  , _______  , _______  , _______  , _______   , _______  ,      _______ ,  _______      , _______     , _______     , _______     , _______
    ),

    [_UTIL] = LAYOUT_universal(
        _______  , _______ , _______  , _______  , _______  ,                            G(KC_Q)  , MC_TAB   , MC_STAB  , _______  , _______  ,
        _______  , _______ , _______  , _______  , _______  ,                            C(KC_F4) , KC_RSFT  , _______  , MC_TMCP  , MC_TMUX  ,
        _______  , _______ , _______  , _______  , _______  ,                            _______  , MC_J     , MC_K     , _______  , _______  ,
        _______  , _______ , _______  , _______  , _______  , _______  ,      _______ ,  _______  , _______  , _______  , _______  , NG_MON
    ),

    [_MOUSE] = LAYOUT_universal(
        SCRL_MO  , SCRL_WD  , _______  , KC_TRPB  , SCRL_TB  ,                            _______  , _______  , AC_INS   , _______  , _______  ,
        SCRL_HO  , _______  , _______  , _______  , KC_DBLB  ,                            AC_KEP   ,KC_OG_BTN1,KC_OG_BTN2, _______  , SCRL_HO  ,
        SCRL_VR  , _______  ,KC_OG_BTN4,KC_OG_BTN5, _______  ,                            _______  ,KC_OG_BTN4,KC_OG_BTN5, _______  , _______  ,
        _______  , _______  , _______  , _______  , _______  , _______  ,      _______  , _______  , _______  , _______  , _______  , _______
    ),

};
// clang-format on

// init keyboard
void matrix_init_user(void) {
    set_naginata_layer(_NAGINATA);
}

void raw_hid_receive(uint8_t *data, uint8_t length) {
    // Your code goes here
    // `data` is a pointer to the buffer containing the received HID report
    // `length` is the length of the report - always `RAW_EPSIZE`
    raw_hid_send(data, length);
}

layer_state_t layer_state_set_user(layer_state_t state) {
    // Auto enable scroll mode when the highest layer is 3

    // レイヤー移動のたびに新設したkeep_click_layerをオフにする
    if(get_highest_layer(state) <= _MOUSE) {
        keep_click_layer = false;
    }
    return state;
}
