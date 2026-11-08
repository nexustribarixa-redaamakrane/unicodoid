#include "unicodoid/uax/uax45_usource_ideographs.hpp"
#include "unicodoid/unicodoid_ucd.h"

#include <cstdint>
#include <string_view>

namespace unicodoid {
namespace uax45 {

bool is_usource(uint32_t cp) {
    if (unicodoid_ucd_has_binary_property(cp, UNICODOID_BP_UNIFIED_IDEOGRAPH)) {
        return true;
    }
    return (cp >= 0x20000 && cp <= 0x2FA1F) ||
           (cp >= 0x30000 && cp <= 0x3134F);
}

bool is_us1_staging(uint32_t cp) {
    return cp >= 0xF900 && cp <= 0xFAFF;
}

std::string_view usource_family(uint32_t cp) {
    if (is_us1_staging(cp)) return "US1";
    if (is_usource(cp)) return "US";
    return {};
}

} // namespace uax45
} // namespace unicodoid
