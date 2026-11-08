#include "unicodoid/uts/uts61_set_notation.hpp"
#include "unicodoid/uts/uts61_set_notation.h"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace unicodoid {
namespace uts61 {

int set_membership(std::string_view set, uint32_t cp) {
    const std::string owned(set);
    return unicodoid_uts61_set_membership(owned.c_str(), cp);
}

std::optional<std::pair<uint32_t, uint32_t>> parse_range(
    std::string_view text) {
    const std::string owned(text);
    uint32_t lo = 0;
    uint32_t hi = 0;
    if (!unicodoid_uts61_parse_range(owned.c_str(), &lo, &hi)) {
        return std::nullopt;
    }
    return std::make_pair(lo, hi);
}

bool in_range(uint32_t cp, uint32_t lo, uint32_t hi) {
    return unicodoid_uts61_in_range(cp, lo, hi);
}

} // namespace uts61
} // namespace unicodoid
