#include "unicodoid/uts/uts18_regex.hpp"
#include "unicodoid/uts/uts18_regex.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace unicodoid {
namespace uts18 {

std::size_t property_escape_length(std::string_view pattern) {
    const std::string owned(pattern);
    return unicodoid_uts18_property_escape_length(owned.c_str());
}

Match property_match(std::string_view pattern, uint32_t cp) {
    const std::string owned(pattern);
    return static_cast<Match>(
        unicodoid_uts18_property_match(owned.c_str(), cp));
}

bool is_valid_escape(std::string_view pattern) {
    const std::string owned(pattern);
    return unicodoid_uts18_is_valid_escape(owned.c_str());
}

} // namespace uts18
} // namespace unicodoid
