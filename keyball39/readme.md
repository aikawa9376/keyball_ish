# Keyball39

-- TODO --
- レイヤーに名前をつける
- raw HID機能を開発

## Pointing-device tuning

The Keyballish pointing path uses a 2 ms report interval, extended mouse and
wheel reports, and QMK high-resolution scrolling. Cursor acceleration is a
continuous velocity-based curve with fractional output carry; adjust the
`KEYBALL_ACCEL_*` options in `lib/keyball/keyball.h` if the initial curve is
too slow or too fast.

The scroll axis lock is enabled by default for high-resolution scrolling and
is released after 100 ms without motion. Set
`KEYBALL_SCROLL_AXIS_LOCK_ENABLE` to `0` when diagonal scrolling is preferred.

`KEYBALL_PMW3360_UPLOAD_SROM_ID` is intentionally opt-in. Only enable it after
confirming the matching SROM (`0x04` or `0x81`) for the installed sensor module;
the driver logs the detected ID when console output is enabled.
