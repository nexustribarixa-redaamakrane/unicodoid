#ifndef UNICODOID_UTS61_SET_NOTATION_H
#define UNICODOID_UTS61_SET_NOTATION_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file uts61_set_notation.h
 * @brief UTS #61 Unicode Set Notation (Version 1, Draft 4).
 *
 * Draft set-notation syntax for character sets: \p{...}, \P{...},
 * ranges \p{Lu}.. and property sets.  This module implements the
 * draft core: membership test for a single \p{...} set expression and
 * range-set parsing for "A..Z" style spans.  Full UnicodeSet algebra
 * (&&, nested sets) is out of scope.
 */

/**
 * @brief Membership of @p cp in the set expression @p set.
 * @return 1 in, 0 out, -1 on parse error.
 */
int unicodoid_uts61_set_membership(const char* set, uint32_t cp);

/**
 * @brief Parse "X..Y" range text (hex scalars separated by "..").
 * @return 1 when parsed, 0 otherwise.
 */
int unicodoid_uts61_parse_range(const char* text, uint32_t* lo, uint32_t* hi);

/** True when @p cp lies in [lo, hi]. */
bool unicodoid_uts61_in_range(uint32_t cp, uint32_t lo, uint32_t hi);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_UTS61_SET_NOTATION_H */
