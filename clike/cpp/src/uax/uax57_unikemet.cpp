#include "unicodoid/uax/uax57_unikemet.hpp"

#include <cstdint>
#include <string>
#include <string_view>

namespace unicodoid {
namespace uax57 {

bool is_hieroglyph(uint32_t cp) {
    return (cp >= 0x13000 && cp <= 0x1342F) ||
           (cp >= 0x13460 && cp <= 0x143FF);
}

bool is_core(uint32_t cp) {
    return cp >= 0x13460 && cp <= 0x143FF;
}

bool is_legacy(uint32_t cp) {
    return cp >= 0x13000 && cp <= 0x1342F;
}

bool is_no_mirror(uint32_t cp) {
    return cp == 0x130BB || cp == 0x130BD;
}

std::string unikemet_field(uint32_t /*cp*/, std::string_view /*field*/) {
    return {};
}

} // namespace uax57
} // namespace unicodoid
