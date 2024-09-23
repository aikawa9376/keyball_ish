#include <string.h> // for strncpy
#include "raw_hid.h"

#define APPLICATION_NAME 0x01
#define DATETIME_UPDATE 0x02

char application_name[32 + 1]; // +1 for null terminator
char current_datetime[32 + 1]; // +1 for null terminator


void raw_hid_receive(uint8_t *data, uint8_t length) {
    // nullptr チェック
    if (data != NULL && length > 0) {
        // 先頭バイトでイベントを判定
        uint8_t event_type = data[0];

        switch (event_type) {
            case APPLICATION_NAME:
                // データのコピー (アプリケーション名更新)
                memset(application_name, 0, sizeof(application_name));
                strncpy(application_name, (char*)(data + 1), length - 1);
                application_name[length - 1] = '\0';  // null terminator

                // key_overridesを設定
                set_key_overrides(application_name);
                break;

            case DATETIME_UPDATE:
                memset(current_datetime, 0, sizeof(current_datetime));
                strncpy(current_datetime, (char*)(data + 1), length - 1);
                current_datetime[length - 1] = '\0';  // null terminator
                send_string(current_datetime);
                break;

            default:
                break;
        }
    }
}
