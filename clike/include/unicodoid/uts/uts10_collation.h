#ifndef UNICODOID_UTS10_COLLATION_H
#define UNICODOID_UTS10_COLLATION_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file uts10_collation.h
 * @brief UTS #10 Unicode Collation Algorithm (Unicode 18.0.0, Revision 55).
 *
 * Implements the Default Unicode Collation Element Table (DUCET) for Unicode
 * 18.0.0, in its NON_IGNORABLE variable-weighting configuration:
 *
 *   - NFD normalization as the mandatory first step (S1.1),
 *   - collation element construction with longest contiguous matches and
 *     discontiguous non-starter extension with blocking (S2.1-S2.5),
 *   - three-level sort key formation with 16-bit weights (S3.1-S3.9).
 *
 * Not implemented: backward secondary level, SHIFTED/NON-IGNORABLE tailoring
 * variants beyond the default, semi-stability level, script reordering and
 * other CLDR tailorings, matching/searching (§11).  The DUCET weights are
 * taken directly from the generated Unicode 18.0.0 tables.
 */

/** A unit of a sort key (16-bit weights plus the level separator zero). */
typedef uint32_t unicodoid_uts10_unit_t;

/**
 * @brief Build the sort key for a scalar sequence.
 *
 * @param cps     the input code points (any normalization form),
 * @param length  number of code points,
 * @param out     destination buffer, or NULL to query the required length,
 * @param cap     capacity of @p out.
 *
 * @return the number of units required/written; a sort key carries no NUL
 *         terminator (compare units [0, returned) lexicographically).
 */
size_t unicodoid_uts10_sortkey(
    const uint32_t* cps, size_t length,
    unicodoid_uts10_unit_t* out, size_t cap);

/**
 * @brief Compare two scalar strings under the default UCA.
 *
 * @return -1 when a sorts before b, 0 when they compare equal, 1 when a sorts
 *         after b.
 */
int unicodoid_uts10_compare(
    const uint32_t* a, size_t a_len,
    const uint32_t* b, size_t b_len);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_UTS10_COLLATION_H */
