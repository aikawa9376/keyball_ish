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

#include "utils/functions.h"
#include "features/auto_click_layer.h"
#include "features/one_tap_multi_click.h"
#include "features/macro_keys.h"
#include "features/combo.h"
#include "features/tap_hold.h"

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    // keymap for default
    [0] = LAYOUT_universal(
        KC_Q       , KC_W    , KC_E    , KC_R        , KC_T    ,                       KC_Y         , KC_U    , KC_I    , KC_O    , KC_P    ,
        KC_A       , KC_S    , KC_D    , LT(4, KC_F) , KC_G    ,                       KC_H         , KC_J    , KC_K    , KC_L    , LT(1, KC_SCLN) ,
        KC_Z       , KC_X    , KC_C    , KC_V        , KC_B    ,                       KC_N         , KC_M    , KC_COMM , KC_DOT  , LT(6, KC_SLSH) ,
        KC_LGUI    , KC_LALT , MO(3)   , KC_LALT     , MC_ESC  , MO(3),         MO(2), SFT_T(KC_SPC), _______ , _______ , _______ , KC_RSFT
    ),

    [1] = LAYOUT_universal(
        KC_EXLM  , KC_AT    , KC_HASH  , KC_DLR   , KC_PERC  ,                            KC_CIRC  , KC_AMPR  , KC_UNDS  , KC_EQL   , KC_BSLS  ,
        KC_LPRN  , KC_RPRN  , KC_LBRC  , KC_RBRC  , KC_PIPE  ,                            KC_LABK  , KC_RABK  , KC_GRV   , KC_DQT   , KC_QUOT  ,
        KC_LCBR  , KC_RCBR  , C(KC_C)  , G(KC_V)  , G(KC_SPC),                            KC_PLUS  , KC_MINS  , KC_PGDN  , KC_PGUP  , KC_TILD  ,
        _______  , _______  , _______  , MC_TMCP  , MC_TMUX  , _______  ,      KC_LCBR ,  KC_RCBR  , _______  , _______  , _______  , _______
    ),

    [2] = LAYOUT_universal(
        KC_F1   , KC_F2    , KC_F3     , KC_F4    , KC_F5    ,                            KC_F6    , KC_F7    , KC_F8    , KC_F9    , KC_F10   ,
        KC_1    , KC_2     , KC_3      , KC_4     , KC_5     ,                            KC_6     , KC_7     , KC_8     , KC_9     , KC_0     ,
        KC_PLUS , KC_MINS  , KC_ASTR   , KC_SLSH  , KC_EQL   ,                            KC_SPC   , _______  , _______  , _______  , _______  ,
        _______ , _______  , _______   , KC_F11   , KC_F12   , _______  ,      _______ ,  _______  , _______  , _______  , _______  , _______
    ),

    [3] = LAYOUT_universal(
        G(KC_Q)  , C(KC_F4) , _______  , _______  , _______   ,                            S(KC_LEFT)   , S(KC_DOWN)  , S(KC_UP)    , S(KC_RGHT)  , _______  ,
        G(KC_A)  , _______  , C(KC_UP) ,C(KC_DOWN), C(KC_PGDN),                            KC_LEFT      , KC_DOWN     , KC_UP       , KC_RGHT     , MC_TMUX  ,
        _______  , _______  , _______  , _______  , _______   ,                            G(KC_SCLN)   , G(KC_SLSH)  , G(KC_LBRC)  , G(KC_QUOT)  , ALT_TAB  ,
        _______  , _______  , _______  , _______  , _______   , _______  ,      _______ ,  _______      , _______     , _______     , _______     , _______
    ),

    [4] = LAYOUT_universal(
        _______  , _______ , _______  , _______  , _______  ,                            G(KC_Q)  , MC_TAB   , MC_STAB  , _______  , _______  ,
        _______  , _______ , _______  , _______  , _______  ,                            C(KC_F4) , MC_ESC   , _______  , MC_TMCP  , MC_TMUX  ,
        _______  , _______ , _______  , _______  , _______  ,                            _______  , MC_J     , MC_K     , _______  , _______  ,
        _______  , _______ , _______  , _______  , _______  , _______  ,      _______ ,  _______  , _______  , _______  , _______  , _______
    ),

    [5] = LAYOUT_universal(
        SCRL_MO  , SCRL_WD  , _______  , KC_TRPB  , SCRL_TB  ,                            _______  , _______  , AC_INS   , _______  , _______  ,
        SCRL_HO  , _______  ,_______   ,_______   , KC_DBLB  ,                            AC_KEP   ,KC_OG_BTN1,KC_OG_BTN2, _______  , SCRL_HO  ,
        SCRL_VR  , _______  ,KC_OG_BTN4,KC_OG_BTN5, _______  ,                            _______  ,KC_OG_BTN4,KC_OG_BTN5, _______  , _______  ,
        _______  , _______  , _______  , _______  , _______  , _______  ,      _______  , _______  , _______  , _______  , _______  , _______
    ),

    [6] = LAYOUT_universal(
        KC_CIRC  , KC_AMPR  , KC_UNDS  , KC_EQL   , KC_BSLS  ,                            KC_CIRC  , KC_AMPR  , KC_UNDS  , KC_EQL   , KC_BSLS  ,
        KC_ASTR  , KC_SLSH  , KC_GRV   , KC_DQT   , KC_QUOT  ,                            KC_LABK  , KC_RABK  , KC_GRV   , KC_DQT   , KC_QUOT  ,
        KC_PLUS  , KC_MINS  , KC_PGDN  , KC_PGUP  , KC_TILD  ,                            KC_PLUS  , KC_MINS  , KC_PGDN  , KC_PGUP  , KC_TILD  ,
        _______  , _______  , _______  , _______  , _______  , _______  ,      _______ ,  _______  , _______  , _______  , _______  , _______
    ),

};
// clang-format on

