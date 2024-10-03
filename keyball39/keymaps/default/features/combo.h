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
    CM_34,
    CE_VF,
    CE_VE,
    CE_VA,
    CE_VW,
    CE_VB,
    CE_VR,
    CE_VL,
    CE_VG,
    CE_VN,
    CE_VO,
    CE_VC,
    CE_VX,
    CE_VQ,
    CE_VBL,
    CE_VLF,
    CE_VLR,
    CE_VLL,
    CE_VLA,
    CE_VLE,
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
const uint16_t PROGMEM numet_combo[] = {KC_3, KC_4, COMBO_END};

// vim leader key simulator
const uint16_t PROGMEM vimlf_combo[] = {LT(SM, KC_SCLN), LT(UT, KC_F), COMBO_END};
const uint16_t PROGMEM vimle_combo[] = {LT(SM, KC_SCLN), KC_E, COMBO_END};
const uint16_t PROGMEM vimla_combo[] = {LT(SM, KC_SCLN), KC_A, COMBO_END};
const uint16_t PROGMEM vimlw_combo[] = {LT(SM, KC_SCLN), KC_W, COMBO_END};
const uint16_t PROGMEM vimlb_combo[] = {LT(SM, KC_SCLN), KC_B, COMBO_END};
const uint16_t PROGMEM vimlr_combo[] = {LT(SM, KC_SCLN), KC_R, COMBO_END};
const uint16_t PROGMEM vimll_combo[] = {LT(SM, KC_SCLN), KC_L, COMBO_END};
const uint16_t PROGMEM vimlg_combo[] = {LT(SM, KC_SCLN), KC_G, COMBO_END};
const uint16_t PROGMEM vimln_combo[] = {LT(SM, KC_SCLN), KC_N, COMBO_END};
const uint16_t PROGMEM vimlo_combo[] = {LT(SM, KC_SCLN), KC_O, COMBO_END};
const uint16_t PROGMEM vimlc_combo[] = {LT(SM, KC_SCLN), KC_C, COMBO_END};
const uint16_t PROGMEM vimlx_combo[] = {LT(SM, KC_SCLN), KC_X, COMBO_END};
const uint16_t PROGMEM vimlq_combo[] = {LT(SM, KC_SCLN), KC_Q, COMBO_END};
const uint16_t PROGMEM vimlbl_combo[] = {LT(SM, KC_SCLN), MC_ESC, COMBO_END};
const uint16_t PROGMEM vimllf_combo[] = {LT(SM, KC_SCLN), S(KC_F), COMBO_END};
const uint16_t PROGMEM vimllr_combo[] = {LT(SM, KC_SCLN), S(KC_R), COMBO_END};
const uint16_t PROGMEM vimlll_combo[] = {LT(SM, KC_SCLN), S(KC_L), COMBO_END};
const uint16_t PROGMEM vimlla_combo[] = {LT(SM, KC_SCLN), S(KC_A), COMBO_END};
const uint16_t PROGMEM vimlle_combo[] = {LT(SM, KC_SCLN), S(KC_E), COMBO_END};

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
    [CM_ES] = COMBO(escape_combo, MC_ESC),
    [CM_34] = COMBO(numet_combo, KC_ENTER),
    [CE_VF] = COMBO_ACTION(vimlf_combo),
    [CE_VE] = COMBO_ACTION(vimle_combo),
    [CE_VA] = COMBO_ACTION(vimla_combo),
    [CE_VW] = COMBO_ACTION(vimlw_combo),
    [CE_VB] = COMBO_ACTION(vimlb_combo),
    [CE_VR] = COMBO_ACTION(vimlr_combo),
    [CE_VL] = COMBO_ACTION(vimll_combo),
    [CE_VG] = COMBO_ACTION(vimlg_combo),
    [CE_VN] = COMBO_ACTION(vimln_combo),
    [CE_VO] = COMBO_ACTION(vimlo_combo),
    [CE_VC] = COMBO_ACTION(vimlc_combo),
    [CE_VX] = COMBO_ACTION(vimlx_combo),
    [CE_VQ] = COMBO_ACTION(vimlq_combo),
    [CE_VBL] = COMBO_ACTION(vimlbl_combo),
    [CE_VLF] = COMBO_ACTION(vimllf_combo),
    [CE_VLR] = COMBO_ACTION(vimllr_combo),
    [CE_VLL] = COMBO_ACTION(vimlll_combo),
    [CE_VLA] = COMBO_ACTION(vimlla_combo),
    [CE_VLE] = COMBO_ACTION(vimlle_combo),
};

