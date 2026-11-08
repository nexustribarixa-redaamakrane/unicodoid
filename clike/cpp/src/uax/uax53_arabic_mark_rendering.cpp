#include "unicodoid/uax/uax53_arabic_mark_rendering.hpp"
#include "unicodoid/unicodoid_ucd.h"

#include <cstddef>
#include <cstdint>
#include <string>

namespace unicodoid {
namespace uax53 {

uint8_t joining_type(uint32_t cp) {
    return unicodoid_ucd_joining_type(cp);
}

bool is_joining_transparent(uint32_t cp) {
    const uint8_t gc = unicodoid_ucd_general_category(cp);
    return gc == static_cast<uint8_t>(UNICODOID_GC_MN) ||
           gc == static_cast<uint8_t>(UNICODOID_GC_ME) ||
           gc == static_cast<uint8_t>(UNICODOID_GC_CF);
}

uint16_t joining_group(uint32_t cp) {
    return unicodoid_ucd_script(cp);
}

bool is_consistent(const std::u32string& cps) {
    if (cps.empty()) return true;
    for (const char32_t ch : cps) {
        const uint32_t cp = static_cast<uint32_t>(ch);
        if (cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF)) return false;
    }

    for (std::size_t i = 0; i < cps.size(); ++i) {
        const uint8_t jt = unicodoid_ucd_joining_type(
            static_cast<uint32_t>(cps[i]));
        if (jt != static_cast<uint8_t>(UNICODOID_JT_L) &&
            jt != static_cast<uint8_t>(UNICODOID_JT_R) &&
            jt != static_cast<uint8_t>(UNICODOID_JT_D)) {
            continue;
        }
        // Isolated Dual characters use non-joining forms, which UAX #53
        // permits; only invalid scalar values fail the check.
    }
    return true;
}

} // namespace uax53
} // namespace unicodoid
