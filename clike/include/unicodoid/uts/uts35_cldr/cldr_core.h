#ifndef UNICODOID_CLDR_CORE_H
#define UNICODOID_CLDR_CORE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cldr_core.h
 * @brief UTS #35 CLDR Part 1: Core (CLDR 49, living document).
 *
 * Locale identifier model (BCP 47): language (ISO 639), script (ISO 15924),
 * region (ISO 3166), variant, and extensions.  This module is the
 * foundation every other CLDR part builds on.
 */

typedef struct unicodoid_cldr_locale {
    char language[8];   /* "und" when absent */
    char script[5];     /* "" when absent */
    char region[4];     /* "" when absent */
    char variant[16];   /* "" when absent */
} unicodoid_cldr_locale_t;

/** Parse a BCP 47 tag into @p out. Returns false on invalid tag. */
bool unicodoid_cldr_parse_locale(const char* tag, unicodoid_cldr_locale_t* out);

/** True when @p tag is a well-formed BCP 47 (CLDR) language tag. */
bool unicodoid_cldr_is_valid_locale(const char* tag);

/** Default locale for @p language (CLDR fallback), or "root". */
const char* unicodoid_cldr_default_locale(const char* language);

/** CLDR version string ("49"). */
const char* unicodoid_cldr_version(void);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_CLDR_CORE_H */
