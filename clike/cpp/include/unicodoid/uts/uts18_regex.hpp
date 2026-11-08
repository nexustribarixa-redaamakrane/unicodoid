#ifndef UNICODOID_UTS18_REGEX_HPP
#define UNICODOID_UTS18_REGEX_HPP

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace unicodoid {
namespace uts18 {

/** UTS #18 Unicode Regular Expressions (Revision 25, RL1.1 level). */

/** Result family of a property escape. */
enum class Match : int {
    None = 0,
    GeneralCategory = 1,
    Script = 2,
    Binary = 3,
    EastAsianWidth = 4,
    Invalid = 5
};

/** Width in bytes of a "\p{...}" escape starting at @p pattern. */
std::size_t property_escape_length(std::string_view pattern);

/** Evaluate a property escape against @p cp. */
Match property_match(std::string_view pattern, uint32_t cp);

/** True when @p pattern is a syntactically valid property escape. */
bool is_valid_escape(std::string_view pattern);

} // namespace uts18
} // namespace unicodoid

#endif /* UNICODOID_UTS18_REGEX_HPP */
