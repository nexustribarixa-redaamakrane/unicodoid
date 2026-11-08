#ifndef UNICODOID_UTR56_CUNEIFORM_SIGNLISTS_H
#define UNICODOID_UTR56_CUNEIFORM_SIGNLISTS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file utr56_cuneiform_signlists.h
 * @brief UTR #56 Unicode Cuneiform Sign Lists (Revision 5).
 *
 * Sign-list repertoire for Cuneiform (U+12000..U+1247F, plus extensions).
 * Membership helpers over the generated Script/Block data; the sign-list
 * documents (Borger/Heeß/Zehetner numbers) are references, not runtime
 * tables.
 */

/** True when @p cp is in Cuneiform or its extensions. */
bool unicodoid_utr56_is_cuneiform(uint32_t cp);

/** True when @p cp is a Cuneiform number (Hittite/old Assyrian digits U+12480..U+124FF). */
bool unicodoid_utr56_is_number(uint32_t cp);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_UTR56_CUNEIFORM_SIGNLISTS_H */
