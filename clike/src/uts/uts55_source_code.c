#include "unicodoid/uts/uts55_source_code.h"
#include "unicodoid/unicodoid_ucd.h"

bool unicodoid_uts55_is_bidi_control(uint32_t cp) {
    return unicodoid_ucd_has_binary_property(cp, UNICODOID_BP_BIDI_CONTROL);
}

bool unicodoid_uts55_is_joiner(uint32_t cp) {
    return cp == 0x200C || cp == 0x200D;
}

size_t unicodoid_uts55_flag_count(const uint32_t* cps, size_t length) {
    if (!cps) return 0;
    size_t n = 0;
    for (size_t i = 0; i < length; ++i) {
        if (unicodoid_uts55_is_bidi_control(cps[i]) || unicodoid_uts55_is_joiner(cps[i])) ++n;
    }
    return n;
}
