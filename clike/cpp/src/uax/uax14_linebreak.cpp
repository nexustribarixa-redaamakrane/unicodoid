#include "unicodoid/uax/uax14_linebreak.hpp"
#include "unicodoid/uax/uax29_segmentation.hpp"
#include "unicodoid/unicodoid_ucd.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace unicodoid {
namespace uax14 {

uint8_t line_break(uint32_t cp) {
    return unicodoid_ucd_line_break(cp);
}

std::size_t next_break(const std::u32string& cps, std::size_t start) {
    if (start >= cps.size()) return cps.size();
    const std::vector<std::size_t> boundaries =
        Segmenter::boundaries(cps, BoundaryType::Line);
    const auto it = std::upper_bound(boundaries.begin(), boundaries.end(), start);
    if (it == boundaries.end()) return cps.size();
    return *it;
}

bool is_break(const std::u32string& cps, std::size_t index) {
    if (index == 0 || index >= cps.size()) return true;
    const std::vector<std::size_t> boundaries =
        Segmenter::boundaries(cps, BoundaryType::Line);
    return std::binary_search(boundaries.begin(), boundaries.end(), index);
}

} // namespace uax14
} // namespace unicodoid
