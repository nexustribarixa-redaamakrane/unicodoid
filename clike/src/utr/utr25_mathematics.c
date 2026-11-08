#include "unicodoid/utr/utr25_mathematics.h"
#include "unicodoid/unicodoid_ucd.h"

bool unicodoid_utr25_is_math(uint32_t cp) {
    return unicodoid_ucd_has_binary_property(cp, UNICODOID_BP_MATH);
}

bool unicodoid_utr25_is_math_alphanumeric(uint32_t cp) {
    return cp >= 0x1D400 && cp <= 0x1D7FF;
}

bool unicodoid_utr25_is_operator(uint32_t cp) {
    if (!unicodoid_utr25_is_math(cp)) return false;
    uint8_t gc = unicodoid_ucd_general_category(cp);
    return gc == (uint8_t)UNICODOID_GC_SM || gc == (uint8_t)UNICODOID_GC_SC;
}
