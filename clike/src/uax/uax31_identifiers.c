#include "unicodoid/uax/uax31_identifiers.h"
#include "unicodoid/unicodoid_ucd.h"
#include "unicodoid/uax/uax15_normalization.h"

#include <stdlib.h>

bool unicodoid_uax31_is_xid_start(uint32_t cp) {
    return unicodoid_ucd_has_binary_property(cp, UNICODOID_BP_XID_START);
}

bool unicodoid_uax31_is_xid_continue(uint32_t cp) {
    return unicodoid_ucd_has_binary_property(cp, UNICODOID_BP_XID_CONTINUE);
}

bool unicodoid_uax31_is_id_start(uint32_t cp) {
    return unicodoid_ucd_has_binary_property(cp, UNICODOID_BP_ID_START);
}

bool unicodoid_uax31_is_id_continue(uint32_t cp) {
    return unicodoid_ucd_has_binary_property(cp, UNICODOID_BP_ID_CONTINUE);
}

bool unicodoid_uax31_is_valid(const uint32_t* cps, size_t length) {
    if (!cps || length == 0) return false;
    for (size_t i = 0; i < length; ++i) {
        uint32_t cp = cps[i];
        if (cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF)) return false;
    }
    if (!unicodoid_uax31_is_xid_start(cps[0])) return false;
    for (size_t i = 1; i < length; ++i) {
        uint32_t cp = cps[i];
        if (cp == 0x200C || cp == 0x200D) continue;
        if (!unicodoid_uax31_is_xid_continue(cp)) return false;
    }
    return true;
}

bool unicodoid_uax31_is_nfkc_stable(const uint32_t* cps, size_t length) {
    if (!cps || length == 0) return true;
    uint32_t tmp[64];
    size_t got = unicodoid_uax15_normalize(cps, length, 2, NULL, 0);
    if (got != length) return false;
    if (length <= sizeof(tmp) / sizeof(tmp[0])) {
        unicodoid_uax15_normalize(cps, length, 2, tmp, 64);
        for (size_t i = 0; i < length; ++i) if (tmp[i] != cps[i]) return false;
        return true;
    }
    /* Large input: normalize into heap. */
    uint32_t* heap = (uint32_t*)malloc(length * sizeof(uint32_t));
    if (!heap) return false;
    unicodoid_uax15_normalize(cps, length, 2, heap, length);
    bool ok = true;
    for (size_t i = 0; i < length; ++i) if (heap[i] != cps[i]) { ok = false; break; }
    free(heap);
    return ok;
}
