#include "unicodoid/uax/uax24_script_property.h"
#include "unicodoid/unicodoid_ucd.h"

#include <string.h>

size_t unicodoid_uax24_count(void) {
    return (size_t)unicodoid_ucd_script_names_count;
}

const char* unicodoid_uax24_script_name(uint16_t value) {
    return unicodoid_ucd_script_name(value);
}

uint16_t unicodoid_uax24_script(uint32_t cp) {
    return unicodoid_ucd_script(cp);
}

uint16_t unicodoid_uax24_find(const char* name) {
    if (!name) return 0;
    for (size_t i = 0; i < unicodoid_ucd_script_names_count; ++i) {
        if (strcmp(unicodoid_ucd_script_names[i], name) == 0) return (uint16_t)i;
    }
    return 0;
}

bool unicodoid_uax24_is_script(uint32_t cp, uint16_t script) {
    return unicodoid_ucd_script(cp) == script;
}
