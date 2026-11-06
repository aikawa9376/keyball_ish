/* Host regression tests; compiler invocation is documented in readme.md. */
#include "qmk.h"
#include "usb_device_state.h"
#include "eeprom.h"
#include "wear_leveling.h"
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
static uint8_t flash[TOTAL_EEPROM_BYTE_COUNT], cache[TOTAL_EEPROM_BYTE_COUNT];
static unsigned write_count;
static int write_budget = -1;

wear_leveling_status_t wear_leveling_read(uint32_t address, void *value, size_t length) {
    assert(address + length <= sizeof(cache));
    memcpy(value, cache + address, length);
    return WEAR_LEVELING_SUCCESS;
}
wear_leveling_status_t wear_leveling_write(uint32_t address, const void *value, size_t length) {
    assert(address >= 1024 && address + length <= 1856);
    ++write_count;
    // Match QMK's cache-before-flash behavior, including a failed flash write.
    if (memcmp(cache + address, value, length) == 0) return WEAR_LEVELING_SUCCESS;
    memcpy(cache + address, value, length);
    for (size_t i = 0; i < length; ++i) {
        if (write_budget == 0) return WEAR_LEVELING_FAILED;
        flash[address + i] = ((const uint8_t *)value)[i];
        if (write_budget > 0) --write_budget;
    }
    return WEAR_LEVELING_CONSOLIDATED;
}

static void reboot(void) {
    memcpy(cache, flash, sizeof(cache));
    write_budget = -1;
    packet_count = 0;
    keyboard_post_init_user();
    assert(packet_count == 0); // Boot must neither send clipboard packets nor paste.
}

static void expect_no_slot(void) {
    packet_count = 0;
    clipboard_hid_paste(true);
    clock_ms += 4;
    clipboard_hid_task();
    clipboard_hid_paste(false);
    assert(packet_count == 0);
}

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
    // Virgin flash must not synthesize an empty clipboard or a paste.
    memset(flash, 0xff, sizeof(flash));
    reboot();
    expect_no_slot();
    uint8_t text[400];
    for (unsigned i = 0; i < sizeof(text); i += 4) memcpy(text + i, "\xf0\x9f\x98\x80", 4);
    uint8_t token = request_store();
    upload(token, text, sizeof(text));
    assert(packets[packet_count - 1][0] == CLIPBOARD_STORE_STATUS && packets[packet_count - 1][2] == 0);
    reboot(); // RAM is rebuilt from EEPROM before the first paste.
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

    // All failed/invalid uploads above must leave the durable slot intact too.
    reboot();
    download(text, sizeof(text));
    unsigned previous_writes = write_count;
    upload(request_store(), text, sizeof(text));
    assert(packets[packet_count - 1][2] == 0 && write_count == previous_writes);

    // Two records allow fallback when the latest record is corrupt.
    const uint8_t next[] = "next PC\n\xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e";
    upload(request_store(), next, sizeof(next) - 1);
    reboot();
    download(next, sizeof(next) - 1);
    uint8_t baseline[sizeof(flash)];
    memcpy(baseline, flash, sizeof(flash));
    flash[1024 + 416 + 16] ^= 1; // Damage newest payload (CRC must reject it).
    reboot();
    download(text, sizeof(text));
    memset(flash + 1024, 0, 832); // Missing/invalid markers must also be rejected.
    reboot();
    expect_no_slot();

    // Cut every changed byte of a third save, including its final commit marker.
    // The previous committed slot must survive every simulated interruption.
    const uint8_t replacement[] = "replacement";
    for (int cut = 0; cut < 420; ++cut) {
        memcpy(flash, baseline, sizeof(flash));
        reboot();
        write_budget = cut;
        upload(request_store(), replacement, sizeof(replacement) - 1);
        if (packets[packet_count - 1][2] == 1) {
            download(next, sizeof(next) - 1); // Failed save preserves live RAM.
            unsigned failed_writes = write_count;
            write_budget = -1;
            upload(request_store(), replacement, sizeof(replacement) - 1);
            assert(packets[packet_count - 1][2] == 1 && write_count == failed_writes);
            reboot();
            download(next, sizeof(next) - 1);
        } else {
            reboot();
            download(replacement, sizeof(replacement) - 1);
        }
        assert(memcmp(flash, baseline, 1024) == 0);
        assert(memcmp(flash + 1856, baseline + 1856, sizeof(flash) - 1856) == 0);
    }

    // Clearing by explicitly storing empty text persists across a restart.
    memcpy(flash, baseline, sizeof(flash));
    reboot();
    upload(request_store(), (const uint8_t *)"", 0);
    assert(packets[packet_count - 1][2] == 0);
    reboot();
    download((const uint8_t *)"", 0);
    puts("clipboard HID tests passed");
}
