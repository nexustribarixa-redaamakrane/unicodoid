#include "unicodoid/uts/uts35_cldr/cldr_collation.h"

#include <string.h>

void unicodoid_cldr_collation(const char* locale, unicodoid_cldr_collation_t* out) {
    if (!out) return;
    out->strength = 3;
    out->alternate = unicodoid_cldr_needs_shifted(locale) ? 1 : 0;
}

bool unicodoid_cldr_needs_shifted(const char* locale) {
    if (!locale) return false;
    const char* p = locale;
    return (strcmp(p, "zh") == 0 || strcmp(p, "sv") == 0 ||
            strstr(p, "_zh") != NULL || strstr(p, "_sv") != NULL);
}
