#ifndef UNICODOID_UTS55_SOURCE_CODE_H
#define UNICODOID_UTS55_SOURCE_CODE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file uts55_source_code.h
 * @brief UTS #55 Unicode Source Code Handling (Revision 5).
 *
 * Source-code safety checks: Bidi embedding/override marks in comments
 * and identifiers, Default_Ignorable_Code_Point in source, and homoglyph
 * warnings.  This module flags the characters UTS #55 calls out for
 * sanitization in code (bidi controls, ZWJ/ZWNJ outside joins).  Full
 * language-grammar handling is out of scope.
 */

/** True when @p cp is a Bidi control (Emb/Override/Isolate). */
bool unicodoid_uts55_is_bidi_control(uint32_t cp);

/** True when @p cp is ZWJ or ZWNJ (source-sensitive joiners). */
bool unicodoid_uts55_is_joiner(uint32_t cp);

/** Number of UTS #55 flagged characters in @p cps (bidi controls + lone joiners). */
size_t unicodoid_uts55_flag_count(const uint32_t* cps, size_t length);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_UTS55_SOURCE_CODE_H */
