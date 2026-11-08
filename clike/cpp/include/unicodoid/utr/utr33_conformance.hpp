#ifndef UNICODOID_UTR33_CONFORMANCE_HPP
#define UNICODOID_UTR33_CONFORMANCE_HPP

#include <string_view>

namespace unicodoid {
namespace utr33 {

/** UTR #33 Conformance Model (Revision 7). */

/** Conformance levels. */
enum class Level : int {
    Unspecified = 0,
    WellFormed = 1,
    Valid = 2,
    Stable = 3
};

/** Human-readable name for a conformance level. */
std::string_view level_name(int level);

} // namespace utr33
} // namespace unicodoid

#endif /* UNICODOID_UTR33_CONFORMANCE_HPP */
