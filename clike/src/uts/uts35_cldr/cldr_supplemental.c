#include "unicodoid/uts/uts35_cldr/cldr_supplemental.h"

#include <string.h>

bool unicodoid_cldr_is_zone_id(const char* zone) {
    if (!zone || !*zone) return false;
    return strchr(zone, '/') != NULL || strcmp(zone, "UTC") == 0;
}

const char* unicodoid_cldr_day_period(int hour) {
    if (hour < 0 || hour > 23) return "at night";
    if (hour < 12) return "AM";
    return "PM";
}

size_t unicodoid_cldr_day_period_count(void) { return 4; }
