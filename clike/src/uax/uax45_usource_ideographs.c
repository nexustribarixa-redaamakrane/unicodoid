#include "unicodoid/uax/uax45_usource_ideographs.h"
#include "unicodoid/unicodoid_ucd.h"

bool unicodoid_uax45_is_usource(uint32_t cp) {
    if (unicodoid_ucd_has_binary_property(cp, UNICODOID_BP_UNIFIED_IDEOGRAPH)) return true;
    /* CJK Ext blocks carry U-source ideographs even before Unified_Ideograph
     * assignment in a given release. */
    return (cp >= 0x20000 && cp <= 0x2FA1F) || (cp >= 0x30000 && cp <= 0x3134F);
}

bool unicodoid_uax45_is_us1_staging(uint32_t cp) {
    return cp >= 0xF900 && cp <= 0xFAFF;
}

const char* unicodoid_uax45_usource_family(uint32_t cp) {
    if (unicodoid_uax45_is_us1_staging(cp)) return "US1";
    if (unicodoid_uax45_is_usource(cp)) return "US";
    return NULL;
}
