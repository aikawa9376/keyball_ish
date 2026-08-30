#include QMK_KEYBOARD_H

#include "../keymap.h"
#include "lib/naginata/naginata.h"
#include "auto_click_layer.h"
#include "mouse_buttons.h"
#include "one_tap_multi_click.h"
#include "os_detection.h"
#include "raw_hid_handler.h"
#include "macro_keys.h"
#include "raw_hid.h"
#include "selection_mode.h"

#include "lib/keyball/keyball.h"

extern uint16_t horizontal_flag;

static bool hold_ctrl = false;
static bool hold_lsft = false;
static bool hold_rsft = false;
static bool is_single_tap = true;
static bool is_ime_on = false;
static bool is_lt4_on = false;
static bool is_alt_tab_active = false; // ADD this near the beginning of keymap.c
static uint16_t alt_tab_timer = 0;     // we will be using them soon.

static void mouse_button_func(uint16_t keycode, bool regist_flag) {
    switch(keycode) {
        case KC_OG_BTN1: {
            regist_flag ? register_code(MS_BTN1) : unregister_code(MS_BTN1);
            break;
        }
        case KC_OG_BTN2: {
            regist_flag ? register_code(MS_BTN2) : unregister_code(MS_BTN2);
            break;
        }
        case KC_OG_BTN3: {
            regist_flag ? register_code(MS_BTN3) : unregister_code(MS_BTN3);
            break;
        }
        case KC_OG_BTN4: {
            keyball_set_scroll_mode(false);
            is_single_tap = true;

            if(horizontal_flag == 1) {
                if(regist_flag) {
                    tap_code16(C(KC_C));
                } else {
                    horizontal_flag = 0;
                }
            } else {
                // regist_flag ? register_code(MS_BTN4) : unregister_code(MS_BTN4);
                if(regist_flag) {
                    hold_ctrl = true;
                    register_code(KC_RCTL);
                } else if (is_single_tap) {
                    hold_ctrl = false;
                    unregister_code(KC_RCTL);
                    tap_code16(MS_BTN4);
                }
            }
            break;
        }
        case KC_OG_BTN5: {
            keyball_set_scroll_mode(false);
            is_single_tap = true;

            if(horizontal_flag == 1) {
                if(regist_flag) {
                    tap_code16(C(KC_V));
                } else {
                    horizontal_flag = 0;
                }
            } else {
                // regist_flag ? register_code(MS_BTN5) : unregister_code(MS_BTN5);
                if(regist_flag) {
                    hold_ctrl = true;
                    register_code(KC_LCTL);
                } else if (is_single_tap) {
                    hold_ctrl = false;
                    unregister_code(KC_LCTL);
                    tap_code16(MS_BTN5);
                }
            }
            break;
        }
    }
}

static void disable_click_layer_all_state(void) {
    if (selection_mode_get() == SELECTION_MODE_MOUSE) {
        selection_mode_stop();
    }
    state = NONE;
    horizontal_flag = 0;
    disable_click_layer();
    keyball_set_scroll_mode(false);
    keyball_set_cpi(KEYBALL_CPI_DEFAULT / 100);
    unregister_code(MS_BTN2);
    unregister_code(KC_RALT);
    unregister_code(MS_BTN1);
    unregister_code(MS_BTN2);
    unregister_code(MS_BTN3);
    unregister_code(MS_BTN4);
    unregister_code(MS_BTN5);
}

