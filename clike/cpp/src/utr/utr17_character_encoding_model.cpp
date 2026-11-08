#include "unicodoid/utr/utr17_character_encoding_model.hpp"
#include "unicodoid/unicodoid_ucd.h"

#include <cstdint>

namespace unicodoid {
namespace utr17 {

bool is_valid(uint32_t cp) {
    return cp <= 0x10FFFF && !(cp >= 0xD800 && cp <= 0xDFFF);
}

int utf8_length(uint32_t cp) {
    if (!is_valid(cp)) return 0;
    return cp <= 0x7F ? 1 : cp <= 0x7FF ? 2 : cp <= 0xFFFF ? 3 : 4;
}

int utf16_length(uint32_t cp) {
    if (!is_valid(cp)) return 0;
    return cp <= 0xFFFF ? 1 : 2;
}

bool is_replacement(uint32_t cp) {
    if (unicodoid_ucd_has_binary_property(
            cp, UNICODOID_BP_NONCHARACTER_CODE_POINT)) {
        return true;
    }
    return unicodoid_ucd_general_category(cp) ==
           static_cast<uint8_t>(UNICODOID_GC_CN);
}

} // namespace utr17
} // namespace unicodoid
