#include "unicodoid/utr/utr17_character_encoding_model.h"
#include "unicodoid/unicodoid_types.h"
#include "unicodoid/unicodoid_ucd.h"

int unicodoid_utr17_utf8_length(uint32_t cp) {
    if (!unicodoid_is_valid_unicode_scalar(cp)) return 0;
    return cp <= 0x7F ? 1 : cp <= 0x7FF ? 2 : cp <= 0xFFFF ? 3 : 4;
}

int unicodoid_utr17_utf16_length(uint32_t cp) {
    if (!unicodoid_is_valid_unicode_scalar(cp)) return 0;
    return cp <= 0xFFFF ? 1 : 2;
}

bool unicodoid_utr17_is_valid(uint32_t cp) {
    return unicodoid_is_valid_unicode_scalar(cp);
}

bool unicodoid_utr17_is_replacement(uint32_t cp) {
    if (unicodoid_ucd_has_binary_property(cp, UNICODOID_BP_NONCHARACTER_CODE_POINT)) return true;
    return unicodoid_ucd_general_category(cp) == (uint8_t)UNICODOID_GC_CN;
}
