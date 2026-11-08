#ifndef UNICODOID_CLDR_COLLATION_H
#define UNICODOID_CLDR_COLLATION_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cldr_collation.h
 * @brief UTS #35 Collation (CLDR collation tailoring keys).
 *
 * Collation tailoring is expressed through UCA weights (UTS #10).  This
 * helper reports the default tailoring for a locale (strength and
 * alternate) that Unicodoid's collation layer consumes.
 */

typedef struct unicodoid_cldr_collation {
    int strength;   /* 1 primary..4 tertiary */
    int alternate;  /* 0 non-ignorable, 1 shifted */
} unicodoid_cldr_collation_t;

/** Default collation settings for @p locale. */
void unicodoid_cldr_collation(const char* locale, unicodoid_cldr_collation_t* out);

/** True when @p locale needs the shifted alternate (e.g. zh, sv). */
bool unicodoid_cldr_needs_shifted(const char* locale);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_CLDR_COLLATION_H */
