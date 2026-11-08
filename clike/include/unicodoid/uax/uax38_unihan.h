#ifndef UNICODOID_UAX38_UNIHAN_H
#define UNICODOID_UAX38_UNIHAN_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file uax38_unihan.h
 * @brief UAX #38 Unicode Han Database (Unihan) (Unicode 18.0.0, Rev. 41).
 *
 * Runtime access to the Unihan-derived structure that is present in the
 * generated UCD tables: Unified_Ideograph, Radical, and the CJK blocks.
 * The full Unihan property store (k*, u*, etc. from Unihan.zip) is not in
 * the generated tables; unicodoid_uax38_unihan_field() reports those
 * fields as absent (false) with the data-file name so callers can parse
 * Unihan.txt themselves.  Block-based membership helpers cover the
 * interoperable core used by dictionaries and fonts.
 */

/** True when @p cp is a Unified_Ideograph (Unihan repertoire). */
bool unicodoid_uax38_is_unified_ideograph(uint32_t cp);

/** True when @p cp is a CJK Radical (U+2F00..U+2FDF or U+2E80..U+2FFF). */
bool unicodoid_uax38_is_radical(uint32_t cp);

/** True when @p cp lies in a CJK block (Blocks.txt). */
bool unicodoid_uax38_is_cjk_block(uint32_t cp);

/**
 * @brief Look up a Unihan field value.
 * @param field NUL-terminated field name (e.g. "kDefinition").
 * @param out   buffer for the value text (UTF-8), or NULL to query length.
 * @param cap   capacity of @p out bytes.
 * The generated tables do not carry the Unihan field store, so this returns false
 * and sets *out_len to 0 (see scope note above).
 */
bool unicodoid_uax38_unihan_field(
    uint32_t cp, const char* field, char* out, size_t cap, size_t* out_len);

/**
 * @brief Radical index of @p cp (CJKRadicals.txt numbering, 1-based),
 *        or 0 when @p cp is not a radical.
 */
int unicodoid_uax38_radical_index(uint32_t cp);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_UAX38_UNIHAN_H */
