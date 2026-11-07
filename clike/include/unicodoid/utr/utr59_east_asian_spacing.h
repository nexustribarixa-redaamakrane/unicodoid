#ifndef UNICODOID_UTR59_EAST_ASIAN_SPACING_H
#define UNICODOID_UTR59_EAST_ASIAN_SPACING_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file utr59_east_asian_spacing.h
 * @brief UTR #59 East Asian Spacing (Unicode 18.0.0, Revision 1 draft).
 *
 * Implements the informative East_Asian_Spacing property and the spacing
 * algorithm of UTR59 §3.3 against the official derived data file
 * (east-asian-spacing.txt) generated for Unicode 18.0.0.
 *
 * The algorithm operates on extended grapheme clusters (UAX #29).  It does
 * not decide the visual width of the inserted space; callers choose whether
 * to add a glyph advance or a space character such as U+2009.
 */

/** East_Asian_Spacing property values (UTR59 Table 1). */
typedef enum unicodoid_eas_value {
    UNICODOID_EAS_W = 1, /**< Wide: East Asian script character. */
    UNICODOID_EAS_N = 2, /**< Narrow: needs spacing next to W. */
    UNICODOID_EAS_O = 3, /**< Other: no auto-spacing. */
    UNICODOID_EAS_C = 4  /**< Conditional: N for Chinese, else O. */
} unicodoid_eas_value_t;

/** Raw East_Asian_Spacing value for a code point (never resolves C). */
uint8_t unicodoid_utr59_east_asian_spacing(uint32_t cp);

/**
 * @brief Resolve the property value of one extended grapheme cluster.
 *
 * Implements UTR59 §3.3.1 steps 2-7:
 *   - take the value of the first code point,
 *   - force O when the cluster contains an enclosing mark (General_Category
 *     Me),
 *   - resolve C to N when @p chinese is true, otherwise to O,
 *   - in vertical layout, turn N into O when the first code point is displayed
 *     upright (Vertical_Orientation U, Tu or Tr).
 *
 * @param cps     code points of the cluster (at least one),
 * @param length  cluster length,
 * @param chinese true for a Chinese language context,
 * @param vertical true when the text is laid out vertically.
 *
 * @return one of the unicodoid_eas_value_t values (W, N or O).
 */
uint8_t unicodoid_utr59_cluster_value(
    const uint32_t* cps, size_t length, bool chinese, bool vertical);

/**
 * @brief Compute where auto-spacing is inserted.
 *
 * Iterates extended grapheme clusters and records every boundary between a
 * W cluster and an N cluster (in either order).  Each recorded offset is the
 * scalar index of the first code point of the following cluster, i.e. a
 * space belongs immediately before @c cps[offset].
 *
 * @param cps      input scalar sequence,
 * @param length   number of code points,
 * @param chinese  Chinese language context,
 * @param vertical vertical text layout,
 * @param out      destination buffer for offsets, or NULL to count only,
 * @param cap      capacity of @p out.
 *
 * @return the total number of insertion points; when @p out is non-NULL the
 *         first min(return, cap) offsets are written.
 */
size_t unicodoid_utr59_spacing_offsets(
    const uint32_t* cps, size_t length,
    bool chinese, bool vertical,
    size_t* out, size_t cap);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_UTR59_EAST_ASIAN_SPACING_H */
