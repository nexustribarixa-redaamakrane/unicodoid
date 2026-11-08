#include "unicodoid/utr/utr56_cuneiform_signlists.hpp"

#include <cstdint>

namespace unicodoid {
namespace utr56 {

bool is_cuneiform(uint32_t cp) {
    return (cp >= 0x12000 && cp <= 0x1247F) ||
           (cp >= 0x12480 && cp <= 0x125FF) ||
           (cp >= 0x14400 && cp <= 0x1467F);
}

bool is_number(uint32_t cp) {
    return cp >= 0x12480 && cp <= 0x125FF;
}

} // namespace utr56
} // namespace unicodoid
