#include "unicodoid/uts/uts37_ivd.hpp"

#include <cstdint>
#include <string>

namespace unicodoid {
namespace uts37 {

bool is_variation_selector(uint32_t cp) {
    return cp >= 0xE0100 && cp <= 0xE01EF;
}

int selector_offset(uint32_t cp) {
    if (!is_variation_selector(cp)) return -1;
    return static_cast<int>(cp - 0xE0100);
}

bool is_ivs(const std::u32string& cps) {
    if (cps.size() != 2) return false;
    const uint32_t base = static_cast<uint32_t>(cps[0]);
    if (base > 0x10FFFF || (base >= 0xD800 && base <= 0xDFFF)) return false;
    return is_variation_selector(static_cast<uint32_t>(cps[1]));
}

} // namespace uts37
} // namespace unicodoid
