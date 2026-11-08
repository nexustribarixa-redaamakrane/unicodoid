#ifndef UNICODOID_UAX57_UNIKEMET_H
#define UNICODOID_UAX57_UNIKEMET_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file uax57_unikemet.h
 * @brief UAX #57 Unicode Egyptian Hieroglyph Database (Unikemet)
 *        (Unicode 18.0.0, Revision 7).
 *
 * Unikemet.txt (Unicode 18.0.0) is the normative property store for
 * Egyptian hieroglyphs (kEH_AltSeq, kEH_Cat, kEH_Core, kEH_Desc, kEH_Func,
 * kEH_FVal, kEH_HG, kEH_IFAO, kEH_JSesh, kEH_NoMirror, kEH_NoRotate,
 * kEH_UniK).  The full property file is 1.4 MB and is not in the generated
 * tables; this module covers the interoperable core: repertoire
 * membership (Egyptian blocks) and Core/Legacy classification by block,
 * with field lookup reporting absence (false) so callers can parse the
 * data file themselves for full fidelity.
 */

/** True when @p cp is an Egyptian hieroglyph (U+13000..U+1342F or U+13460..U+143FF). */
bool unicodoid_uax57_is_hieroglyph(uint32_t cp);

/** True when @p cp belongs to the Unikemet Core set (Core block U+13460..U+143FF. For
 *  legacy block characters membership is reported via is_legacy(). */
bool unicodoid_uax57_is_core(uint32_t cp);

/** True when @p cp is a Legacy sign (original block U+13000..U+1342F). */
bool unicodoid_uax57_is_legacy(uint32_t cp);

/** True when @p cp must not be mirrored (kEH_NoMirror = Y).  The full list
 *  lives in Unikemet.txt; the two well-known mirrored walking-leg pairs
 *  U+130BB/U+130BD are covered, otherwise false. */
bool unicodoid_uax57_is_no_mirror(uint32_t cp);

/**
 * @brief Look up a Unikemet field value.
 * @return the required length (0 when the field is not in the core tables).
 */
size_t unicodoid_uax57_unikemet_field(
    uint32_t cp, const char* field, char* out, size_t cap);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_UAX57_UNIKEMET_H */
