#include "unicodoid/uts/uts35_cldr/cldr_messageformat.h"

int unicodoid_cldr_plural(const char* locale, double n) {
    (void)locale;
    if (n == 1.0) return UNICODOID_CLDR_PLURAL_ONE;
    if (n == 0.0) return UNICODOID_CLDR_PLURAL_ZERO;
    return UNICODOID_CLDR_PLURAL_OTHER;
}

bool unicodoid_cldr_is_message_pattern_valid(const char* pattern) {
    if (!pattern) return false;
    int depth = 0;
    for (const char* p = pattern; *p; ++p) {
        if (*p == '{') ++depth;
        else if (*p == '}') { --depth; if (depth < 0) return false; }
    }
    return depth == 0;
}
