#include "unicodoid/uts/uts35_cldr/cldr_numbers.h"

#include <string.h>

void unicodoid_cldr_number_symbols(const char* region, unicodoid_cldr_number_symbols_t* out) {
    if (!out) return;
    if (region && strcmp(region, "DE") == 0) {
        strcpy(out->group, "."); strcpy(out->decimal, ",");
    } else if (region && strcmp(region, "FR") == 0) {
        strcpy(out->group, " "); strcpy(out->decimal, ",");
    } else {
        strcpy(out->group, ","); strcpy(out->decimal, ".");
    }
    strcpy(out->minus, "-");
    strcpy(out->percent, "%");
}

bool unicodoid_cldr_is_decimal(const char* text) {
    if (!text || !*text) return false;
    int dots = 0;
    for (const char* p = text; *p; ++p) {
        if (*p >= '0' && *p <= '9') continue;
        if (*p == '.' && dots == 0) { dots = 1; continue; }
        return false;
    }
    return true;
}
