uint16_t get_tapping_term(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case SCRL_HO:
        case SCRL_VR:
            return 0;
        default:
            return TAPPING_TERM;
    }
}

// 基本はこれをオンにする
bool get_hold_on_other_key_press(uint16_t keycode, keyrecord_t *record) {
  switch (keycode) {
    case LT(4, KC_F):
    case SFT_T(KC_SPC):
      return false;
    default:
      // Do not select the hold action when another key is pressed.
      return true;
  }
}

bool get_permissive_hold(uint16_t keycode, keyrecord_t *record) {
  switch (keycode) {
    case LT(4, KC_F):
    case SFT_T(KC_SPC):
      // Immediately select the hold action when another key is tapped.
      return true;
    default:
      // Do not select the hold action when another key is tapped.
      return false;
  }
}

uint16_t get_quick_tap_term(uint16_t keycode, keyrecord_t *record) {
  switch (keycode) {
    case LCTL_T(KC_ESC):
      return 0;
    default:
      return QUICK_TAP_TERM;
  }
}
