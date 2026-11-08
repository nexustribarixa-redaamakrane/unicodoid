#include "unicodoid/uts/uts35_cldr/cldr_person_names.h"

#include <string.h>

int unicodoid_cldr_name_order(const char* locale) {
    if (!locale) return UNICODOID_CLDR_NAME_ORDER_GIVEN_FIRST;
    if (strlen(locale) >= 2 && (strncmp(locale, "zh", 2) == 0 ||
                               strncmp(locale, "ko", 2) == 0 ||
                               strncmp(locale, "ja", 2) == 0 ||
                               strncmp(locale, "hu", 2) == 0))
        return UNICODOID_CLDR_NAME_ORDER_FAMILY_FIRST;
    return UNICODOID_CLDR_NAME_ORDER_GIVEN_FIRST;
}

bool unicodoid_cldr_is_person_name(const char* locale, const char* name) {
    (void)locale;
    if (!name || !*name) return false;
    for (const char* p = name; *p; ++p) {
        unsigned char c = (unsigned char)*p;
        if (c == ' ' || c == '-' || c == '\'' || c == '.' ||
            (c >= 0x80)) continue;
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))) return false;
    }
    return true;
}
