#include "unicodoid/uts/uts39_security.hpp"
#include "unicodoid/uts/uts39_security.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace unicodoid {
namespace uts39 {

std::u32string confusable(uint32_t cp) {
    uint32_t out[8];
    const std::size_t n = unicodoid_uts39_confusable(cp, out, 8);
    return std::u32string(out, out + n);
}

std::u32string skeleton(const std::u32string& cps) {
    if (cps.empty()) return {};
    std::vector<uint32_t> out(cps.size() * 4 + 8);
    std::size_t n = unicodoid_uts39_skeleton(
        reinterpret_cast<const uint32_t*>(cps.data()), cps.size(),
        out.data(), out.size());
    if (n > out.size()) {
        out.resize(n);
        n = unicodoid_uts39_skeleton(
            reinterpret_cast<const uint32_t*>(cps.data()), cps.size(),
            out.data(), out.size());
    }
    return std::u32string(out.begin(), out.begin() + static_cast<std::ptrdiff_t>(n));
}

bool are_confusable(const std::u32string& a, const std::u32string& b) {
    return unicodoid_uts39_are_confusable(
        reinterpret_cast<const uint32_t*>(a.data()), a.size(),
        reinterpret_cast<const uint32_t*>(b.data()), b.size());
}

bool is_default_ignorable(uint32_t cp) {
    return unicodoid_uts39_is_default_ignorable(cp);
}

} // namespace uts39
} // namespace unicodoid
