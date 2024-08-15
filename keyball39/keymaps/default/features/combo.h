/* Copyright 2023 kamidai (@d_kamiichi)
 *
 * 参考ページ
 * 公式：https://docs.qmk.fm/#/ja/feature_combo?id=%e3%82%b3%e3%83%b3%e3%83%9c
 *
 * 注意： 以下の設定が必要です
 * rules.mk: COMBO_ENABLE = yes
 * config.h: #define COMBO_COUNT 2（=実際に設定するコンボ数）
 */

enum combos {
    CM_JK,
    CM_DF,
    CM_CV,
    CM_MV,
    CM_MS,
    CM_LC,
    CM_RC,
    CM_TG,
    CM_AP,
    CM_AQ,
    CM_ES,
};

const uint16_t PROGMEM space_combo[] = {KC_J, KC_K, COMBO_END};
const uint16_t PROGMEM enter_combo[] = {KC_D, LT(UT, KC_F), COMBO_END};
const uint16_t PROGMEM imeon_combo[] = {KC_C, KC_V, COMBO_END};
const uint16_t PROGMEM escape_combo[] = {LT(UT, KC_F), KC_J, COMBO_END};
// const uint16_t PROGMEM rctrl_combo[] = {KC_J, KC_K, KC_L, COMBO_END};
const uint16_t PROGMEM rctrl_combo[] = {KC_J, KC_K, KC_L, COMBO_END};
const uint16_t PROGMEM lctrl_combo[] = {KC_S, KC_D, LT(UT, KC_F), COMBO_END};
const uint16_t PROGMEM spacem_combo[] = {KC_OG_BTN1, KC_OG_BTN2, COMBO_END};
const uint16_t PROGMEM imeonm_combo[] = {KC_OG_BTN4, KC_OG_BTN5, COMBO_END};

// test
const uint16_t PROGMEM stogg_combo[] = {KC_Z, KC_SLSH, COMBO_END};
const uint16_t PROGMEM altsp_combo[] = {KC_K, KC_L, COMBO_END};
const uint16_t PROGMEM altsq_combo[] = {KC_D, KC_S, COMBO_END};

combo_t key_combos[] = {
    [CM_JK] = COMBO(space_combo, LT(SM, KC_SPACE)),
    [CM_DF] = COMBO(enter_combo, LT(SM, KC_ENTER)),
    [CM_CV] = COMBO(imeon_combo, KC_RGUI),
    [CM_LC] = COMBO(rctrl_combo, MO(MV)),
    [CM_RC] = COMBO(lctrl_combo, MO(MV)),
    [CM_TG] = COMBO(stogg_combo, CW_TOGG),
    [CM_MV] = COMBO(imeonm_combo, KC_RGUI),
    [CM_MS] = COMBO(spacem_combo, KC_SPACE),
    [CM_AP] = COMBO(altsp_combo, KC_P),
    [CM_AQ] = COMBO(altsq_combo, KC_Q),
    [CM_ES] = COMBO(escape_combo, MC_ESC),
};