layer_state_t layer_state_set_user(layer_state_t state) {
    // Auto enable scroll mode when the highest layer is 3

    // レイヤー移動のたびに新設したkeep_click_layerをオフにする
    if(get_highest_layer(state) <= 4) {
        keep_click_layer = false;
    }
    return state;
}

#ifdef OLED_ENABLE

#include "lib/oledkit/oledkit.h"

void oledkit_render_info_user(void) {
  // デバッグ用に変数を表示する
  // oled_write_P(PSTR("Debug:"), false);
  // oled_write(get_u8_str(xxx, ' '), false);

  keyball_oled_render_keyinfo();   // キー情報を表示
  keyball_oled_render_ballinfo();  // トラックボール情報を表示
  // keyball_oled_render_layerinfo();  // トラックボール情報を表示

  // <Layer>を表示する
  oled_write_P(PSTR("L\xB6\xB7r\xB1"), false);
  for (uint8_t i = 1; i < DYNAMIC_KEYMAP_LAYER_COUNT; i++) {
      oled_write_char(keyball_get_oled_layer_char(i), false);
  }

  // <マウス移動量 / クリックレイヤーしきい値>を表示
  // oled_write_P(PSTR(" MV:"), false);
  // oled_write(get_u8_str(mouse_movement, ' '), false);
  // oled_write_P(PSTR("/"), false);
  // oled_write(get_u8_str(to_clickable_movement, ' '), false);

  // <state>を表示
  switch (state) {
    case WAITING:
      oled_write_ln_P(PSTR(" WAITING"), false);
      break;
    case CLICKABLE:
      oled_write_ln_P(PSTR(" CLICKABLE"), false);
      break;
    case CLICKING:
      oled_write_ln_P(PSTR(" CLICKING"), false);
      break;
    case CLICKED:
      oled_write_ln_P(PSTR(" CLICKED"), false);
      break;
    case NONE:
      oled_write_ln_P(PSTR(" NONE"), false);
      break;
  }
}
#endif