// マクロキーの処理を行う関数
bool process_record_user(uint16_t keycode, keyrecord_t *record) {

    if (record->event.pressed) {
        // Finalize selections before actions that do not extend them.
        if (keycode != MC_DRAG && selection_mode_should_stop(keycode)) {
            selection_mode_stop();
        }

        is_single_tap = false;
        // ctrlキー押下時に他キーが押されたらクリックレイヤーを解除
        if (hold_ctrl && click_layer && get_highest_layer(layer_state) == click_layer) {
            disable_click_layer_all_state();
            hold_ctrl = false;
        }
    }

    switch (keycode) {
        case MC_DRAG: {
            if (record->event.pressed) {
                selection_mode_toggle(get_highest_layer(layer_state) == click_layer);
            }
            return false;
        }
        case LT(UT, KC_F): {
            disable_click_layer();
            if (record->event.pressed) {
                is_lt4_on = true;
            } else {
                unregister_code16(KC_RALT);
                is_lt4_on = false;
            }
            return true;
        }
        case MC_J: {
            if (record->event.pressed) {
                if (is_lt4_on) {
                    register_code16(KC_RALT);
                }
                register_code16(KC_J);
            } else {
                unregister_code16(KC_J);
                if (!is_lt4_on) {
                    unregister_code16(KC_RALT);
                }
            }
            return false;
        }
        case MC_K: {
            if (record->event.pressed) {
                if (is_lt4_on) {
                    register_code16(KC_RALT);
                }
                register_code16(KC_K);
            } else {
                unregister_code16(KC_K);
                if (!is_lt4_on) {
                    unregister_code16(KC_RALT);
                }
            }
            return false;
        }
        case MC_TAB: {
            if (record->event.pressed) {
                if (is_lt4_on) {
                    register_code16(KC_RALT);
                }
                register_code16(KC_TAB);
            } else {
                unregister_code16(KC_TAB);
                if (!is_lt4_on) {
                    unregister_code16(KC_RALT);
                }
            }
            return false;
        }
        case MC_STAB: {
            if (record->event.pressed) {
                if (is_lt4_on) {
                    register_code16(KC_RALT);
                }
                register_code16(S(KC_TAB));
            } else {
                unregister_code16(S(KC_TAB));
                if (!is_lt4_on) {
                    unregister_code16(KC_RALT);
                }
            }
            return false;
        }

        // Tmuxのプレフィックス
        case MC_TMUX: {
            if (record->event.pressed) {
                if (os_name == OS_LINUX) {
                    tap_code16(RALT(KC_SPACE));
                } else {
                    tap_code16(RALT(KC_B));
                }
            }
            return false;  // キーのデフォルトの動作をスキップする
        }
        // Tmuxのコピーモード
        case MC_TMCP: {
            if (record->event.pressed) {
                if (os_name == OS_LINUX) {
                    tap_code16(RALT(KC_SPACE));
                } else {
                    tap_code16(RALT(KC_B));
                }
                tap_code16(KC_SPACE);
            }
            return false;  // キーのデフォルトの動作をスキップする
        }

        // CTRL+K
        case MC_ALKL: {
            if (record->event.pressed) {
                register_code16(KC_LSFT);
                tap_code16(KC_END);
                tap_code16(KC_DELETE);
            } else {
                unregister_code16(KC_LSFT);
            }
            return false;  // キーのデフォルトの動作をスキップする
        }

        // CTRL+U
        case MC_BLKL: {
            if (record->event.pressed) {
                register_code16(KC_LSFT);
                tap_code16(KC_HOME);
                tap_code16(KC_DELETE);
            } else {
                unregister_code16(KC_LSFT);
            }
            return false;  // キーのデフォルトの動作をスキップする
        }

        // 現在日付時刻
        case MC_DATE: {
            if (record->event.pressed) {
                uint8_t report[32] = {0};
                report[0] = DATETIME_UPDATE;

                raw_hid_send(report, sizeof(report));
            } else {
                return false;
            }
            return false;  // キーのデフォルトの動作をスキップする
        }

        // windows切り替え用
        case ALT_TAB: {
            if (record->event.pressed) {
                if (!is_alt_tab_active) {
                    is_alt_tab_active = true;
                    register_code(KC_LALT);
                }
                alt_tab_timer = timer_read();
                register_code(KC_TAB);
            } else {
                unregister_code(KC_TAB);
            }
            break;
        }

        // 自動クリックレイヤーではESCは解除キーとして扱う
        case MC_ESC: {
            if (record->event.pressed) {
                is_single_tap = true;
                hold_ctrl = true;
                register_code(KC_LCTL);
            } else {
                hold_ctrl = false;
                unregister_code(KC_LCTL);
                if(is_single_tap) {
                    selection_mode_stop();
                    if (click_layer && get_highest_layer(layer_state) == click_layer) {
                        disable_click_layer_all_state();
                    } else {
                        if (os_name == OS_MACOS) {
                            tap_code16(KC_LNG2);
                        } else {
                            tap_code16(KC_INT5);
                        }
                        if (!is_ime_on) {
                            tap_code16(KC_ESC);
                        }
                        is_ime_on = false;
                    }
                }
            }

            return false;
        }

        case MC_LSFT: {
            if (record->event.pressed) {
                hold_lsft = true;
                if (hold_rsft) {
                    unregister_code(KC_LSFT);
                    layer_on(_NUM);
                } else {
                    register_code(KC_LSFT);
                }
            } else {
                hold_lsft = false;
                layer_off(_NUM);
                unregister_code(KC_LSFT);
            }

            return false;
        }

        case MC_RSFT: {
            if (record->event.pressed) {
                hold_rsft = true;
                if (hold_lsft) {
                    unregister_code(KC_LSFT);
                    layer_on(_NUM);
                } else {
                    register_code(KC_LSFT);
                }
            } else {
                hold_rsft = false;
                layer_off(_NUM);
                unregister_code(KC_LSFT);
            }

            return false;
        }

        case KC_RGUI: {
            if (record->event.pressed) {
                is_ime_on = true;
                disable_click_layer();
            }

            if (os_name == OS_MACOS) {
                tap_code16(KC_LNG1);

                return false;
            }

            return true;
        }

        // 水平に固定してスクロール
        case SCRL_HO: {
            if (click_layer && get_highest_layer(layer_state) == click_layer) {
                keyball_set_scroll_mode(record->event.pressed);
                if (record->event.pressed) {
                    horizontal_flag = 1;
                    state = CLICKING;
                } else {
                    horizontal_flag = 0;
                    state = CLICKED;
                }
                return false;
            }
        }
        // 垂直に固定してスクロール
        case SCRL_VR: {
            if (click_layer && get_highest_layer(layer_state) == click_layer) {
                keyball_set_scroll_mode(record->event.pressed);
                if (record->event.pressed) {
                    horizontal_flag = 2;
                    state = CLICKING;
                } else {
                    horizontal_flag = 0;
                    state = CLICKED;
                }
                return false;
            }
        }

        // vivaldiのタブサイクルをワンキーで
        case SCRL_TB: {
            if (click_layer && get_highest_layer(layer_state) == click_layer) {
                keyball_set_scroll_mode(record->event.pressed);
                if (record->event.pressed) {
                    keyball_set_cpi(4);
                    horizontal_flag = 1;
                    register_code(MS_BTN2);
                    state = CLICKING;
                } else {
                    keyball_set_cpi(KEYBALL_CPI_DEFAULT / 100);
                    horizontal_flag = 0;
                    unregister_code(MS_BTN2);
                    state = CLICKED;
                }
                return false;
            }
        }

        // windowsでトラボでタスクスイッチするやつ
        case SCRL_WD: {
            if (click_layer && get_highest_layer(layer_state) == click_layer) {
                keyball_set_scroll_mode(record->event.pressed);
                if (record->event.pressed) {
                    scroll_convert_flag = true;
                    horizontal_flag = 1;
                    keyball_set_cpi(2);
                    register_code(KC_RALT);
                    state = CLICKING;
                } else {
                    scroll_convert_flag = false;
                    horizontal_flag = 0;
                    keyball_set_cpi(KEYBALL_CPI_DEFAULT / 100);
                    unregister_code(KC_RALT);
                    state = CLICKED;
                }
                return false;
            }
        }

        case KC_OG_BTN1:
        case KC_OG_BTN2:
        case KC_OG_BTN3:
        case KC_OG_BTN4:
        case KC_OG_BTN5: {
            if (click_layer && get_highest_layer(layer_state) == click_layer) {
                if (record->event.pressed) {
                    // キーダウン時: 状態をCLICKINGに設定
                    state = CLICKING;
                    mouse_button_func(keycode, true);
                } else {
                    // キーアップ時: クリックレイヤーを有効にして、状態をCLICKEDに設定
                    enable_click_layer();
                    state = CLICKED;
                    mouse_button_func(keycode, false);
                }
                return false;
            } else {
                if (!record->event.pressed) {
                    unregister_code(keycode);

                    if(keycode == KC_OG_BTN5) {
                        unregister_code(KC_LCTL);
                    }
                    if(keycode == KC_OG_BTN4) {
                        unregister_code(KC_RCTL);
                    }
                }
            }
            return true;
        }

        case KC_DBLB:
        case KC_TRPB: {
            if (click_layer && get_highest_layer(layer_state) == click_layer) {
                if (record->event.pressed) {
                    // キーダウン時
                    state = CLICKING;
                    // `KC_DBLB`の場合
                    if (keycode == KC_DBLB) {
                        double_click_mouse_button1();  // マウスボタン1をダブルクリック
                    }
                    // `KC_TRPB`の場合
                    if (keycode == KC_TRPB) {
                        triple_click_mouse_button1();  // マウスボタン1をトリプルクリック
                    }
                } else {
                    // キーアップ時: クリックレイヤーを有効にして、状態をCLICKEDに設定
                    enable_click_layer();
                    state = CLICKED;
                }
                return false;  // キーのデフォルトの動作をスキップする
            } else {
                return true;
            }
        }
        case AC_KEP: {
            if (click_layer && get_highest_layer(layer_state) == click_layer) {
                if (record->event.pressed) {
                    keep_click_layer = true;
                }
            }
            return false;
        }
        case AC_INS: {
            if (click_layer && get_highest_layer(layer_state) == click_layer) {
                if (record->event.pressed) {
                    tap_code16(LCTL(S(KC_C)));
                }
            }
            return false;
        }

        case NG_MON: {
            if (record->event.pressed) {
                naginata_on();
            }
            return false;
        }

        case NG_MOFF: {
            if (record->event.pressed) {
                naginata_off();
            }
            return false;
        }

        // debug key
        case MC_APPN: {
            if (record->event.pressed && application_name[0] != '\0' ) {
                send_string(application_name);
            }
            return false;
        }
        // debug key
        case MC_OSNE: {
            if (os_name == OS_WINDOWS) {
                send_string("win");
            } else if (os_name == OS_MACOS) {
                send_string("mac");
            } else {
                send_string("other");
            }

            return false;
        }
    }

    if (naginata_state()) {
        if (!process_naginata(keycode, record)) return false;
    }

    disable_click_layer_all_state();
    return true;
}

void matrix_scan_user(void) {
    one_tap_multi_click_task();

    if (is_alt_tab_active) {
        if (timer_elapsed(alt_tab_timer) > 1000) {
            unregister_code(KC_LALT);
            is_alt_tab_active = false;
        }
    }
}
