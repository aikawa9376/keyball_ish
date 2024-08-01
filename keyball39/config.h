/*
Copyright 2022 @Yowkees
Copyright 2022 MURAOKA Taro (aka KoRoN, @kaoriya)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

// Key matrix parameters
// #define MATRIX_ROWS         (4 * 2)  // split keyboard
// #define MATRIX_COLS         6
// #define MATRIX_MASKED

#undef RP_SPI_USE_SPI0
#define RP_SPI_USE_SPI0 TRUE

//* define RP2040 boot用 */
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET_LED GP17
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET_TIMEOUT 1000U

/* SPI & PMW3360 settings. */
#define SPI_DRIVER SPID0
#define SPI_SCK_PIN GP22
#define SPI_MISO_PIN GP20
#define SPI_MOSI_PIN GP23
#define POINTING_DEVICE_CS_PIN GP21
#define PMW33XX_CS_PIN GP21
#define PMW3360_CS_PIN GP21

// #define POINTING_DEVICE_INVERT_X
// #define POINTING_DEVICE_ROTATION_90

// Split parameters
#define SPLIT_HAND_MATRIX_GRID  GP27, GP9
#define SPLIT_USB_DETECT
#define SPLIT_USB_TIMEOUT       3000
#define SPLIT_HAND_MATRIX_GRID_LOW_IS_LEFT

#define SPLIT_TRANSACTION_IDS_KB KEYBALL_GET_INFO, KEYBALL_GET_MOTION, KEYBALL_SET_CPI

#ifndef OLED_FONT_H
#    define OLED_FONT_H "keyboards/keyball/lib/logofont/logofont.c"
#    define OLED_FONT_START 32
#    define OLED_FONT_END 195
#endif

#if !defined(LAYER_STATE_8BIT) && !defined(LAYER_STATE_16BIT) && !defined(LAYER_STATE_32BIT)
#    define LAYER_STATE_8BIT
#endif
