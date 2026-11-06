#include QMK_KEYBOARD_H
#include <string.h> // for strncpy
#include "raw_hid.h"

#include "overrides.h"
#include "raw_hid_handler.h"
#include "clipboard_hid.h"

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
                size_t app_length = length - 1;
                if (app_length >= sizeof(application_name)) app_length = sizeof(application_name) - 1;
                memcpy(application_name, data + 1, app_length);

                // key_overridesを設定
                set_key_overrides(application_name);
                break;

            case DATETIME_UPDATE:
                memset(current_datetime, 0, sizeof(current_datetime));
                size_t datetime_length = length - 1;
                if (datetime_length >= sizeof(current_datetime)) datetime_length = sizeof(current_datetime) - 1;
                memcpy(current_datetime, data + 1, datetime_length);
                send_string(current_datetime);
                break;

            default:
                clipboard_hid_receive(data, length);
                break;
        }
    }
}
