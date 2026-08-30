#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    SELECTION_MODE_NONE,
    SELECTION_MODE_KEYBOARD,
    SELECTION_MODE_MOUSE,
} selection_mode_t;

selection_mode_t selection_mode_get(void);
bool             selection_mode_is_active(void);
bool             selection_mode_should_stop(uint16_t keycode);
void             selection_mode_start(bool mouse_mode);
void             selection_mode_stop(void);
void             selection_mode_toggle(bool mouse_mode);
