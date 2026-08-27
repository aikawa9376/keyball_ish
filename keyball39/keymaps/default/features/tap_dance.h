#pragma once

#include "process_tap_dance.h"

enum {
    TD_QUOT,
    TD_UNDS,
    TD_RPRN,
    TD_RBRC,
    TD_RCBR,
    TD_PIPE,
    TD_0,
    TD_COUNT,
};

extern tap_dance_action_t tap_dance_actions[TD_COUNT];
