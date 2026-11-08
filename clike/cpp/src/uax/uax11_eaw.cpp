#include "unicodoid/uax/uax11_eaw.hpp"
#include "unicodoid/unicodoid_ucd.h"

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace unicodoid {
namespace uax11 {

uint8_t eaw(uint32_t cp) {
    return unicodoid_ucd_east_asian_width(cp);
}

bool is_wide(uint32_t cp) {
    const uint8_t value = eaw(cp);
    return value == static_cast<uint8_t>(Width::Fullwidth) ||
           value == static_cast<uint8_t>(Width::Wide);
}

int column_count(uint32_t cp, bool ambiguous_as_wide) {
    const uint8_t value = eaw(cp);
    if (value == static_cast<uint8_t>(Width::Fullwidth) ||
        value == static_cast<uint8_t>(Width::Wide)) {
        return 2;
    }
    if (value == static_cast<uint8_t>(Width::Ambiguous) && ambiguous_as_wide) {
        return 2;
    }
    return 1;
}

std::size_t columns(std::u32string_view cps, bool ambiguous_as_wide) {
    std::size_t total = 0;
    for (const char32_t cp : cps) {
        total += static_cast<std::size_t>(
            column_count(static_cast<uint32_t>(cp), ambiguous_as_wide));
    }
    return total;
}

} // namespace uax11
} // namespace unicodoid
