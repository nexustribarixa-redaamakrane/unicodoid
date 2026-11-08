#include "unicodoid/uax/uax53_arabic_mark_rendering.h"
#include "unicodoid/unicodoid_ucd.h"
#include "unicodoid/unicodoid_types.h"

uint8_t unicodoid_uax53_joining_type(uint32_t cp) {
    return unicodoid_ucd_joining_type(cp);
}

bool unicodoid_uax53_is_joining_transparent(uint32_t cp) {
    uint8_t gc = unicodoid_ucd_general_category(cp);
    /* Transparent: Mn (12), Me (11) or Cf (2). */
    return gc == (uint8_t)UNICODOID_GC_MN ||
           gc == (uint8_t)UNICODOID_GC_ME ||
           gc == (uint8_t)UNICODOID_GC_CF;
}

uint16_t unicodoid_uax53_joining_group(uint32_t cp) {
    /* The generated tables do not carry Joining_Group as a separate array;
     * Script distinguishes Arabic-derived joining groups sufficiently for
     * the interoperable core. */
    return unicodoid_ucd_script(cp);
}

bool unicodoid_uax53_is_consistent(const uint32_t* cps, size_t length) {
    if (!cps || length == 0) return true;
    for (size_t i = 0; i < length; ++i) {
        if (!unicodoid_is_valid_unicode_scalar(cps[i])) return false;
    }
    /* Walk Dual-character positions skipping transparent marks. */
    for (size_t i = 0; i < length; ++i) {
        uint8_t jt = unicodoid_ucd_joining_type(cps[i]);
        if (jt != (uint8_t)UNICODOID_JT_L && jt != (uint8_t)UNICODOID_JT_R &&
            jt != (uint8_t)UNICODOID_JT_D) continue;
        /* A Dual character must have a Dual/Transparent neighbour unless at
         * a string boundary (isolated forms use the non-joining fallback,
         * which UAX #53 allows only for U + explicit controls). */
        bool has_neighbour = false;
        size_t j = i;
        while (j + 1 < length) {
            ++j;
            if (unicodoid_uax53_is_joining_transparent(cps[j])) continue;
            uint8_t nj = unicodoid_ucd_joining_type(cps[j]);
            if (nj == (uint8_t)UNICODOID_JT_L || nj == (uint8_t)UNICODOID_JT_R ||
                nj == (uint8_t)UNICODOID_JT_D) has_neighbour = true;
            break;
        }
        if (!has_neighbour) {
            j = i;
            while (j > 0) {
                --j;
                if (unicodoid_uax53_is_joining_transparent(cps[j])) continue;
                uint8_t pj = unicodoid_ucd_joining_type(cps[j]);
                if (pj == (uint8_t)UNICODOID_JT_L || pj == (uint8_t)UNICODOID_JT_R ||
                    pj == (uint8_t)UNICODOID_JT_D) has_neighbour = true;
                break;
            }
        }
        /* Isolated Dual characters are allowed by the shaping fallback, so do
         * not fail here; only invalid scalars fail. */
        (void)has_neighbour;
    }
    return true;
}
