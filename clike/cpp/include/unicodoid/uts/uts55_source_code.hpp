#ifndef UNICODOID_UTS55_SOURCE_CODE_HPP
#define UNICODOID_UTS55_SOURCE_CODE_HPP

#include <cstddef>
#include <cstdint>
#include <string>

namespace unicodoid {
namespace uts55 {

/** UTS #55 Unicode Source Code Handling (Revision 5). */

/** True when @p cp is a Bidi control. */
bool is_bidi_control(uint32_t cp);

/** True when @p cp is ZWJ or ZWNJ. */
bool is_joiner(uint32_t cp);

/** Number of flagged characters in @p cps. */
std::size_t flag_count(const std::u32string& cps);

} // namespace uts55
} // namespace unicodoid

#endif /* UNICODOID_UTS55_SOURCE_CODE_HPP */
