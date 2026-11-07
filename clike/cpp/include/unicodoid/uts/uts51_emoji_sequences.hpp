#ifndef UNICODOID_UTS51_EMOJI_SEQUENCES_HPP
#define UNICODOID_UTS51_EMOJI_SEQUENCES_HPP

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace unicodoid {
namespace uts51 {

/**
 * @brief UTS #51 Unicode Emoji sequence validation (Unicode 18.0.0).
 *
 * Implements the machine-readable RGI emoji sequence sets distributed in
 * emoji-sequences.txt and emoji-zwj-sequences.txt.
 */

/** Number of RGI emoji sequences in the generated data. */
std::size_t count();

/** RGI type name of sequence @p index, or an empty view when out of range. */
std::string_view type(std::size_t index);

/** Code points of sequence @p index, or nullopt when out of range. */
std::optional<std::vector<char32_t>> sequence(std::size_t index);

/** RGI type name when @p cps is an exact RGI sequence. */
std::optional<std::string_view> is_rgi(const std::u32string& cps);

/** Structural validation independent of the RGI table. */
bool is_well_formed(const std::u32string& cps);

} // namespace uts51
} // namespace unicodoid

#endif /* UNICODOID_UTS51_EMOJI_SEQUENCES_HPP */
