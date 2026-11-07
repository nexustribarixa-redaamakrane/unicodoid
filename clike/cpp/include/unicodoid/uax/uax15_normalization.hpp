#ifndef UNICODOID_UAX15_NORMALIZATION_HPP
#define UNICODOID_UAX15_NORMALIZATION_HPP

#include <string>
#include <string_view>

namespace unicodoid {
namespace uax15 {

/**
 * @brief Unicode normalization forms (UAX #15).
 */
enum class Form {
    Nfc,
    Nfd,
    Nfkc,
    Nfkd
};

/**
 * @brief Normalize a scalar string to the requested form.
 *
 * Implements the full UAX #15 algorithm over the generated Unicode 18.0.0
 * data: recursive canonical/compatibility decomposition, canonical ordering,
 * Hangul algorithmic decomposition/composition and canonical composition.
 *
 * Ill-formed scalar values (surrogates or values above U+10FFFF) are passed
 * through unchanged; callers validating input should reject them beforehand.
 */
std::u32string normalize(const std::u32string& input, Form form);

/** True when @p input is already in the requested normalization form. */
bool is_normalized(const std::u32string& input, Form form);

} // namespace uax15
} // namespace unicodoid

#endif /* UNICODOID_UAX15_NORMALIZATION_HPP */
