#ifndef UNICODOID_UAX24_SCRIPT_PROPERTY_HPP
#define UNICODOID_UAX24_SCRIPT_PROPERTY_HPP

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace unicodoid {
namespace uax24 {

/**
 * @brief UAX #24 Unicode Script Property (Unicode 18.0.0, Revision 41).
 *
 * Script values are generated indices; use script_name() for the long
 * Script name (for example "Latin").  Script_Extensions is not carried by
 * the generated tables; it is reported as out of scope, and security
 * analysis that needs it should use UTS #39 data.
 */

/** Number of known Script values (including Unknown at index 0). */
std::size_t count();

/** Script name for @p value, or an empty view when out of range. */
std::string_view script_name(uint16_t value);

/** Script value of @p cp. */
uint16_t script(uint32_t cp);

/** Find a Script value by name; 0 (Unknown) when absent. */
uint16_t find(std::string_view name);

/** True when @p cp belongs to Script value @p script. */
bool is_script(uint32_t cp, uint16_t script);

} // namespace uax24
} // namespace unicodoid

#endif /* UNICODOID_UAX24_SCRIPT_PROPERTY_HPP */
