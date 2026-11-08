#include "unicodoid/uax/uax57_unikemet.h"

bool unicodoid_uax57_is_hieroglyph(uint32_t cp) {
    return (cp >= 0x13000 && cp <= 0x1342F) || (cp >= 0x13460 && cp <= 0x143FF);
}

bool unicodoid_uax57_is_core(uint32_t cp) {
    return cp >= 0x13460 && cp <= 0x143FF;
}

bool unicodoid_uax57_is_legacy(uint32_t cp) {
    return cp >= 0x13000 && cp <= 0x1342F;
}

bool unicodoid_uax57_is_no_mirror(uint32_t cp) {
    return cp == 0x130BB || cp == 0x130BD;
}

size_t unicodoid_uax57_unikemet_field(
    uint32_t cp, const char* field, char* out, size_t cap) {
    (void)cp;
    (void)field;
    (void)out;
    (void)cap;
    return 0; /* Full kEH_* store lives in Unikemet.txt. */
}
