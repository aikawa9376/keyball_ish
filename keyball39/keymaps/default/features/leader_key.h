void leader_end_user(void) {
    if (leader_sequence_one_key(KC_F)) {
        tap_code16(KC_SPACE);
        tap_code16(KC_F);
    } else if (leader_sequence_one_key(KC_A)) {
        tap_code16(KC_SPACE);
        tap_code16(KC_A);
    } else if (leader_sequence_one_key(KC_W)) {
        tap_code16(KC_SPACE);
        tap_code16(KC_W);
    } else if (leader_sequence_one_key(KC_E)) {
        tap_code16(KC_SPACE);
        tap_code16(KC_E);
    } else if (leader_sequence_one_key(KC_B)) {
        tap_code16(KC_SPACE);
        tap_code16(KC_B);
    } else if (leader_sequence_one_key(KC_N)) {
        tap_code16(KC_SPACE);
        tap_code16(KC_N);
    } else if (leader_sequence_one_key(KC_O)) {
        tap_code16(KC_SPACE);
        tap_code16(KC_O);
    } else if (leader_sequence_one_key(KC_C)) {
        tap_code16(KC_SPACE);
        tap_code16(KC_C);
    } else if (leader_sequence_one_key(KC_L)) {
        tap_code16(KC_SPACE);
        tap_code16(KC_L);
    } else if (leader_sequence_one_key(KC_SCLN)) {
        tap_code16(KC_SPACE);
        tap_code16(KC_SCLN);
    } else {
        tap_code16(KC_SCLN);
    }
}
