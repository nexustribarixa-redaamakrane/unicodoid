#ifndef UNICODOID_UAX24_SCRIPT_PROPERTY_H
#define UNICODOID_UAX24_SCRIPT_PROPERTY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file uax24_script_property.h
 * @brief UAX #24 Unicode Script Property (Unicode 18.0.0, Revision 41).
 *
 * Scalar API over the generated Script property data (Scripts.txt).  Values
 * are the generated script indices; use unicodoid_uax24_script_name() for
 * the four-letter Script value (e.g. "Latn").  Script_Extensions data is not
 * in the generated tables; unicodoid_uax24_script_extensions() falls back
 * to the primary Script value for the core East-Asian and common scripts
 * and is documented as partial (see Security/UTS #39 for full use).
 */

/** Number of known Script values (including Unknown at index 0). */
size_t unicodoid_uax24_count(void);

/** Four-letter Script name for @p value, or NULL when out of range. */
const char* unicodoid_uax24_script_name(uint16_t value);

/** Script value of @p cp. */
uint16_t unicodoid_uax24_script(uint32_t cp);

/** Find a Script value by name; returns 0 (Unknown) when absent. */
uint16_t unicodoid_uax24_find(const char* name);

/** True when @p cp belongs to Script value @p script. */
bool unicodoid_uax24_is_script(uint32_t cp, uint16_t script);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_UAX24_SCRIPT_PROPERTY_H */
