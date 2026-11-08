#include "unicodoid/uts/uts61_set_notation.h"
#include "unicodoid/uts/uts18_regex.h"

#include <stdlib.h>
#include <string.h>

int unicodoid_uts61_parse_range(const char* text, uint32_t* lo, uint32_t* hi) {
    if (!text || !lo || !hi) return 0;
    const char* dots = strstr(text, "..");
    if (!dots) return 0;
    char* end = NULL;
    /* lo is hex before "..", hi after. */
    if (dots == text) return 0;
    unsigned long a = strtoul(text, &end, 16);
    if (end != dots) return 0;
    unsigned long b = strtoul(dots + 2, &end, 16);
    if (*end) return 0;
    if (a > 0x10FFFF || b > 0x10FFFF || a > b) return 0;
    *lo = (uint32_t)a;
    *hi = (uint32_t)b;
    return 1;
}

bool unicodoid_uts61_in_range(uint32_t cp, uint32_t lo, uint32_t hi) {
    return cp >= lo && cp <= hi;
}

int unicodoid_uts61_set_membership(const char* set, uint32_t cp) {
    if (!set) return -1;
    /* Single property escape: delegate to UTS #18 (which already applies
     * \P negation). */
    if (set[0] == '\\' && (set[1] == 'p' || set[1] == 'P')) {
        int m = unicodoid_uts18_property_match(set, cp);
        if (m == UNICODOID_UTS18_INVALID) return -1;
        return (m == UNICODOID_UTS18_NONE) ? 0 : 1;
    }
    /* Range set "X..Y". */
    uint32_t lo = 0, hi = 0;
    if (unicodoid_uts61_parse_range(set, &lo, &hi)) {
        return unicodoid_uts61_in_range(cp, lo, hi) ? 1 : 0;
    }
    return -1;
}
