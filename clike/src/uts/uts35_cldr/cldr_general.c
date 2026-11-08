#include "unicodoid/uts/uts35_cldr/cldr_general.h"

#include <string.h>

static const char* k_weekdays_en[] = {
    "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
};

bool unicodoid_cldr_is_weekday(const char* locale, const char* text) {
    (void)locale;
    if (!text) return false;
    for (size_t i = 0; i < 7; ++i) if (strcmp(k_weekdays_en[i], text) == 0) return true;
    return false;
}

size_t unicodoid_cldr_month_count(const char* locale) {
    (void)locale;
    return 12;
}

size_t unicodoid_cldr_day_count(const char* locale, int month, bool leap) {
    (void)locale;
    static const size_t k_days[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if (month < 1 || month > 12) return 0;
    if (month == 2 && leap) return 29;
    return k_days[month - 1];
}
