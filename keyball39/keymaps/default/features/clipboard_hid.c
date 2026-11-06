#include QMK_KEYBOARD_H
#include <string.h>
#include "raw_hid.h"
#include "usb_device_state.h"
#include "clipboard_hid.h"
#include "clipboard_storage.h"
#include "os_detection.h"
#include "raw_hid_handler.h"

#define CLIPBOARD_TIMEOUT 3000
#define CLIPBOARD_PACKET_INTERVAL 4

enum clipboard_state { IDLE, STORE_REQUESTED, RECEIVING, PASTE_BEGIN, PASTE_DATA, PASTE_END, WAIT_READY, PASTE_KEY };
static enum clipboard_state state;
static uint8_t token;
static uint16_t started, last_packet, text_length, transfer_length, offset;
static bool has_text, paste_pressed;
static volatile bool usb_cancel;
static char stored_text[CLIPBOARD_MAX_BYTES + 1];
static char incoming_text[CLIPBOARD_MAX_BYTES + 1];

static uint16_t read_u16(const uint8_t *data) {
    return data[0] | ((uint16_t)data[1] << 8);
}

static void cancel_transfer(void) {
    state = IDLE;
    offset = 0;
    memset(incoming_text, 0, sizeof(incoming_text));
}

// Validate scalars, including overlong sequences and surrogate exclusions.
static bool valid_text(const uint8_t *text, uint16_t length) {
    uint16_t position = 0, count = 0;
    while (position < length) {
        uint8_t first = text[position++], remaining;
        uint32_t codepoint, minimum;
        if (first > 0 && first < 0x80) {
            codepoint = first;
            minimum = 0;
            remaining = 0;
        } else if (first >= 0xc2 && first <= 0xdf) {
            codepoint = first & 0x1f;
            minimum = 0x80;
            remaining = 1;
        } else if (first >= 0xe0 && first <= 0xef) {
            codepoint = first & 0x0f;
            minimum = 0x800;
            remaining = 2;
        } else if (first >= 0xf0 && first <= 0xf4) {
            codepoint = first & 0x07;
            minimum = 0x10000;
            remaining = 3;
        } else {
            return false;
        }
        while (remaining--) {
            if (position >= length || (text[position] & 0xc0) != 0x80) return false;
            codepoint = (codepoint << 6) | (text[position++] & 0x3f);
        }
        if (codepoint < minimum || codepoint > 0x10ffff || (codepoint >= 0xd800 && codepoint <= 0xdfff) || ++count > CLIPBOARD_MAX_CHARS) return false;
    }
    return true;
}

void keyboard_post_init_user(void) {
    cancel_transfer();
    token = 0;
    paste_pressed = false;
    usb_cancel = false;
    text_length = 0;
    memset(stored_text, 0, sizeof(stored_text));
    has_text = clipboard_storage_load(stored_text, &text_length) && valid_text((const uint8_t *)stored_text, text_length);
    if (!has_text) {
        text_length = 0;
        memset(stored_text, 0, sizeof(stored_text));
    }
}

static void send_status(uint8_t status) {
    uint8_t report[32] = {CLIPBOARD_STORE_STATUS, token, status};
    raw_hid_send(report, sizeof(report));
}

static bool terminal_application(void) {
    const char *names[] = {"kitty", "Alacritty", "alacritty", "wezterm", "org.wezfurlong.wezterm", "foot", "footclient", "org.gnome.Terminal", "gnome-terminal-server", "konsole", "xfce4-terminal", "XTerm", "xterm", "Windows Terminal Host"};
    for (uint8_t i = 0; i < sizeof(names) / sizeof(names[0]); ++i) {
        if (strcmp(application_name, names[i]) == 0) return true;
    }
    return false;
}

void clipboard_hid_store(void) {
    // An explicit new store cancels an older operation, preserving the saved slot.
    cancel_transfer();
    usb_cancel = false;
    ++token;
    state = STORE_REQUESTED;
    started = timer_read();
    uint8_t report[32] = {CLIPBOARD_STORE_REQUEST, token};
    raw_hid_send(report, sizeof(report));
}

void clipboard_hid_paste(bool pressed) {
    paste_pressed = pressed;
    if (!pressed || !has_text) return;
    cancel_transfer();
    usb_cancel = false;
    ++token;
    state = PASTE_BEGIN;
    started = timer_read();
    last_packet = started;
}

