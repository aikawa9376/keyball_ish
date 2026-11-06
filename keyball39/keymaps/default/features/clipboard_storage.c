#include QMK_KEYBOARD_H
#include <string.h>
#include "eeprom.h"
#include "nvm_eeprom_eeconfig_internal.h"
#include "wear_leveling.h"
#include "clipboard_hid.h"
#include "clipboard_storage.h"

#if !defined(EEPROM_WEAR_LEVELING) || !defined(WEAR_LEVELING_RP2040_FLASH)
#    error "Clipboard persistence requires RP2040 flash wear leveling"
#endif
#if defined(VIA_ENABLE) || defined(DYNAMIC_KEYMAP_ENABLE)
#    error "Reserve the clipboard EEPROM region before enabling dynamic keymaps"
#endif

// Fixed logical EEPROM addresses leave the existing Keyball/Naginata settings intact.
// Each record: magic/version (4), generation (4), length (2), reserved (2),
// CRC32 (4), UTF-8 payload (400). Keep this layout stable across firmware updates.
#define STORAGE_BASE 1024
#define HEADER_BYTES 16
#define RECORD_BYTES (HEADER_BYTES + CLIPBOARD_MAX_BYTES)
#define STORAGE_MAGIC 0x3142434bUL // "KCB1"

_Static_assert(EECONFIG_SIZE <= STORAGE_BASE, "Clipboard overlaps QMK settings");
_Static_assert(STORAGE_BASE + 2 * RECORD_BYTES <= TOTAL_EEPROM_BYTE_COUNT, "Clipboard exceeds EEPROM capacity");

static int8_t active_slot = -1;
static uint32_t generation;
static bool write_failed;

static uint32_t read_u32(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static void write_u32(uint8_t *p, uint32_t value) {
    for (uint8_t i = 0; i < 4; ++i) p[i] = value >> (8 * i);
}

static uint32_t record_crc(const uint8_t *record) {
    uint32_t crc = 0xffffffffUL;
    for (uint16_t i = 4; i < RECORD_BYTES; ++i) {
        if (i >= 12 && i < HEADER_BYTES) continue; // Exclude the CRC itself.
        crc ^= record[i];
        for (uint8_t bit = 0; bit < 8; ++bit) crc = (crc >> 1) ^ ((crc & 1) ? 0xedb88320UL : 0);
    }
    return ~crc;
}

bool clipboard_storage_load(char *text, uint16_t *length) {
    uint8_t record[RECORD_BYTES];
    active_slot = -1;
    generation = 0;
    write_failed = false;
    for (int8_t slot = 0; slot < 2; ++slot) {
        if (wear_leveling_read(STORAGE_BASE + slot * RECORD_BYTES, record, sizeof(record)) == WEAR_LEVELING_FAILED) continue;
        uint16_t size = record[8] | ((uint16_t)record[9] << 8);
        if (read_u32(record) != STORAGE_MAGIC || size > CLIPBOARD_MAX_BYTES || read_u32(record + 12) != record_crc(record)) continue;
        uint32_t candidate = read_u32(record + 4);
        // Unsigned subtraction also orders consecutive generations across wraparound.
        if (active_slot >= 0 && (candidate - generation == 0 || candidate - generation >= 0x80000000UL)) continue;
        active_slot = slot;
        generation = candidate;
        memcpy(text, record + HEADER_BYTES, size);
        text[size] = '\0';
        *length = size;
    }
    return active_slot >= 0;
}

static bool persist(uint32_t address, const void *data, uint16_t size) {
    if (wear_leveling_write(address, data, size) != WEAR_LEVELING_FAILED) return true;
    // QMK updates its RAM cache even when the flash write fails. Until reboot,
    // a retry could otherwise report success for data that never reached flash.
    write_failed = true;
    return false;
}

bool clipboard_storage_save(const char *text, uint16_t length) {
    if (write_failed || length > CLIPBOARD_MAX_BYTES) return false;
    uint8_t record[RECORD_BYTES] = {0};
    uint8_t slot = active_slot == 0 ? 1 : 0;
    uint32_t address = STORAGE_BASE + slot * RECORD_BYTES;
    write_u32(record + 4, generation + 1);
    record[8] = length & 0xff;
    record[9] = length >> 8;
    memcpy(record + HEADER_BYTES, text, length);
    write_u32(record + 12, record_crc(record));
    // Invalidate the older record before changing it; commit its marker last.
    // The other record remains available if this operation is interrupted.
    if (!persist(address, record, 4) || !persist(address + 4, record + 4, sizeof(record) - 4)) return false;
    write_u32(record, STORAGE_MAGIC);
    if (!persist(address, record, 4)) return false;
    active_slot = slot;
    ++generation;
    return true;
}
