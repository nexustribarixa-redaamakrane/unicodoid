#include "unicodoid/uax/uax11_eaw.h"
#include "unicodoid/unicodoid_ucd.h"

uint8_t unicodoid_uax11_eaw(uint32_t cp) {
    return unicodoid_ucd_east_asian_width(cp);
}

bool unicodoid_uax11_is_wide(uint32_t cp) {
    uint8_t v = unicodoid_ucd_east_asian_width(cp);
    return v == (uint8_t)UNICODOID_EAW_F || v == (uint8_t)UNICODOID_EAW_W;
}

int unicodoid_uax11_column_count(uint32_t cp, bool ambiguous_as_wide) {
    uint8_t v = unicodoid_ucd_east_asian_width(cp);
    if (v == (uint8_t)UNICODOID_EAW_F || v == (uint8_t)UNICODOID_EAW_W) return 2;
    if (v == (uint8_t)UNICODOID_EAW_A && ambiguous_as_wide) return 2;
    return 1;
}

size_t unicodoid_uax11_columns(
    const uint32_t* cps, size_t length, bool ambiguous_as_wide) {
    if (!cps) return 0;
    size_t total = 0;
    for (size_t i = 0; i < length; ++i)
        total += (size_t)unicodoid_uax11_column_count(cps[i], ambiguous_as_wide);
    return total;
}
