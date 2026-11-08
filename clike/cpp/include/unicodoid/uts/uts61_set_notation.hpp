#ifndef UNICODOID_UTS61_SET_NOTATION_HPP
#define UNICODOID_UTS61_SET_NOTATION_HPP

#include <cstdint>
#include <optional>
#include <string_view>
#include <utility>

namespace unicodoid {
namespace uts61 {

/** UTS #61 Unicode Set Notation (Version 1, Draft 4). */

/**
 * Membership of @p cp in the set expression @p set.
 * @return 1 in, 0 out, -1 on parse error.
 */
int set_membership(std::string_view set, uint32_t cp);

/** Parse "X..Y" range text (hex scalars separated by ".."). */
std::optional<std::pair<uint32_t, uint32_t>> parse_range(std::string_view text);

/** True when @p cp lies in [lo, hi]. */
bool in_range(uint32_t cp, uint32_t lo, uint32_t hi);

} // namespace uts61
} // namespace unicodoid

#endif /* UNICODOID_UTS61_SET_NOTATION_HPP */
