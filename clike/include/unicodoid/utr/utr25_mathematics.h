#ifndef UNICODOID_UTR25_MATHEMATICS_H
#define UNICODOID_UTR25_MATHEMATICS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file utr25_mathematics.h
 * @brief UTR #25 Unicode Support for Mathematics (Revision 16, Draft 5).
 *
 * Mathematical alphanumeric symbols and the Math binary property.  The
 * generated Math property (DerivedCoreProperties) is the source of truth;
 * this module adds the alphanumeric range checks (Mathematical Alphanumeric
 * block U+1D400..U+1D7FF) and the NFKC math folding note.
 */

/** True when @p cp has Math = Yes. */
bool unicodoid_utr25_is_math(uint32_t cp);

/** True when @p cp is in the Mathematical Alphanumeric Symbols block. */
bool unicodoid_utr25_is_math_alphanumeric(uint32_t cp);

/** True when @p cp is a mathematical operator (Math + Sm/Sc category). */
bool unicodoid_utr25_is_operator(uint32_t cp);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_UTR25_MATHEMATICS_H */