void clipboard_hid_cancel_paste(void) {
    if (state >= PASTE_BEGIN) cancel_transfer();
}

void clipboard_hid_receive(const uint8_t *data, uint8_t length) {
    if (length != 32 || usb_cancel || state == IDLE || data[1] != token) return;
    if (timer_elapsed(started) >= CLIPBOARD_TIMEOUT) {
        cancel_transfer();
        return;
    }
    switch (data[0]) {
        case CLIPBOARD_STORE_BEGIN:
            if (state != STORE_REQUESTED) return;
            transfer_length = read_u16(data + 2);
            if (transfer_length > CLIPBOARD_MAX_BYTES) {
                send_status(1);
                cancel_transfer();
                return;
            }
            offset = 0;
            state = RECEIVING;
            break;
        case CLIPBOARD_STORE_DATA: {
            if (state != RECEIVING) return;
            uint8_t count = data[4];
            if (!count || count > CLIPBOARD_CHUNK_BYTES || read_u16(data + 2) != offset || offset + count > transfer_length) {
                send_status(1);
                cancel_transfer();
                return;
            }
            memcpy(incoming_text + offset, data + 5, count);
            offset += count;
            break;
        }
        case CLIPBOARD_STORE_COMMIT:
            if (state != RECEIVING) return;
            if (offset == transfer_length && valid_text((const uint8_t *)incoming_text, transfer_length) &&
                ((has_text && text_length == transfer_length && memcmp(stored_text, incoming_text, transfer_length) == 0) ||
                 clipboard_storage_save(incoming_text, transfer_length))) {
                memcpy(stored_text, incoming_text, transfer_length);
                stored_text[transfer_length] = '\0';
                text_length = transfer_length;
                has_text = true;
                send_status(0);
            } else {
                send_status(1);
            }
            cancel_transfer();
            break;
        case CLIPBOARD_PASTE_READY:
            if (state != WAIT_READY) return;
            if (data[2] == 0) state = PASTE_KEY;
            else cancel_transfer();
            break;
    }
}

void notify_usb_device_state_change_user(struct usb_device_state usb_state) {
    // Keep the slot across a powered KVM switch, cancel only in-flight work.
    if (usb_state.configure_state != USB_DEVICE_STATE_CONFIGURED) usb_cancel = true;
}

void clipboard_hid_task(void) {
    if (usb_cancel) {
        cancel_transfer();
        usb_cancel = false;
    }
    if (state == IDLE) return;
    if (timer_elapsed(started) >= CLIPBOARD_TIMEOUT) {
        cancel_transfer();
        return;
    }
    if (state == PASTE_KEY) {
        // Wait for the trigger and modifiers to be released; never paste a failure.
        if (paste_pressed || get_mods() || get_weak_mods()) return;
        if (os_name == OS_MACOS) tap_code16(G(KC_V));
        else if (terminal_application()) tap_code16(C(S(KC_V)));
        else tap_code16(C(KC_V));
        cancel_transfer();
        return;
    }
    if (timer_elapsed(last_packet) < CLIPBOARD_PACKET_INTERVAL) return;
    uint8_t report[32] = {0};
    report[1] = token;
    switch (state) {
        case PASTE_BEGIN:
            report[0] = CLIPBOARD_PASTE_BEGIN;
            report[2] = text_length & 0xff;
            report[3] = text_length >> 8;
            offset = 0;
            state = text_length ? PASTE_DATA : PASTE_END;
            break;
        case PASTE_DATA: {
            uint16_t remaining = text_length - offset;
            uint8_t count = remaining > CLIPBOARD_CHUNK_BYTES ? CLIPBOARD_CHUNK_BYTES : remaining;
            report[0] = CLIPBOARD_PASTE_DATA;
            report[2] = offset & 0xff;
            report[3] = offset >> 8;
            report[4] = count;
            memcpy(report + 5, stored_text + offset, count);
            offset += count;
            if (offset == text_length) state = PASTE_END;
            break;
        }
        case PASTE_END:
            report[0] = CLIPBOARD_PASTE_COMMIT;
            state = WAIT_READY;
            break;
        default:
            return;
    }
    raw_hid_send(report, sizeof(report));
    last_packet = timer_read();
}
