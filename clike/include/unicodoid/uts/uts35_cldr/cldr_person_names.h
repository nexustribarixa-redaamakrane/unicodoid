#ifndef UNICODOID_CLDR_PERSON_NAMES_H
#define UNICODOID_CLDR_PERSON_NAMES_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cldr_person_names.h
 * @brief UTS #35 Person Names (personal name formatting).
 *
 * Name-order and formality helpers.  Locale tables decide order; this
 * module reports the CLDR name-order vocabulary and validates simple
 * person-name strings (letters, marks, hyphen, space).
 */

typedef enum unicodoid_cldr_name_order {
    UNICODOID_CLDR_NAME_ORDER_FAMILY_FIRST = 0,
    UNICODOID_CLDR_NAME_ORDER_GIVEN_FIRST = 1
} unicodoid_cldr_name_order_t;

/** Name order for @p locale (family-first for CJK/Korean/Hungarian). */
int unicodoid_cldr_name_order(const char* locale);

/** True when @p name looks like a personal name (ASCII letters/marks/space). */
bool unicodoid_cldr_is_person_name(const char* locale, const char* name);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_CLDR_PERSON_NAMES_H */
