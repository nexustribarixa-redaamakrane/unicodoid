#ifndef UNICODOID_UAX57_UNIKEMET_HPP
#define UNICODOID_UAX57_UNIKEMET_HPP

#include <cstdint>
#include <string>
#include <string_view>

namespace unicodoid {
namespace uax57 {

/** UAX #57 Unicode Egyptian Hieroglyph Database (Unicode 18.0.0, Rev. 7). */

/** True when @p cp is an Egyptian hieroglyph. */
bool is_hieroglyph(uint32_t cp);

/** True when @p cp belongs to the Unikemet Core set. */
bool is_core(uint32_t cp);

/** True when @p cp is a Legacy sign. */
bool is_legacy(uint32_t cp);

/** True when @p cp must not be mirrored (covered core entries). */
bool is_no_mirror(uint32_t cp);

/**
 * Look up a Unikemet field value.  Returns an empty string because the
 * full kEH_* store is distributed in Unikemet.txt.
 */
std::string unikemet_field(uint32_t cp, std::string_view field);

} // namespace uax57
} // namespace unicodoid

#endif /* UNICODOID_UAX57_UNIKEMET_HPP */
