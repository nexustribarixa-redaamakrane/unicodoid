#ifndef UNICODOID_UTR17_CHARACTER_ENCODING_MODEL_H
#define UNICODOID_UTR17_CHARACTER_ENCODING_MODEL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file utr17_character_encoding_model.h
 * @brief UTR #17 Unicode Character Encoding Model (Revision 9).
 *
 * Informative model: code points, code units, and the transformation
 * formats (UTF-8/16/32) as interchange encodings.  This module answers
 * the model queries that encoders need: code-unit widths, scalar validity,
 * and replacement-character handling.  Canonical vs. compatibility encoding
 * policy is covered by UAX #15; this module is the encoding-model view.
 */

/** Number of UTF-8 code units for a valid scalar, 0 for invalid. */
int unicodoid_utr17_utf8_length(uint32_t cp);

/** Number of UTF-16 code units for a valid scalar, 0 for invalid. */
int unicodoid_utr17_utf16_length(uint32_t cp);

/** True when @p cp is a replacement candidate (noncharacter or unassigned). */
bool unicodoid_utr17_is_replacement(uint32_t cp);

/** True when @p cp is a valid Unicode scalar value. */
bool unicodoid_utr17_is_valid(uint32_t cp);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_UTR17_CHARACTER_ENCODING_MODEL_H */
