#ifndef UNICODOID_UAX53_ARABIC_MARK_RENDERING_H
#define UNICODOID_UAX53_ARABIC_MARK_RENDERING_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file uax53_arabic_mark_rendering.h
 * @brief UAX #53 Unicode Arabic Mark Rendering (Unicode 18.0.0, Rev. 13).
 *
 * Joining behavior for Arabic shaping: Joining_Type / Joining_Group
 * (DerivedJoiningType.txt) plus transparent-mark handling (General_Category
 * Mn/Me/Cf join as Transparent).  This module answers the shaping queries
 * UAX #53 defines: joining class, whether a mark is transparent (joins
 * through), and whether a sequence's joining is consistent (no isolated
 * Dual_Joining characters forced apart).
 */

/** Joining_Type of @p cp (unicodoid_joining_type_t value: U,C,D,L,R,T). */
uint8_t unicodoid_uax53_joining_type(uint32_t cp);

/** True when @p cp joins transparently (Mn, Me or Cf with Joining_Type Transparent). */
bool unicodoid_uax53_is_joining_transparent(uint32_t cp);

/**
 * @brief Shaping integrity check: every Dual-Joining (L/R/D) character
 *        except at an RTL boundary joins its neighbour (transparent marks
 *        are skipped over).  Returns false on a shaping break or an
 *        invalid scalar.
 */
bool unicodoid_uax53_is_consistent(const uint32_t* cps, size_t length);

/** Joining group identifier (e.g. for U+0627) as Script-derived value, 0 when none. */
uint16_t unicodoid_uax53_joining_group(uint32_t cp);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_UAX53_ARABIC_MARK_RENDERING_H */
