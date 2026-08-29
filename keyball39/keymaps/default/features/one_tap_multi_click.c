#include QMK_KEYBOARD_H

#include "mouse_buttons.h"
#include "one_tap_multi_click.h"

#ifndef KEYBALL_MULTI_CLICK_INTERVAL
#    define KEYBALL_MULTI_CLICK_INTERVAL 30
#endif

typedef enum {
    CLICK_SEQUENCE_IDLE,
    CLICK_SEQUENCE_PRESSED,
    CLICK_SEQUENCE_RELEASED,
} click_sequence_phase_t;

static click_sequence_phase_t click_sequence_phase = CLICK_SEQUENCE_IDLE;
static uint8_t                click_sequence_remaining;
static uint16_t               click_sequence_timer;

static void start_click_sequence(uint8_t click_count) {
    // Restarting a sequence must not leave the previous synthetic press held.
    if (click_sequence_phase == CLICK_SEQUENCE_PRESSED) {
        release_mouse_button1();
    }

    click_sequence_remaining = click_count;
    click_sequence_phase     = CLICK_SEQUENCE_PRESSED;
    click_sequence_timer     = timer_read();
    click_mouse_button1();
}

void one_tap_multi_click_task(void) {
    if (click_sequence_phase == CLICK_SEQUENCE_IDLE || timer_elapsed(click_sequence_timer) < KEYBALL_MULTI_CLICK_INTERVAL) {
        return;
    }

    click_sequence_timer = timer_read();

    if (click_sequence_phase == CLICK_SEQUENCE_PRESSED) {
        release_mouse_button1();
        if (--click_sequence_remaining == 0) {
            click_sequence_phase = CLICK_SEQUENCE_IDLE;
        } else {
            click_sequence_phase = CLICK_SEQUENCE_RELEASED;
        }
    } else {
        click_mouse_button1();
        click_sequence_phase = CLICK_SEQUENCE_PRESSED;
    }
}

void double_click_mouse_button1(void) {
    start_click_sequence(2);
}

void triple_click_mouse_button1(void) {
    start_click_sequence(3);
}
