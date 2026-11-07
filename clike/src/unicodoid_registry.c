#include "unicodoid/unicodoid_registry.h"
#include "unicodoid/systems/unicodoid_utf8.h"
#include "unicodoid/systems/unicodoid_utf16.h"
#include "unicodoid/systems/unicodoid_utf32.h"
#include "unicodoid/systems/unicodoid_ascii.h"
#include "unicodoid/systems/unicodoid_custom_conlang.h"
#include "unicodoid/systems/unicodoid_shiftmodal16.h"
#include <string.h>

#define UNICODOID_MAX_REGISTERED_SYSTEMS 64

static const unicodoid_system_t* s_registry[UNICODOID_MAX_REGISTERED_SYSTEMS];
static size_t s_registry_count = 0;
static bool s_initialized = false;

unicodoid_status_t unicodoid_registry_init(void) {
    if (s_initialized) {
        return UNICODOID_OK;
    }

    s_registry_count = 0;

    /* Register built-in reference systems */
    unicodoid_registry_register(unicodoid_system_get_utf8());
    unicodoid_registry_register(unicodoid_system_get_utf16le());
    unicodoid_registry_register(unicodoid_system_get_utf16be());
    unicodoid_registry_register(unicodoid_system_get_utf32le());
    unicodoid_registry_register(unicodoid_system_get_utf32be());
    unicodoid_registry_register(unicodoid_system_get_ascii());
    unicodoid_registry_register(unicodoid_system_get_custom_conlang());
    unicodoid_registry_register(unicodoid_system_get_shiftmodal16());

    s_initialized = true;
    return UNICODOID_OK;
}

void unicodoid_registry_cleanup(void) {
    s_registry_count = 0;
    s_initialized = false;
}

unicodoid_status_t unicodoid_registry_register(const unicodoid_system_t* sys) {
    if (!sys || !sys->meta.id) {
        return UNICODOID_ERR_INVALID_PARAM;
    }

    /* Check for duplicates */
    for (size_t i = 0; i < s_registry_count; ++i) {
        if (strcmp(s_registry[i]->meta.id, sys->meta.id) == 0) {
            return UNICODOID_ERR_ALREADY_EXISTS;
        }
    }

    if (s_registry_count >= UNICODOID_MAX_REGISTERED_SYSTEMS) {
        return UNICODOID_ERR_CAPACITY_EXCEEDED;
    }

    s_registry[s_registry_count++] = sys;
    return UNICODOID_OK;
}

unicodoid_status_t unicodoid_registry_unregister(const char* id) {
    if (!id) {
        return UNICODOID_ERR_INVALID_PARAM;
    }

    for (size_t i = 0; i < s_registry_count; ++i) {
        if (strcmp(s_registry[i]->meta.id, id) == 0) {
            /* Shift down remaining */
            for (size_t j = i; j + 1 < s_registry_count; ++j) {
                s_registry[j] = s_registry[j + 1];
            }
            s_registry_count--;
            return UNICODOID_OK;
        }
    }

    return UNICODOID_ERR_NOT_FOUND;
}

const unicodoid_system_t* unicodoid_registry_find_by_id(const char* id) {
    if (!s_initialized) {
        unicodoid_registry_init();
    }
    if (!id) {
        return NULL;
    }

    for (size_t i = 0; i < s_registry_count; ++i) {
        if (strcmp(s_registry[i]->meta.id, id) == 0) {
            return s_registry[i];
        }
    }

    return NULL;
}

const unicodoid_system_t* unicodoid_registry_find_by_name(const char* name) {
    if (!s_initialized) {
        unicodoid_registry_init();
    }
    if (!name) {
        return NULL;
    }

    for (size_t i = 0; i < s_registry_count; ++i) {
        if (s_registry[i]->meta.name && strcmp(s_registry[i]->meta.name, name) == 0) {
            return s_registry[i];
        }
    }

    return NULL;
}

size_t unicodoid_registry_count(void) {
    if (!s_initialized) {
        unicodoid_registry_init();
    }
    return s_registry_count;
}

const unicodoid_system_t* unicodoid_registry_get_at(size_t index) {
    if (!s_initialized) {
        unicodoid_registry_init();
    }
    if (index >= s_registry_count) {
        return NULL;
    }
    return s_registry[index];
}
