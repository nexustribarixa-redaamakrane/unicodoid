#ifndef UNICODOID_UAX11_EAW_H
#define UNICODOID_UAX11_EAW_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file uax11_eaw.h
 * @brief UAX #11 East Asian Width (Unicode 18.0.0, Revision 46).
 *
 * Thin scalar API over the generated East_Asian_Width data.  Ambiguous
 * characters count as narrow unless the caller opts into wide treatment,
 * following UAX #11's column-count conventions for CJK context.
 */

/** East_Asian_Width values (UAX #11). */
typedef enum unicodoid_eaw_width {
    UNICODOID_EAW_N = 0,  /**< Neutral. */
    UNICODOID_EAW_A = 1,  /**< Ambiguous. */
    UNICODOID_EAW_H = 2,  /**< Halfwidth. */
    UNICODOID_EAW_F = 3,  /**< Fullwidth. */
    UNICODOID_EAW_Na = 4, /**< Narrow. */
    UNICODOID_EAW_W = 5   /**< Wide. */
} unicodoid_eaw_width_t;

/** Raw East_Asian_Width of @p cp (unicodoid_eaw_width_t value). */
uint8_t unicodoid_uax11_eaw(uint32_t cp);

/** True when @p cp is Fullwidth or Wide. */
bool unicodoid_uax11_is_wide(uint32_t cp);

/**
 * @brief Terminal column count of @p cp.
 *
 * W/F count as 2; H/Na/N as 1; A as 1 unless @p ambiguous_as_wide is true.
 */
int unicodoid_uax11_column_count(uint32_t cp, bool ambiguous_as_wide);

/**
 * @brief Column count of a scalar sequence (sum of column counts).
 */
size_t unicodoid_uax11_columns(
    const uint32_t* cps, size_t length, bool ambiguous_as_wide);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_UAX11_EAW_H */
