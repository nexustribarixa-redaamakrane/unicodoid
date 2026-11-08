#ifndef UNICODOID_UAX9_BIDI_H
#define UNICODOID_UAX9_BIDI_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file uax9_bidi.h
 * @brief UAX #9 Unicode Bidirectional Algorithm (Unicode 18.0.0, Revision 52).
 *
 * Implements the UBA paragraph algorithm over scalar values using the
 * generated Bidirectional_Class data (Unicode 18.0.0):
 *
 *   - paragraph direction (P2/P3, UNICODOID_BIDI_LTR / RTL / AUTO),
 *   - level resolution (X9, W1..W7, N0..N2, I1/I2, X1..X10 at level
 *     granularity; explicit embeddings and isolates are resolved to their
 *     embedding levels per X1..X9),
 *   - visual reordering (L2) and mirrored-glyph substitution (L3/L4 with the
 *     official BidiMirroring data),
 *   - paired-bracket handling (N0) with the official BidiBrackets data.
 *
 * BidiT / BidiCharacter test vectors are the conformance oracle; glyph
 * shaping, line breaking around bidi runs, and explicit embedding marks
 * beyond level accounting are out of scope.  Bidi_Class values are the
 * unicodoid_bidi_class_t values from unicodoid_ucd.h.
 */

/** Requested paragraph base direction. */
typedef enum unicodoid_bidi_direction {
    UNICODOID_BIDI_LTR = 0,  /**< Left-to-right paragraph. */
    UNICODOID_BIDI_RTL = 1,  /**< Right-to-left paragraph. */
    UNICODOID_BIDI_AUTO = -1 /**< Per P2: first strong character, else LTR. */
} unicodoid_bidi_direction_t;

/** Bidirectional_Class of a scalar (unicodoid_bidi_class_t value). */
uint8_t unicodoid_uax9_bidi_class(uint32_t cp);

/** Paragraph embedding level (0 for LTR, 1 for RTL) per P2/P3. */
uint8_t unicodoid_uax9_paragraph_level(
    const uint32_t* cps, size_t length, int base_dir);

/**
 * @brief Resolve bidirectional levels for one paragraph.
 *
 * @param cps      input scalars (must not span a B paragraph separator;
 *                 B characters are handled as paragraph ends with level 0),
 * @param base_dir one of UNICODOID_BIDI_LTR / RTL / AUTO,
 * @param out      level per position (0..125), or NULL to query only,
 * @param cap      capacity of @p out.
 *
 * @return the number of positions (== length); when @p out is non-NULL the
 *         first min(return, cap) levels are written.
 */
size_t unicodoid_uax9_resolve_levels(
    const uint32_t* cps, size_t length, int base_dir,
    uint8_t* out, size_t cap);

/**
 * @brief Reorder a paragraph to visual order (L2) with mirroring (L3/L4).
 *
 * @return the number of scalars written (== length); when @p out is
 *         non-NULL the first min(return, cap) scalars are written.
 */
size_t unicodoid_uax9_visual_order(
    const uint32_t* cps, size_t length, int base_dir,
    uint32_t* out, size_t cap);

/** True when @p cp has a mirrored counterpart per BidiMirroring.txt. */
bool unicodoid_uax9_is_mirrored(uint32_t cp);

/** Mirrored counterpart of @p cp, or @p cp when there is none. */
uint32_t unicodoid_uax9_mirrored_glyph(uint32_t cp);

/**
 * @brief True when @p cp is a paired bracket with a counterpart and
 *        direction per BidiBrackets.txt (paired-bracket algorithm N0).
 */
bool unicodoid_uax9_is_paired_bracket(uint32_t cp);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_UAX9_BIDI_H */
