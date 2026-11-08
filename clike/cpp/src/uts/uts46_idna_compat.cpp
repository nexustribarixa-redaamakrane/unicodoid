#include "unicodoid/uts/uts46_idna_compat.hpp"
#include "unicodoid/uts/uts46_idna_compat.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace unicodoid {
namespace uts46 {

Disposition disposition(uint32_t cp) {
    return static_cast<Disposition>(unicodoid_uts46_disposition(cp));
}

std::u32string map(const std::u32string& input) {
    if (input.empty()) return {};
    std::vector<uint32_t> out(input.size() * 4 + 4);
    std::size_t n = unicodoid_uts46_map(
        reinterpret_cast<const uint32_t*>(input.data()), input.size(),
        out.data(), out.size());
    if (n > out.size()) {
        out.resize(n);
        n = unicodoid_uts46_map(
            reinterpret_cast<const uint32_t*>(input.data()), input.size(),
            out.data(), out.size());
    }
    return std::u32string(out.begin(), out.begin() + static_cast<std::ptrdiff_t>(n));
}

bool check_label(const std::u32string& label) {
    return unicodoid_uts46_check_label(
        reinterpret_cast<const uint32_t*>(label.data()), label.size());
}

bool is_transitional_disallowed(uint32_t cp) {
    return unicodoid_uts46_is_transitional_disallowed(cp);
}

} // namespace uts46
} // namespace unicodoid
