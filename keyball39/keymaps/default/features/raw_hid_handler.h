#include <string.h> // for strncpy
#include "raw_hid.h"

char application_name[32 + 1]; // +1 for null terminator

void raw_hid_receive(uint8_t *data, uint8_t length) {
    // nullptr チェック
    if (data != NULL) {
        // データのコピー
        memset(application_name, 0, length);
        strncpy(application_name, (char*)(data + 1), length);
        application_name[length] = '\0';  // null terminator

        // key_overridesを設定
        set_key_overrides(application_name);
    }
}
