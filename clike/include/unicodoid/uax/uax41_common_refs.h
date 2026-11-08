#ifndef UNICODOID_UAX41_COMMON_REFS_H
#define UNICODOID_UAX41_COMMON_REFS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file uax41_common_refs.h
 * @brief UAX #41 Common References for UAXs (Unicode 18.0.0, Revision 38).
 *
 * Documentation-only annex: no runtime algorithm is derived from it.
 * This header records the revision metadata so callers and tools can cite
 * the UAX family uniformly.
 */

/** UAX #41 revision (Unicode 18.0.0). */
const char* unicodoid_uax41_revision(void);

/** Publication date (YYYY-MM-DD) of UAX #41 Revision 38. */
const char* unicodoid_uax41_date(void);

/** Canonical URL of UAX #41. */
const char* unicodoid_uax41_url(void);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_UAX41_COMMON_REFS_H */
