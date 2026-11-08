#ifndef UNICODOID_UTR17_CHARACTER_ENCODING_MODEL_HPP
#define UNICODOID_UTR17_CHARACTER_ENCODING_MODEL_HPP

#include <cstdint>

namespace unicodoid {
namespace utr17 {

/** UTR #17 Unicode Character Encoding Model (Revision 9). */

/** UTF-8 code-unit length for a valid scalar, 0 for invalid. */
int utf8_length(uint32_t cp);

/** UTF-16 code-unit length for a valid scalar, 0 for invalid. */
int utf16_length(uint32_t cp);

/** True when @p cp is a replacement candidate (noncharacter or unassigned). */
bool is_replacement(uint32_t cp);

/** True when @p cp is a valid Unicode scalar value. */
bool is_valid(uint32_t cp);

} // namespace utr17
} // namespace unicodoid

#endif /* UNICODOID_UTR17_CHARACTER_ENCODING_MODEL_HPP */
