#ifndef UNICODOID_CLDR_GENERAL_H
#define UNICODOID_CLDR_GENERAL_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cldr_general.h
 * @brief UTS #35 General (days, months, units display names).
 *
 * Display-name sizes and unit helpers.  Full display-name tables come
 * from CLDR XML; this module reports the size vocabulary and checks
 * day/month/weekday names for the supported locales.
 */

typedef enum unicodoid_cldr_display_size {
    UNICODOID_CLDR_DISPLAY_NARROW = 0,
    UNICODOID_CLDR_DISPLAY_SHORT = 1,
    UNICODOID_CLDR_DISPLAY_WIDE = 2
} unicodoid_cldr_display_size_t;

/** True when @p text is a weekday name for the locale (wide English core). */
bool unicodoid_cldr_is_weekday(const char* locale, const char* text);

/** Month count (always 12 for Gregorian CLDR). */
size_t unicodoid_cldr_month_count(const char* locale);

/** Day count in @p month (1..12) for February-leap approximation (Feb=29 leap). */
size_t unicodoid_cldr_day_count(const char* locale, int month, bool leap);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_CLDR_GENERAL_H */
