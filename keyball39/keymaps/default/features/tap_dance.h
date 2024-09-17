//Tap Dance Declarations
enum {
  TD_QUOT,
  TD_UNDS,
  TD_RPRN,
  TD_RBRC,
  TD_RCBR,
  TD_PIPE,
  TD_0,
};

//Tap Dance Definitions
tap_dance_action_t tap_dance_actions[] = {
  //Tap once for Esc, twice for Caps Lock
  [TD_QUOT]  = ACTION_TAP_DANCE_DOUBLE(KC_QUOT, KC_GRV),
  [TD_UNDS]  = ACTION_TAP_DANCE_DOUBLE(KC_UNDS, KC_MINS),
  [TD_RPRN]  = ACTION_TAP_DANCE_DOUBLE(KC_RPRN, KC_QUOT),
  [TD_RBRC]  = ACTION_TAP_DANCE_DOUBLE(KC_RBRC, KC_DQT),
  [TD_RCBR]  = ACTION_TAP_DANCE_DOUBLE(KC_RCBR, KC_GRV),
  [TD_PIPE]  = ACTION_TAP_DANCE_DOUBLE(KC_PIPE, KC_EQL),
  [TD_0]  = ACTION_TAP_DANCE_DOUBLE(KC_0, KC_COLN),
};
