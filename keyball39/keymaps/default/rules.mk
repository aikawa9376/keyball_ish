RGBLIGHT_ENABLE = false

OLED_ENABLE = yes
COMBO_ENABLE = yes
CAPS_WORD_ENABLE = yes
TAP_DANCE_ENABLE = yes
UNICODE_ENABLE = yes
OS_DETECTION_ENABLE = yes
RAW_ENABLE = yes
KEY_OVERRIDE_ENABLE = yes
LEADER_ENABLE = yes

SRC += lib/naginata/naginata_v15.c

SRC += \
    features/auto_click_layer.c \
    features/combo.c \
    features/leader_key.c \
    features/macro_keys.c \
    features/mouse_buttons.c \
    features/oled.c \
    features/one_tap_multi_click.c \
    features/os_detection.c \
    features/overrides.c \
    features/raw_hid_handler.c \
    features/selection_mode.c \
    features/swipe_gesture.c \
    features/tap_dance.c \
    features/tap_hold.c \
    utils/functions.c
