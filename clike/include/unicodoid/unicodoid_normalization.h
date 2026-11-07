#ifndef UNICODOID_NORMALIZATION_H
#define UNICODOID_NORMALIZATION_H

#include "unicodoid_string.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file unicodoid_normalization.h
 * @brief Partial UAX #15 normalization for Unicode-compatible systems.
 *
 * This module is opt-in: the system must expose a Unicode profile.  The
 * implementation covers canonical decomposition/composition, compatibility
 * mappings for a documented subset, and canonical ordering of combining marks.
 * It does not claim full UAX #15 conformance against the complete UCD.
 */

typedef enum unicodoid_norm_form {
    UNICODOID_NORM_NFC   = 0, /**< Canonical decomposition, canonical composition */
    UNICODOID_NORM_NFD   = 1, /**< Canonical decomposition */
    UNICODOID_NORM_NFKC  = 2, /**< Compatibility decomposition, canonical composition */
    UNICODOID_NORM_NFKD  = 3, /**< Compatibility decomposition */
    UNICODOID_NORM_CUSTOM = 4 /**< Reserved for system-defined forms */
} unicodoid_norm_form_t;

/**
 * @brief Normalize a byte buffer into another buffer.
 *
 * @param out_written On UNICODOID_ERR_BUFFER_TOO_SMALL, receives the required
 *                    number of bytes.
 */
unicodoid_status_t unicodoid_normalize(
    const unicodoid_system_t* sys,
    const void* in_buf,
    size_t in_len,
    unicodoid_norm_form_t form,
    void* out_buf,
    size_t out_cap,
    size_t* out_written
);

/** Normalize into a newly allocated owned string. */
unicodoid_status_t unicodoid_normalize_string(
    const unicodoid_string_t* src,
    unicodoid_norm_form_t form,
    unicodoid_string_t* out
);

/** Check whether a buffer is already in the requested normalization form. */
bool unicodoid_is_normalized(
    const unicodoid_system_t* sys,
    const void* in_buf,
    size_t in_len,
    unicodoid_norm_form_t form
);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_NORMALIZATION_H */
