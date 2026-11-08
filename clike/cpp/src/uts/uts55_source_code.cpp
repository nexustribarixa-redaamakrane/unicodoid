#include "unicodoid/uts/uts55_source_code.hpp"
#include "unicodoid/unicodoid_ucd.h"

#include <cstddef>
#include <cstdint>
#include <string>

namespace unicodoid {
namespace uts55 {

bool is_bidi_control(uint32_t cp) {
    return unicodoid_ucd_has_binary_property(cp, UNICODOID_BP_BIDI_CONTROL);
}

bool is_joiner(uint32_t cp) {
    return cp == 0x200C || cp == 0x200D;
}

std::size_t flag_count(const std::u32string& cps) {
    std::size_t n = 0;
    for (const char32_t ch : cps) {
        const uint32_t cp = static_cast<uint32_t>(ch);
        if (is_bidi_control(cp) || is_joiner(cp)) ++n;
    }
    return n;
}

} // namespace uts55
} // namespace unicodoid
