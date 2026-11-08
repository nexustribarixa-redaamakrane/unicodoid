#ifndef UNICODOID_UTS37_IVD_H
#define UNICODOID_UTS37_IVD_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file uts37_ivd.h
 * @brief UTS #37 Unicode Ideographic Variation Database (Revision 16).
 *
 * IVD records ideographic variation sequences (IVS): base character plus
 * variation selector (U+E0100..U+E01EF).  The IVD file (ivd.txt) is not in
 * the generated tables; this module provides the IVS well-formedness
 * check and the selector range helpers that callers use to validate IVD
 * entries without the full file.  Full IVD lookup requires ivd.txt.
 */

/** True when @p cp is an ideographic variation selector (U+E0100..U+E01EF). */
bool unicodoid_uts37_is_variation_selector(uint32_t cp);

/** True when @p cps is an IVS: exactly two scalars, second a variation selector. */
bool unicodoid_uts37_is_ivs(const uint32_t* cps, size_t length);

/** Selector offset (0-based) of @p cp, or -1 when not a selector. */
int unicodoid_uts37_selector_offset(uint32_t cp);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_UTS37_IVD_H */
