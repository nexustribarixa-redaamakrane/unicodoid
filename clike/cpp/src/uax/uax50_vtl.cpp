#include "unicodoid/uax/uax50_vtl.hpp"
#include "unicodoid/unicodoid_ucd.h"

#include <cstdint>

namespace unicodoid {
namespace uax50 {

uint8_t vertical_orientation(uint32_t cp) {
    return unicodoid_ucd_vertical_orientation(cp);
}

uint8_t resolved_orientation(uint32_t cp) {
    const uint8_t value = vertical_orientation(cp);
    if (value == static_cast<uint8_t>(UNICODOID_VO_TR)) {
        return static_cast<uint8_t>(UNICODOID_VO_R);
    }
    if (value == static_cast<uint8_t>(UNICODOID_VO_TU)) {
        return static_cast<uint8_t>(UNICODOID_VO_U);
    }
    return value;
}

bool is_upright(uint32_t cp) {
    return resolved_orientation(cp) == static_cast<uint8_t>(UNICODOID_VO_U);
}

} // namespace uax50
} // namespace unicodoid
