#include "unicodoid/uts/uts37_ivd.h"

bool unicodoid_uts37_is_variation_selector(uint32_t cp) {
    return cp >= 0xE0100 && cp <= 0xE01EF;
}

int unicodoid_uts37_selector_offset(uint32_t cp) {
    if (!unicodoid_uts37_is_variation_selector(cp)) return -1;
    return (int)(cp - 0xE0100);
}

bool unicodoid_uts37_is_ivs(const uint32_t* cps, size_t length) {
    if (!cps || length != 2) return false;
    if (cps[0] > 0x10FFFF || (cps[0] >= 0xD800 && cps[0] <= 0xDFFF)) return false;
    return unicodoid_uts37_is_variation_selector(cps[1]);
}
