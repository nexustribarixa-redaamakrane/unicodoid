#ifndef UNICODOID_CLDR_MESSAGEFORMAT_H
#define UNICODOID_CLDR_MESSAGEFORMAT_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cldr_messageformat.h
 * @brief UTS #35 MessageFormat (MessageFormat 2.0 resource bundles).
 *
 * MF2 data-model helpers: plural-category tables and pattern
 * well-formedness (balanced braces).  Rendering is out of scope.
 */

typedef enum unicodoid_cldr_plural {
    UNICODOID_CLDR_PLURAL_OTHER = 0,
    UNICODOID_CLDR_PLURAL_ONE,
    UNICODOID_CLDR_PLURAL_TWO,
    UNICODOID_CLDR_PLURAL_FEW,
    UNICODOID_CLDR_PLURAL_MANY,
    UNICODOID_CLDR_PLURAL_ZERO
} unicodoid_cldr_plural_t;

/** Plural category of @p n for @p locale (English + root rules). */
int unicodoid_cldr_plural(const char* locale, double n);

/** True when @p pattern has balanced `{`/`}` and no stray `}`. */
bool unicodoid_cldr_is_message_pattern_valid(const char* pattern);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_CLDR_MESSAGEFORMAT_H */
