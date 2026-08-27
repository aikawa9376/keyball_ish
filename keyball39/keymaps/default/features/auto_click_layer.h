#pragma once

#include <stdbool.h>
#include <stdint.h>

enum ball_state {
    NONE = 0,
    WAITING,
    CLICKABLE,
    CLICKING,
    CLICKED,
};

extern enum ball_state state;
extern const uint16_t click_layer;
extern bool keep_click_layer;
extern bool scroll_convert_flag;

void enable_click_layer(void);
void disable_click_layer(void);
