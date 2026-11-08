#ifndef UNICODOID_UAX14_LINEBREAK_HPP
#define UNICODOID_UAX14_LINEBREAK_HPP

#include <cstddef>
#include <cstdint>
#include <string>

namespace unicodoid {
namespace uax14 {

/**
 * @brief UAX #14 Unicode Line Breaking Algorithm (Unicode 18.0.0, Rev. 57).
 *
 * The full LB1..LB31 rule engine is the native C++ segmenter; these helpers
 * expose it as a scalar API.  Break positions are scalar indices, and
 * mandatory breaks (BK/CR/LF/NL) are reported like any other boundary.
 */

/** Line_Break class of @p cp (unicodoid_lb_t value). */
uint8_t line_break(uint32_t cp);

/** Index one past the end of the line segment starting at @p start. */
std::size_t next_break(const std::u32string& cps, std::size_t start);

/** True when a break occurs immediately before @p index (1..size). */
bool is_break(const std::u32string& cps, std::size_t index);

} // namespace uax14
} // namespace unicodoid

#endif /* UNICODOID_UAX14_LINEBREAK_HPP */
