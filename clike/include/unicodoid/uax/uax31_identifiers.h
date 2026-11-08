#ifndef UNICODOID_UAX31_IDENTIFIERS_H
#define UNICODOID_UAX31_IDENTIFIERS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file uax31_identifiers.h
 * @brief UAX #31 Unicode Identifiers and Syntax (Unicode 18.0.0, Rev. 45).
 *
 * Derived identifier syntax: the XID subset (XID_Start / XID_Continue /
 * XID_Default_Ignorable_Code_Point) is the default for programming
 * languages.  This module exposes those derived properties and validates
 * whole identifiers against them (XID_Start (XID_Continue | ZWNJ | ZWJ)*).
 * Line_Separator / Paragraph_Separator termination, NFKC normalization
 * checks, and language-specific tails are out of scope (see UTS #39 for
 * security checks on top of these).
 */

/** True when @p cp is XID_Start. */
bool unicodoid_uax31_is_xid_start(uint32_t cp);

/** True when @p cp is XID_Continue. */
bool unicodoid_uax31_is_xid_continue(uint32_t cp);

/** True when @p cp is ID_Start. */
bool unicodoid_uax31_is_id_start(uint32_t cp);

/** True when @p cp is ID_Continue. */
bool unicodoid_uax31_is_id_continue(uint32_t cp);

/**
 * @brief Validate an identifier.
 *
 * Non-empty, starts with XID_Start, continues with XID_Continue, U+200C
 * (ZWNJ) or U+200D (ZWJ).  Surrogates and out-of-range values are rejected.
 */
bool unicodoid_uax31_is_valid(const uint32_t* cps, size_t length);

/** True when @p cps is NFKC-stable (UAX #31 stability check). */
bool unicodoid_uax31_is_nfkc_stable(const uint32_t* cps, size_t length);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_UAX31_IDENTIFIERS_H */
