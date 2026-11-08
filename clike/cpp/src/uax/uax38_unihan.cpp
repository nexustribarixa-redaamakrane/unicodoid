#include "unicodoid/uax/uax38_unihan.hpp"
#include "unicodoid/unicodoid_ucd.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace unicodoid {
namespace uax38 {

bool is_unified_ideograph(uint32_t cp) {
    return unicodoid_ucd_has_binary_property(cp, UNICODOID_BP_UNIFIED_IDEOGRAPH);
}

bool is_radical(uint32_t cp) {
    if (unicodoid_ucd_has_binary_property(cp, UNICODOID_BP_RADICAL)) return true;
    return (cp >= 0x2F00 && cp <= 0x2FDF) || (cp >= 0x2E80 && cp <= 0x2FFF);
}

bool is_cjk_block(uint32_t cp) {
    return (cp >= 0x3000 && cp <= 0x303F) ||
           (cp >= 0x3040 && cp <= 0x309F) ||
           (cp >= 0x30A0 && cp <= 0x30FF) ||
           (cp >= 0x3100 && cp <= 0x312F) ||
           (cp >= 0x3130 && cp <= 0x318F) ||
           (cp >= 0x3190 && cp <= 0x31FF) ||
           (cp >= 0x3200 && cp <= 0x32FF) ||
           (cp >= 0x3300 && cp <= 0x33FF) ||
           (cp >= 0x3400 && cp <= 0x4DBF) ||
           (cp >= 0x4E00 && cp <= 0x9FFF) ||
           (cp >= 0xA000 && cp <= 0xA4CF) ||
           (cp >= 0xAC00 && cp <= 0xD7AF) ||
           (cp >= 0xF900 && cp <= 0xFAFF) ||
           (cp >= 0xFE30 && cp <= 0xFE4F) ||
           (cp >= 0xFF00 && cp <= 0xFF60) ||
           (cp >= 0x20000 && cp <= 0x2A6DF) ||
           (cp >= 0x2A700 && cp <= 0x2CEAF) ||
           (cp >= 0x2CEB0 && cp <= 0x2EBEF) ||
           (cp >= 0x30000 && cp <= 0x323AF);
}

int radical_index(uint32_t cp) {
    if (cp >= 0x2F00 && cp <= 0x2FDF) {
        return static_cast<int>(cp - 0x2F00) + 1;
    }
    if (!is_radical(cp)) return 0;
    return 1;
}

bool unihan_field(uint32_t /*cp*/, std::string_view /*field*/, std::string& out) {
    out.clear();
    return false;
}

} // namespace uax38
} // namespace unicodoid
