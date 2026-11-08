#ifndef UNICODOID_UTR33_CONFORMANCE_H
#define UNICODOID_UTR33_CONFORMANCE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file utr33_conformance.h
 * @brief UTR #33 Conformance Model (Revision 7).
 *
 * Informative conformance model: levels of conformance and how the SDK
 * maps them to status codes.  No runtime algorithm; this header records
 * the conformance vocabulary used in module documentation.
 */

typedef enum unicodoid_conformance_level {
    UNICODOID_CONFORMANCE_UNSPECIFIED = 0,
    UNICODOID_CONFORMANCE_WELL_FORMED = 1,
    UNICODOID_CONFORMANCE_VALID = 2,
    UNICODOID_CONFORMANCE_STABLE = 3
} unicodoid_conformance_level_t;

/** Human-readable name for a conformance level. */
const char* unicodoid_utr33_level_name(int level);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_UTR33_CONFORMANCE_H */
