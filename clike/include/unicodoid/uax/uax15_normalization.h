#ifndef UNICODOID_UAX15_NORMALIZATION_H
#define UNICODOID_UAX15_NORMALIZATION_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file uax15_normalization.h
 * @brief UAX #15 Unicode Normalization Forms (Unicode 18.0.0, Rev. 58).
 *
 * Scalar API over the generated decomposition/composition data.  The
 * byte-oriented normalizer (unicodoid_normalization.c) implements the same
 * algorithm; these helpers expose it for callers that already hold scalars:
 * decomposition, composition, canonical ordering, and form checks with the
 * Hangul algorithm.  Compatibility mappings are only applied for NFKC/NFKD.
 */

typedef enum unicodoid_norm_form_s {
    UNICODOID_NORM_S_NFC = 0,
    UNICODOID_NORM_S_NFD = 1,
    UNICODOID_NORM_S_NFKC = 2,
    UNICODOID_NORM_S_NFKD = 3
} unicodoid_norm_form_s_t;

/** True when @p cp has a decomposition of the requested kind. */
bool unicodoid_uax15_has_decomposition(uint32_t cp, bool compatibility);

/**
 * @brief Decompose @p cp (single level, without Hangul split).
 * @return scalars written.
 */
size_t unicodoid_uax15_decomposition(uint32_t cp, bool compatibility, uint32_t* out, size_t cap);

/**
 * @brief Normalize a scalar sequence into @p out.
 * @return required length; writes first min(return, cap).
 */
size_t unicodoid_uax15_normalize(const uint32_t* in, size_t length, int form, uint32_t* out, size_t cap);

/** True when @p in is already in the requested normal form. */
bool unicodoid_uax15_is_normalized(const uint32_t* in, size_t length, int form);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_UAX15_NORMALIZATION_H */
