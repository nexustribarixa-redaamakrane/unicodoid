#ifndef UNICODOID_UTS10_COLLATION_HPP
#define UNICODOID_UTS10_COLLATION_HPP

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace unicodoid {
namespace uts10 {

/**
 * @brief UTS #10 Unicode Collation Algorithm (Unicode 18.0.0, Revision 55).
 *
 * Implements the Default Unicode Collation Element Table in its
 * NON_IGNORABLE variable-weighting configuration:
 *
 *   - NFD normalization as the mandatory first step (S1.1),
 *   - collation element construction with longest contiguous matches and
 *     discontiguous non-starter extension with blocking (S2.1-S2.5),
 *   - three-level sort key formation with 16-bit weights (S3.1-S3.9).
 *
 * Backward secondary level, CLDR tailorings, semi-stability, script reordering
 * and the searching/matching algorithm of §11 are out of scope.
 */

using Unit = uint32_t;

/** Build the three-level sort key for a scalar sequence. */
std::vector<Unit> sort_key(const std::u32string& input);

/**
 * @brief Compare two scalar strings under the default UCA.
 *
 * @return negative when a sorts before b, zero when equal, positive otherwise.
 */
int compare(const std::u32string& a, const std::u32string& b);

} // namespace uts10
} // namespace unicodoid

#endif /* UNICODOID_UTS10_COLLATION_HPP */
