#ifndef UNICODOID_CLDR_DATES_H
#define UNICODOID_CLDR_DATES_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cldr_dates.h
 * @brief UTS #35 Dates (calendar and date/time pattern helpers).
 *
 * Calendar identifier and week-start tables (CLDR supplemental).  Pattern
 * syntax (yMdHms) is reported but not rendered here.
 */

typedef struct unicodoid_cldr_date_format {
    const char* calendar;  /* gregory, buddhist, chinese, ... */
    const char* pattern;   /* e.g. "yyyy-MM-dd" */
} unicodoid_cldr_date_format_t;

/** Calendar name for @p locale (defaults to gregory). */
const char* unicodoid_cldr_calendar(const char* locale);

/** First day of week (0=Sunday..6=Saturday) for @p locale. */
int unicodoid_cldr_first_weekday(const char* locale);

/** True when @p pattern is a CLDR date pattern (only yMdhmsE/a/B/Z/z). */
bool unicodoid_cldr_is_date_pattern(const char* pattern);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_CLDR_DATES_H */
