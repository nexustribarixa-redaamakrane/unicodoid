#include "unicodoid/utr/utr23_character_property_model.hpp"

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace unicodoid {
namespace utr23 {

std::size_t binary_property_count() { return 41; }

bool is_binary(std::size_t index) { return index < 41; }

PropertyKind property_kind(std::string_view name) {
    if (name == "General_Category") return PropertyKind::Enumerated;
    if (name == "Numeric_Type") return PropertyKind::Numeric;
    if (name == "Unicode_1_Name") return PropertyKind::String;
    return PropertyKind::Binary;
}

} // namespace utr23
} // namespace unicodoid
