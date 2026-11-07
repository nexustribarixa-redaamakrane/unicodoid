#ifndef UNICODOID_UAX34_NAMED_SEQUENCES_HPP
#define UNICODOID_UAX34_NAMED_SEQUENCES_HPP

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace unicodoid {
namespace uax34 {

/**
 * @brief UAX #34 Unicode Named Character Sequences (Unicode 18.0.0).
 *
 * Provides access to the normative NamedSequences.txt data.  Names are
 * compared exactly, as required by UAX #34.
 */

/** Number of registered named character sequences. */
std::size_t count();

/** Name of sequence @p index, or an empty view when out of range. */
std::string_view name(std::size_t index);

/** Code points of sequence @p index, or nullopt when out of range. */
std::optional<std::vector<char32_t>> sequence(std::size_t index);

/** Look up a sequence by exact name. */
std::optional<std::vector<char32_t>> find(std::string_view name);

/** True when @p cps is exactly one of the registered sequences. */
bool is_registered(const std::u32string& cps);

} // namespace uax34
} // namespace unicodoid

#endif /* UNICODOID_UAX34_NAMED_SEQUENCES_HPP */
