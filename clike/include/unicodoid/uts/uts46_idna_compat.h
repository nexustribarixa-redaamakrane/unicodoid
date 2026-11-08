#ifndef UNICODOID_UTS46_IDNA_COMPAT_H
#define UNICODOID_UTS46_IDNA_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file uts46_idna_compat.h
 * @brief UTS #46 Unicode IDNA Compatibility Processing (18.0.0, Rev. 36).
 *
 * IDNA processing steps 1..7 (map, NFC, split, validate, convert) using
 * the generated IdnaMappingTable status (0 valid, 1 mapped, 2 deviation,
 * 3 ignored, 4 disallowed).  Punycode (RFC 3492) conversion and full
 * IDNA2008 label validation (XC/IH/IG) are out of scope; this module
 * covers mapping/normalization/disposition per E000..E013.
 */

typedef enum unicodoid_uts46_disposition {
    UNICODOID_UTS46_VALID = 0,
    UNICODOID_UTS46_MAPPED = 1,
    UNICODOID_UTS46_DEVIATION = 2,
    UNICODOID_UTS46_IGNORED = 3,
    UNICODOID_UTS46_DISALLOWED = 4
} unicodoid_uts46_disposition_t;

/** UTS #46 disposition of one scalar (from the generated table). */
int unicodoid_uts46_disposition(uint32_t cp);

/**
 * @brief Map a scalar sequence (UTS #46 steps 4..6: map + NFC).
 * @return required length; writes when buffer fits.  Disallowed scalars
 *         fail the whole label check via unicodoid_uts46_check_label().
 */
size_t unicodoid_uts46_map(const uint32_t* in, size_t length, uint32_t* out, size_t cap);

/** True when @p label survives mapping and contains no disallowed scalars. */
bool unicodoid_uts46_check_label(const uint32_t* label, size_t length);

/** True when @p cp must not appear in a transitional label (NV8 deviation). */
bool unicodoid_uts46_is_transitional_disallowed(uint32_t cp);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_UTS46_IDNA_COMPAT_H */
