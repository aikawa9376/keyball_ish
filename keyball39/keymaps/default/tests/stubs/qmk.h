#pragma once
#include <stdbool.h>
#include <stdint.h>
typedef enum { OS_UNSURE, OS_LINUX, OS_WINDOWS, OS_MACOS } os_variant_t;
#define KC_V 0x19
#define C(key) ((key) | 0x0100)
#define S(key) ((key) | 0x0200)
#define G(key) ((key) | 0x0800)
uint16_t timer_read(void);
uint16_t timer_elapsed(uint16_t started);
uint8_t get_mods(void);
uint8_t get_weak_mods(void);
void tap_code16(uint16_t keycode);
