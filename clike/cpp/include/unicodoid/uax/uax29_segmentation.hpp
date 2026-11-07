#ifndef UNICODOID_UAX29_SEGMENTATION_HPP
#define UNICODOID_UAX29_SEGMENTATION_HPP

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace unicodoid {

/**
 * @brief Text boundary type for the native segmentation API.
 */
enum class BoundaryType {
    Grapheme = 0,
    Word = 1,
    Line = 2,
    Sentence = 3
};

/**
 * @brief Native Unicode text segmentation (UAX #29 and UAX #14).
 *
 * The algorithms are implemented in C++ directly on top of the generated
 * Unicode 18.0.0 data layer.  They do not forward to the C segmenter.
 */
class Segmenter {
public:
    /**
     * @brief Compute all boundary positions of a scalar string.
     *
     * The result always starts with 0 and ends with `text.size()`.
     */
    static std::vector<std::size_t> boundaries(
        const std::u32string& text, BoundaryType type);

    /**
     * @brief Split a scalar string into (start, end) segments.
     */
    static std::vector<std::pair<std::size_t, std::size_t>> segments(
        const std::u32string& text, BoundaryType type);
};

} // namespace unicodoid

#endif /* UNICODOID_UAX29_SEGMENTATION_HPP */
