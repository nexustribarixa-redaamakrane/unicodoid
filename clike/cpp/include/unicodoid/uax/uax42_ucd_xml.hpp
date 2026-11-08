#ifndef UNICODOID_UAX42_UCD_XML_HPP
#define UNICODOID_UAX42_UCD_XML_HPP

#include <cstdint>
#include <string>

namespace unicodoid {
namespace uax42 {

/** UAX #42 Unicode Character Database in XML (Unicode 18.0.0, Rev. 40). */

/** Format identifier ("UCDxml"). */
std::string_view format();

/** Emit a single <char .../> record for @p cp. */
std::string char_record(uint32_t cp);

/** True when @p cp is representable in UCD XML. */
bool is_representable(uint32_t cp);

} // namespace uax42
} // namespace unicodoid

#endif /* UNICODOID_UAX42_UCD_XML_HPP */
