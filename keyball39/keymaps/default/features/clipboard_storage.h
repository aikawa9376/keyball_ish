#pragma once

#include <stdbool.h>
#include <stdint.h>

bool clipboard_storage_load(char *text, uint16_t *length);
bool clipboard_storage_save(const char *text, uint16_t length);
