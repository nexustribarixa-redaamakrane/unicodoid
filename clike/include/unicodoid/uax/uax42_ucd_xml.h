#ifndef UNICODOID_UAX42_UCD_XML_H
#define UNICODOID_UAX42_UCD_XML_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file uax42_ucd_xml.h
 * @brief UAX #42 Unicode Character Database in XML (Unicode 18.0.0, Rev. 40).
 *
 * UCD XML is a distribution format, not a runtime algorithm.  This module
 * provides the interoperable surface: the canonical file/element names and
 * a minimal writer that emits a <char> record for one scalar from the
 * generated tables (GC, Bidi, EAW, LB, Script, CCC).  Full UCD XML files
 * (UCDxml) are an external distribution; parsers are out of scope.
 */

/** @return "UCDxml" format identifier for Unicode 18.0.0. */
const char* unicodoid_uax42_format(void);

/**
 * @brief Emit a single-character XML record for @p cp.
 * Format: <char cp="XXXX" GC=".." Bidi_C=".." EA=".." LB=".." sc=".." ccc=".."/>
 * @return required length (excluding NUL); writes when buffer is large enough.
 */
size_t unicodoid_uax42_char_record(uint32_t cp, char* out, size_t cap);

/** True when @p cp is representable in UCD XML (valid scalar, not surrogate). */
int unicodoid_uax42_is_representable(uint32_t cp);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_UAX42_UCD_XML_H */
