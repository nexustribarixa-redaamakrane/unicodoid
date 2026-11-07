#ifndef UNICODOID_UTR59_EAST_ASIAN_SPACING_HPP
#define UNICODOID_UTR59_EAST_ASIAN_SPACING_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace unicodoid {
namespace utr59 {

/**
 * @brief UTR #59 East Asian Spacing (Unicode 18.0.0, Revision 1 draft).
 *
 * Implements the informative East_Asian_Spacing property and the spacing
 * algorithm of §3.3 over UAX #29 extended grapheme clusters.
 */

/** East_Asian_Spacing property values (UTR59 Table 1). */
enum class Eas : uint8_t {
    W = 1, /**< Wide. */
    N = 2, /**< Narrow. */
    O = 3, /**< Other. */
    C = 4  /**< Conditional. */
};

/** Raw East_Asian_Spacing value for a code point (never resolves C). */
uint8_t east_asian_spacing(uint32_t cp);

/** Resolve the property value of one extended grapheme cluster. */
uint8_t cluster_value(const std::u32string& cluster, bool chinese, bool vertical);

/**
 * @brief Compute where auto-spacing is inserted.
 *
 * Each offset is the scalar index of the first code point of the following
 * cluster; a space belongs immediately before `cps[offset]`.
 */
std::vector<std::size_t> spacing_offsets(
    const std::u32string& cps, bool chinese, bool vertical);

} // namespace utr59
} // namespace unicodoid

#endif /* UNICODOID_UTR59_EAST_ASIAN_SPACING_HPP */