void process_combo_event(uint16_t combo_index, bool pressed) {
  switch(combo_index) {
    case CE_VF:
      if (pressed) {
        tap_code16(KC_SPACE);
        tap_code16(KC_F);
      }
      break;
    case CE_VE:
      if (pressed) {
        tap_code16(KC_SPACE);
        tap_code16(KC_E);
      }
      break;
    case CE_VA:
      if (pressed) {
        tap_code16(KC_SPACE);
        tap_code16(KC_A);
      }
      break;
    case CE_VW:
      if (pressed) {
        tap_code16(KC_SPACE);
        tap_code16(KC_W);
      }
      break;
    case CE_VB:
      if (pressed) {
        tap_code16(KC_SPACE);
        tap_code16(KC_B);
      }
      break;
    case CE_VR:
      if (pressed) {
        tap_code16(KC_SPACE);
        tap_code16(KC_R);
      }
      break;
    case CE_VL:
      if (pressed) {
        tap_code16(KC_SPACE);
        tap_code16(KC_L);
      }
      break;
    case CE_VG:
      if (pressed) {
        tap_code16(KC_SPACE);
        tap_code16(KC_G);
      }
      break;
    case CE_VN:
      if (pressed) {
        tap_code16(KC_SPACE);
        tap_code16(KC_N);
      }
      break;
    case CE_VO:
      if (pressed) {
        tap_code16(KC_SPACE);
        tap_code16(KC_O);
      }
      break;
    case CE_VC:
      if (pressed) {
        if (
            strcmp(application_name, "kitty") == 0 ||
            strcmp(application_name, "WindowsTerminal") == 0
        ) {
            tap_code16(KC_SPACE);
            tap_code16(KC_C);
        } else {
            tap_code16(C(KC_C));
        }
      }
      break;
    case CE_VX:
      if (pressed) {
        tap_code16(KC_SPACE);
        tap_code16(KC_X);
      }
      break;
    case CE_VQ:
      if (pressed) {
        tap_code16(KC_SPACE);
        tap_code16(KC_Q);
      }
      break;
    case CE_VBL:
      if (pressed) {
        tap_code16(KC_SPACE);
        tap_code16(KC_RBRC);
      }
      break;
    case CE_VLF:
      if (pressed) {
        tap_code16(KC_SPACE);
        tap_code16(S(KC_F));
      }
      break;
    case CE_VLR:
      if (pressed) {
        tap_code16(KC_SPACE);
        tap_code16(S(KC_R));
      }
      break;
    case CE_VLL:
      if (pressed) {
        tap_code16(KC_SPACE);
        tap_code16(S(KC_L));
      }
      break;
    case CE_VLA:
      if (pressed) {
        tap_code16(KC_SPACE);
        tap_code16(S(KC_A));
      }
      break;
    case CE_VLE:
      if (pressed) {
        tap_code16(KC_SPACE);
        tap_code16(S(KC_A));
      }
      break;
  }
}

uint16_t get_combo_term(uint16_t index, combo_t *combo) {
    switch (combo->keys[0]) {
        case LT(SM, KC_SCLN):
            return 300;
        default:
            return COMBO_TERM;
    }
}

bool get_combo_must_press_in_order(uint16_t combo_index, combo_t *combo) {
    switch (combo_index) {
        case CE_VF:
        case CE_VE:
        case CE_VA:
        case CE_VW:
        case CE_VB:
        case CE_VR:
        case CE_VL:
        case CE_VG:
        case CE_VN:
        case CE_VO:
        case CE_VC:
        case CE_VX:
        case CE_VQ:
        case CE_VBL:
        case CE_VLF:
        case CE_VLR:
        case CE_VLL:
        case CE_VLA:
        case CE_VLE:
            return true;
        default:
            return false;
    }
}
