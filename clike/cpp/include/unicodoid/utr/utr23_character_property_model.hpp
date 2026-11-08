#ifndef UNICODOID_UTR23_CHARACTER_PROPERTY_MODEL_HPP
#define UNICODOID_UTR23_CHARACTER_PROPERTY_MODEL_HPP

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace unicodoid {
namespace utr23 {

/** UTR #23 The Unicode Character Property Model (Revision 15). */

/** Property-kind classification. */
enum class PropertyKind : int {
    Binary = 0,
    Enumerated = 1,
    Numeric = 2,
    String = 3
};

/** Number of supported binary properties. */
std::size_t binary_property_count();

/** True when the property at @p index is binary. */
bool is_binary(std::size_t index);

/** Property-kind classification for a named property (Binary for unknown). */
PropertyKind property_kind(std::string_view name);

} // namespace utr23
} // namespace unicodoid

#endif /* UNICODOID_UTR23_CHARACTER_PROPERTY_MODEL_HPP */
