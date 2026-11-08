#ifndef UNICODOID_UAX45_USOURCE_IDEOGRAPHS_HPP
#define UNICODOID_UAX45_USOURCE_IDEOGRAPHS_HPP

#include <cstdint>
#include <string_view>

namespace unicodoid {
namespace uax45 {

/** UAX #45 U-source Ideographs (Unicode 18.0.0, Revision 34). */

/** True when @p cp is a U-source ideograph. */
bool is_usource(uint32_t cp);

/** US-source family name ("US", "US1"), or an empty view when absent. */
std::string_view usource_family(uint32_t cp);

/** True when @p cp is in the U-source PUA staging area. */
bool is_us1_staging(uint32_t cp);

} // namespace uax45
} // namespace unicodoid

#endif /* UNICODOID_UAX45_USOURCE_IDEOGRAPHS_HPP */
