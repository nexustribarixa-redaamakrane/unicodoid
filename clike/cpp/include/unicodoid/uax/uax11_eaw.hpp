#ifndef UNICODOID_UAX11_EAW_HPP
#define UNICODOID_UAX11_EAW_HPP

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace unicodoid {
namespace uax11 {

/** UAX #11 East Asian Width (Unicode 18.0.0, Revision 46). */

/** East_Asian_Width values, matching the generated-table order. */
enum class Width : uint8_t {
    Neutral = 0,
    Ambiguous = 1,
    Halfwidth = 2,
    Fullwidth = 3,
    Narrow = 4,
    Wide = 5
};

/** Raw East_Asian_Width of @p cp. */
uint8_t eaw(uint32_t cp);

/** True when @p cp is Fullwidth or Wide. */
bool is_wide(uint32_t cp);

/** Terminal column count: W/F = 2, A = 2 when opted in, otherwise 1. */
int column_count(uint32_t cp, bool ambiguous_as_wide);

/** Sum of column counts for a scalar string. */
std::size_t columns(std::u32string_view cps, bool ambiguous_as_wide);

} // namespace uax11
} // namespace unicodoid

#endif /* UNICODOID_UAX11_EAW_HPP */
