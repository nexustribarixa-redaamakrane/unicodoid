#ifndef UNICODOID_UAX29_SEGMENTATION_H
#define UNICODOID_UAX29_SEGMENTATION_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file uax29_segmentation.h
 * @brief UAX #29 Unicode Text Segmentation (Unicode 18.0.0, Revision 47).
 *
 * Scalar-based extended grapheme cluster boundaries.  This is the shared
 * primitive used by the UTR #59 East Asian spacing algorithm and is also
 * exposed for callers that already hold decoded scalar values.
 *
 * The byte-oriented segmenter in unicodoid_segmentation.h uses the same
 * rules through its own iterator; this module provides the equivalent
 * boundary primitive without requiring an encoding system.
 */

/**
 * @brief Find the end of the extended grapheme cluster starting at @p start.
 *
 * Applies the extended grapheme cluster boundary rules GB3..GB13 and GB999 of
 * UAX #29 against the generated Unicode 18.0.0 Grapheme_Cluster_Break,
 * Indic_Conjunct_Break and Extended_Pictographic data.
 *
 * @return the index one past the last code point of the cluster.  Returns
 *         @p length when @p start is at or beyond the end.
 */
size_t unicodoid_uax29_next_grapheme(
    const uint32_t* cps, size_t length, size_t start);

/**
 * @brief True when a boundary occurs immediately before @p index.
 *
 * @p index must be in the range 1..length; index 0 is always a boundary and
 * index == length is always a boundary.
 */
bool unicodoid_uax29_is_grapheme_boundary(
    const uint32_t* cps, size_t length, size_t index);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_UAX29_SEGMENTATION_H */
