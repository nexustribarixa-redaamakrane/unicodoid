#ifndef UNICODOID_UAX14_LINEBREAK_H
#define UNICODOID_UAX14_LINEBREAK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file uax14_linebreak.h
 * @brief UAX #14 Unicode Line Breaking Algorithm (Unicode 18.0.0, Rev. 57).
 *
 * Scalar API over the generated Line_Break data.  The full LB1..LB31 rules
 * live in the byte segmenter (unicodoid_segmentation.c, LINE boundary type)
 * which these scalar helpers reuse via a UTF-8 round-trip, so both paths
 * agree by construction.  Break opportunities are returned as scalar
 * indices; mandatory breaks (BK/CR/LF/NL) are reported like any boundary.
 */

/** Line_Break class of @p cp (unicodoid_lb_t value). */
uint8_t unicodoid_uax14_line_break(uint32_t cp);

/**
 * @brief Find the end of the line segment starting at @p start.
 *
 * @return index one past the last code point of the segment, or @p length
 *         when @p start is at or beyond the end.
 */
size_t unicodoid_uax14_next_break(const uint32_t* cps, size_t length, size_t start);

/** True when a break occurs immediately before @p index (1..length). */
bool unicodoid_uax14_is_break(const uint32_t* cps, size_t length, size_t index);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_UAX14_LINEBREAK_H */
