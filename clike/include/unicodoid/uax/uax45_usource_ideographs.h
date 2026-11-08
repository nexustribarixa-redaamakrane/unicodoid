#ifndef UNICODOID_UAX45_USOURCE_IDEOGRAPHS_H
#define UNICODOID_UAX45_USOURCE_IDEOGRAPHS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file uax45_usource_ideographs.h
 * @brief UAX #45 U-source Ideographs (Unicode 18.0.0, Revision 34).
 *
 * U-source (USourceData.txt) records ideographs that were encoded from the
 * Unicode sources project (US1, US22, US248, US685 and per-block US* tags).
 * The generated tables carry the RFC-like source metadata via the U-source
 * records in UnicodeData.txt comments; this module exposes membership and
 * the US-source tag classification.  Full USourceData.txt field parsing
 * (exact source strings) is out of scope; membership covers the
 * interoperable core.
 */

/** True when @p cp is a U-source ideograph (Unified_Ideograph in the CJK blocks). */
bool unicodoid_uax45_is_usource(uint32_t cp);

/** US-source family name for @p cp ("US" for CJK, "US1" when in the US1 block, else NULL). */
const char* unicodoid_uax45_usource_family(uint32_t cp);

/** True when @p cp is in the U-source PUA staging area (U+F900..U+FAFF legacy US1). */
bool unicodoid_uax45_is_us1_staging(uint32_t cp);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_UAX45_USOURCE_IDEOGRAPHS_H */
