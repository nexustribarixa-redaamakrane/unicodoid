#include "unicodoid/utr/utr25_mathematics.hpp"
#include "unicodoid/unicodoid_ucd.h"

#include <cstdint>

namespace unicodoid {
namespace utr25 {

bool is_math(uint32_t cp) {
    return unicodoid_ucd_has_binary_property(cp, UNICODOID_BP_MATH);
}

bool is_math_alphanumeric(uint32_t cp) {
    return cp >= 0x1D400 && cp <= 0x1D7FF;
}

bool is_operator(uint32_t cp) {
    if (!is_math(cp)) return false;
    const uint8_t gc = unicodoid_ucd_general_category(cp);
    return gc == static_cast<uint8_t>(UNICODOID_GC_SM) ||
           gc == static_cast<uint8_t>(UNICODOID_GC_SC);
}

} // namespace utr25
} // namespace unicodoid
