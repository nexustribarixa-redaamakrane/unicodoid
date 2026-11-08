#ifndef UNICODOID_UTR54_MONGOLIAN_H
#define UNICODOID_UTR54_MONGOLIAN_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file utr54_mongolian.h
 * @brief UTR #54 Unicode Mongolian 12.1 Baseline (Revision 1).
 *
 * Baseline repertoire and shaping rules for Mongolian (U+1800..U+18AF)
 * as fixed by the 12.1 baseline: Mongolian Free Variation Selectors
 * (U+180B..U+180D, U+180F), Mongolian vowel separator (U+180E, since
 * reclassified), and the block membership helpers.  Full baseline
 * shaping is a font/layout concern; this module is the repertoire view.
 */

/** True when @p cp is in the Mongolian block (U+1800..U+18AF). */
bool unicodoid_utr54_is_mongolian(uint32_t cp);

/** True when @p cp is a Mongolian Free Variation Selector. */
bool unicodoid_utr54_is_fvs(uint32_t cp);

/** True when @p cp is the Mongolian Vowel Separator. */
bool unicodoid_utr54_is_mvs(uint32_t cp);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_UTR54_MONGOLIAN_H */
