#include "unicodoid/utr/utr54_mongolian.hpp"

#include <cstdint>

namespace unicodoid {
namespace utr54 {

bool is_mongolian(uint32_t cp) {
    return cp >= 0x1800 && cp <= 0x18AF;
}

bool is_fvs(uint32_t cp) {
    return cp == 0x180B || cp == 0x180C || cp == 0x180D || cp == 0x180F;
}

bool is_mvs(uint32_t cp) {
    return cp == 0x180E;
}

} // namespace utr54
} // namespace unicodoid
