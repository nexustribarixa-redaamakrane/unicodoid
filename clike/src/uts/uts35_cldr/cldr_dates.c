#include "unicodoid/uts/uts35_cldr/cldr_dates.h"

#include <string.h>

const char* unicodoid_cldr_calendar(const char* locale) {
    if (locale && (strstr(locale, "th") == locale || strstr(locale, "TH") == locale))
        return "buddhist";
    if (locale && (strstr(locale, "zh") == locale || strstr(locale, "ZH") == locale))
        return "chinese";
    return "gregory";
}

int unicodoid_cldr_first_weekday(const char* locale) {
    if (locale && strcmp(locale, "US") == 0) return 0; /* Sunday for US default */
    return 1; /* Monday for most locales */
}

bool unicodoid_cldr_is_date_pattern(const char* pattern) {
    if (!pattern || !*pattern) return false;
    for (const char* p = pattern; *p; ++p) {
        char c = *p;
        bool ok = (c == 'y' || c == 'M' || c == 'd' || c == 'H' || c == 'm' ||
                   c == 's' || c == 'E' || c == 'a' || c == 'B' || c == 'Z' ||
                   c == 'z' || c == 'G' || c == 'u' || c == 'L' || c == 'c' ||
                   c == 'e' || c == 'F' || c == 'g' || c == 'j' || c == 'J' ||
                   c == 'k' || c == 'K' || c == 'n' || c == 'O' || c == 'Q' ||
                   c == 'r' || c == 'S' || c == 't' || c == 'U' || c == 'V' ||
                   c == 'v' || c == 'w' || c == 'W' || c == 'x' || c == 'X' ||
                   c == 'Y' || c == '/' || c == '-' || c == ':' || c == ' ' ||
                   c == '\'' || c == '.' || c == ',');
        if (!ok) return false;
    }
    return true;
}
