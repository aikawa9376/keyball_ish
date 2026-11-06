#pragma once

#include <stdbool.h>
#include <stdint.h>

// Shared with rawhid-rust/src/protocol.rs. All offsets refer to QMK's 32 bytes.
#define CLIPBOARD_MAX_CHARS 100
#define CLIPBOARD_MAX_BYTES (CLIPBOARD_MAX_CHARS * 4)
#define CLIPBOARD_CHUNK_BYTES 27
#define CLIPBOARD_STORE_REQUEST 0x10
#define CLIPBOARD_STORE_BEGIN 0x11
#define CLIPBOARD_STORE_DATA 0x12
#define CLIPBOARD_STORE_COMMIT 0x13
#define CLIPBOARD_STORE_STATUS 0x14
#define CLIPBOARD_PASTE_BEGIN 0x15
#define CLIPBOARD_PASTE_DATA 0x16
#define CLIPBOARD_PASTE_COMMIT 0x17
#define CLIPBOARD_PASTE_READY 0x18

void clipboard_hid_store(void);
void clipboard_hid_paste(bool pressed);
void clipboard_hid_cancel_paste(void);
void clipboard_hid_receive(const uint8_t *data, uint8_t length);
void clipboard_hid_task(void);
