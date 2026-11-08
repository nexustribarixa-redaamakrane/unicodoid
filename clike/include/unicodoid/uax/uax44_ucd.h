#ifndef UNICODOID_UAX44_UCD_H
#define UNICODOID_UAX44_UCD_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file uax44_ucd.h
 * @brief UAX #44 Unicode Character Database (Unicode 18.0.0, Revision 38).
 *
 * The UCD specification itself: property model conventions, file formats,
 * and alias files (PropertyAliases.txt, PropertyValueAliases.txt).  The
 * generated tables behind unicodoid_ucd.h already follow UAX #44's range
 * and default conventions; this module exposes the alias layer and the
 * file-format constants so embedders can cite UAX #44 uniformly.
 */

/** Canonical UCD version string ("18.0.0"). */
const char* unicodoid_uax44_version(void);

/**
 * @brief Find the long property name for a short alias.
 * @return the long name, or NULL when @p alias is not a known alias.
 */
const char* unicodoid_uax44_property_alias(const char* alias);

/**
 * @brief Find the long value name for (property, short value).
 * @return the long value name, or NULL when absent.
 */
const char* unicodoid_uax44_value_alias(const char* property, const char* short_value);

/**
 * @brief True when @p cp has a CaseFolding mapping (DerivedCoreProperties
 *        Full_CaseFolding + CaseFolding.txt status C/F coverage check via
 *        the generated casefold table).
 */
bool unicodoid_uax44_has_casefold(uint32_t cp);

/** Block name for @p cp (Blocks.txt via the generated tables). */
const char* unicodoid_uax44_block_name(uint32_t cp);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_UAX44_UCD_H */
