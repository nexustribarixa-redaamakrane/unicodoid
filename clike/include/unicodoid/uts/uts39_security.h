#ifndef UNICODOID_UTS39_SECURITY_H
#define UNICODOID_UTS39_SECURITY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file uts39_security.h
 * @brief UTS #39 Unicode Security Mechanisms (Unicode 18.0.0, Rev. 34).
 *
 * Confusables (confusables.txt) and identifier status.  Skeleton
 * approximation: fold each scalar through the confusable mapping (the
 * generated table) and compare skeletons.  IdentifierStatus /
 * IdentifierType files are distribution data; this module checks the
 * generated IDNA/identifier-adjacent properties (ID_Start etc. via
 * UAX #31) and reports full-file checks as out of scope.
 */

/**
 * @brief Visual confusable skeleton of one scalar.
 * @return required length; writes when buffer fits.
 */
size_t unicodoid_uts39_skeleton(const uint32_t* cps, size_t length, uint32_t* out, size_t cap);

/** True when the skeletons of @p a and @p b are equal. */
bool unicodoid_uts39_are_confusable(
    const uint32_t* a, size_t a_len, const uint32_t* b, size_t b_len);

/** Confusable mapping of one scalar (0 when none). */
size_t unicodoid_uts39_confusable(uint32_t cp, uint32_t* out, size_t cap);

/** True when @p cp is Default_Ignorable_Code_Point (UTS #39 high-risk set). */
bool unicodoid_uts39_is_default_ignorable(uint32_t cp);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_UTS39_SECURITY_H */
