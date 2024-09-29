uint16_t get_tapping_term(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case SCRL_HO:
        case SCRL_VR:
            return 0;
        case TD(TD_RBRC):
        case TD(TD_0):
            return 130;
        case TD(TD_PIPE):
        case TD(TD_RPRN):
        case TD(TD_QUOT):
        case TD(TD_RCBR):
            return 170;
        case TD(TD_UNDS):
            return 200;
        default:
            return TAPPING_TERM;
    }
}

// 基本はこれをオンにする
bool get_hold_on_other_key_press(uint16_t keycode, keyrecord_t *record) {
  switch (keycode) {
    case LT(UT, KC_F):
    // case LT(SM, KC_SPACE):
    case SFT_T(KC_SPC):
      return false;
    default:
      // Do not select the hold action when another key is pressed.
      return true;
  }
}

bool get_permissive_hold(uint16_t keycode, keyrecord_t *record) {
  switch (keycode) {
    case LT(UT, KC_F):
    // case LT(SM, KC_SPACE):
    case SFT_T(KC_SPC):
      // Immediately select the hold action when another key is tapped.
      return true;
    default:
      // Do not select the hold action when another key is tapped.
      return false;
  }
}
