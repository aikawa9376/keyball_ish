//Tap Dance Declarations
enum {
  TD_QUOT,
  TD_UNDS,
  TD_LPRN,
  TD_LBRC,
  TD_LCBR,
};

//Tap Dance Definitions
tap_dance_action_t tap_dance_actions[] = {
  //Tap once for Esc, twice for Caps Lock
  [TD_QUOT]  = ACTION_TAP_DANCE_DOUBLE(KC_QUOT, KC_GRV),
  [TD_UNDS]  = ACTION_TAP_DANCE_DOUBLE(KC_UNDS, KC_MINS),
  [TD_LPRN]  = ACTION_TAP_DANCE_DOUBLE(KC_LPRN, KC_QUOT),
  [TD_LBRC]  = ACTION_TAP_DANCE_DOUBLE(KC_LBRC, KC_DQT),
  [TD_LCBR]  = ACTION_TAP_DANCE_DOUBLE(KC_LCBR, KC_GRV),
};
