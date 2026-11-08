#ifndef UNICODOID_CLDR_SUPPLEMENTAL_H
#define UNICODOID_CLDR_SUPPLEMENTAL_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cldr_supplemental.h
 * @brief UTS #35 Supplemental (day periods, time zones, units).
 *
 * Supplemental-data helpers: day-period (AM/PM) names and zone IDs.
 * Time-zone databases are external; this module validates zone IDs.
 */

/** True when @p zone is a CLDR zone ID (IANA-style). */
bool unicodoid_cldr_is_zone_id(const char* zone);

/** Day-period name ("AM"/"PM") for @p hour (0..23), or "at night" otherwise. */
const char* unicodoid_cldr_day_period(int hour);

/** Number of day periods (4: morning/afternoon/evening/night + AM/PM). */
size_t unicodoid_cldr_day_period_count(void);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_CLDR_SUPPLEMENTAL_H */
