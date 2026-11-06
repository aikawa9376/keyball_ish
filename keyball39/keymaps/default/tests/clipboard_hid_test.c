/* Host regression tests; compiler invocation is documented in readme.md. */
#include "qmk.h"
#include "usb_device_state.h"
#include "../features/clipboard_hid.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

os_variant_t os_name = OS_LINUX;
char application_name[33] = "kitty";
char current_datetime[33];
static uint16_t clock_ms, pasted_key;
static uint8_t mods;
static unsigned paste_count, packet_count;
static uint8_t packets[40][32];

uint16_t timer_read(void) { return clock_ms; }
uint16_t timer_elapsed(uint16_t started) { return clock_ms - started; }
uint8_t get_mods(void) { return mods; }
uint8_t get_weak_mods(void) { return 0; }
void tap_code16(uint16_t keycode) { pasted_key = keycode; ++paste_count; }
void raw_hid_send(uint8_t *data, uint8_t length) {
    assert(length == 32 && packet_count < 40);
    memcpy(packets[packet_count++], data, 32);
}
static void tick(unsigned ms) { clock_ms += ms; clipboard_hid_task(); }
static uint8_t request_store(void) {
    packet_count = 0;
    clipboard_hid_store();
    assert(packet_count == 1 && packets[0][0] == CLIPBOARD_STORE_REQUEST);
    return packets[0][1];
}
static void receive(uint8_t event, uint8_t token, uint16_t value, const uint8_t *bytes, uint8_t count) {
    uint8_t packet[32] = {event, token, value & 0xff, value >> 8, count};
    if (bytes) memcpy(packet + 5, bytes, count);
    clipboard_hid_receive(packet, 32);
}
static void upload(uint8_t token, const uint8_t *text, unsigned length) {
    receive(CLIPBOARD_STORE_BEGIN, token, length, NULL, 0);
    for (unsigned offset = 0; offset < length;) {
        unsigned count = length - offset;
        if (count > CLIPBOARD_CHUNK_BYTES) count = CLIPBOARD_CHUNK_BYTES;
        receive(CLIPBOARD_STORE_DATA, token, offset, text + offset, count);
        offset += count;
    }
    receive(CLIPBOARD_STORE_COMMIT, token, 0, NULL, 0);
}
static uint8_t download(const uint8_t *expected, unsigned length) {
    packet_count = 0;
    clipboard_hid_paste(true);
    for (unsigned i = 0; i < 20; ++i) tick(4);
    assert(packets[0][0] == CLIPBOARD_PASTE_BEGIN);
    assert((unsigned)(packets[0][2] | (packets[0][3] << 8)) == length);
    unsigned offset = 0;
    for (unsigned i = 1; i + 1 < packet_count; ++i) {
        assert(packets[i][0] == CLIPBOARD_PASTE_DATA);
        assert((unsigned)(packets[i][2] | (packets[i][3] << 8)) == offset);
        assert(packets[i][4] <= CLIPBOARD_CHUNK_BYTES);
        assert(memcmp(expected + offset, packets[i] + 5, packets[i][4]) == 0);
        offset += packets[i][4];
    }
    assert(offset == length);
    assert(packets[packet_count - 1][0] == CLIPBOARD_PASTE_COMMIT);
    return packets[0][1];
}
int main(void) {
    uint8_t text[400];
    for (unsigned i = 0; i < sizeof(text); i += 4) memcpy(text + i, "\xf0\x9f\x98\x80", 4);
    uint8_t token = request_store();
    upload(token, text, sizeof(text));
    assert(packets[packet_count - 1][0] == CLIPBOARD_STORE_STATUS && packets[packet_count - 1][2] == 0);
    token = download(text, sizeof(text));
    receive(CLIPBOARD_PASTE_READY, token + 1, 0, NULL, 0);
    clipboard_hid_paste(false);
    tick(1);
    assert(paste_count == 0); // A response from an old request must not paste.
    receive(CLIPBOARD_PASTE_READY, token, 0, NULL, 0);
    mods = 1;
    tick(1);
    assert(paste_count == 0);
    mods = 0;
    tick(1);
    assert(paste_count == 1 && pasted_key == C(S(KC_V)));

    // Reordered/incomplete uploads and invalid UTF-8 retain the previous slot.
    token = request_store();
    receive(CLIPBOARD_STORE_BEGIN, token, 3, NULL, 0);
    receive(CLIPBOARD_STORE_DATA, token, 1, (const uint8_t *)"a", 1);
    assert(packets[packet_count - 1][2] == 1);
    token = request_store();
    upload(token, (const uint8_t *)"\xed\xa0\x80", 3); // surrogate
    assert(packets[packet_count - 1][2] == 1);
    token = request_store();
    upload(token, (const uint8_t *)"\xc0\x80", 2); // overlong NUL
    assert(packets[packet_count - 1][2] == 1);
    uint8_t too_many[101];
    memset(too_many, 'a', sizeof(too_many));
    token = request_store();
    upload(token, too_many, sizeof(too_many));
    assert(packets[packet_count - 1][2] == 1);
    token = request_store();
    receive(CLIPBOARD_STORE_BEGIN, token, 1, NULL, 0);
    tick(3000);
    receive(CLIPBOARD_STORE_COMMIT, token, 0, NULL, 0);
    token = download(text, sizeof(text));
    receive(CLIPBOARD_PASTE_READY, token, 1, NULL, 0); // Helper failure.
    clipboard_hid_paste(false);
    tick(1);
    assert(paste_count == 1);

    // A powered USB switch cancels a pending paste without discarding the slot.
    token = download(text, sizeof(text));
    notify_usb_device_state_change_user((struct usb_device_state){USB_DEVICE_STATE_INIT});
    receive(CLIPBOARD_PASTE_READY, token, 0, NULL, 0);
    clipboard_hid_paste(false);
    tick(1);
    assert(paste_count == 1);
    token = download(text, sizeof(text));
    receive(CLIPBOARD_PASTE_READY, token, 0, NULL, 0);
    tick(1);
    assert(paste_count == 1); // Trigger still held.
    os_name = OS_MACOS;
    clipboard_hid_paste(false);
    tick(1);
    assert(paste_count == 2 && pasted_key == G(KC_V));
    token = download(text, sizeof(text));
    tick(3000);
    receive(CLIPBOARD_PASTE_READY, token, 0, NULL, 0);
    clipboard_hid_paste(false);
    tick(1);
    assert(paste_count == 2);
    token = download(text, sizeof(text));
    clipboard_hid_cancel_paste(); // Another key was pressed before readiness.
    receive(CLIPBOARD_PASTE_READY, token, 0, NULL, 0);
    clipboard_hid_paste(false);
    tick(1);
    assert(paste_count == 2);
    puts("clipboard HID tests passed");
}
