#pragma once

#include "process_key_override.h"

#define MAX_OVERRIDES 30

extern const key_override_t *key_overrides[MAX_OVERRIDES];

void set_key_overrides(const char *application_name);
