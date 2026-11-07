// Native UTR #59 East Asian Spacing for the C++ API.
//
// The property values come from the generated Unicode 18.0.0 data; the
// cluster resolution and spacing algorithm are implemented here over the
// native C++ segmenter.

#include "unicodoid/utr/utr59_east_asian_spacing.hpp"
#include "unicodoid/uax/uax29_segmentation.hpp"
#include "unicodoid/unicodoid_ucd.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace unicodoid {
namespace utr59 {
namespace {

bool is_upright(uint32_t cp) {
    const uint8_t vo = unicodoid_ucd_vertical_orientation(cp);
    return vo == UNICODOID_VO_U || vo == UNICODOID_VO_TU ||
           vo == UNICODOID_VO_TR;
}

} // namespace

uint8_t east_asian_spacing(uint32_t cp) {
    return unicodoid_ucd_east_asian_spacing(cp);
}

uint8_t cluster_value(
    const std::u32string& cluster, bool chinese, bool vertical) {
    if (cluster.empty()) return static_cast<uint8_t>(Eas::O);

    // Step 3: the first code point determines the initial value.
    uint8_t value = east_asian_spacing(static_cast<uint32_t>(cluster[0]));

    // UTR59 §3.4.1: an enclosing mark forces the whole cluster to O.
    for (std::size_t k = 0; k < cluster.size(); ++k) {
        if (unicodoid_ucd_general_category(static_cast<uint32_t>(cluster[k])) ==
            UNICODOID_GC_ME) {
            value = static_cast<uint8_t>(Eas::O);
            break;
        }
    }

    // Steps 4-5: vertical layout, upright Narrow characters become Other.
    if (vertical && value == static_cast<uint8_t>(Eas::N) &&
        is_upright(static_cast<uint32_t>(cluster[0]))) {
        value = static_cast<uint8_t>(Eas::O);
    }

    // Step 6: resolve Conditional.
    if (value == static_cast<uint8_t>(Eas::C)) {
        value = chinese ? static_cast<uint8_t>(Eas::N)
                        : static_cast<uint8_t>(Eas::O);
    }

    return value;
}

std::vector<std::size_t> spacing_offsets(
    const std::u32string& cps, bool chinese, bool vertical) {
    std::vector<std::size_t> offsets;
    if (cps.empty()) return offsets;

    const std::vector<std::size_t> boundaries =
        Segmenter::boundaries(cps, BoundaryType::Grapheme);
    if (boundaries.size() < 2) return offsets;

    std::size_t prev = cluster_value(
        cps.substr(boundaries[0], boundaries[1] - boundaries[0]),
        chinese, vertical);

    for (std::size_t b = 1; b + 1 < boundaries.size(); ++b) {
        const std::size_t next_start = boundaries[b];
        const std::size_t next_end = boundaries[b + 1];
        const std::size_t next = cluster_value(
            cps.substr(next_start, next_end - next_start), chinese, vertical);

        const bool insert =
            (prev == static_cast<std::size_t>(Eas::W) &&
             next == static_cast<std::size_t>(Eas::N)) ||
            (prev == static_cast<std::size_t>(Eas::N) &&
             next == static_cast<std::size_t>(Eas::W));
        if (insert) offsets.push_back(next_start);

        prev = next;
    }

    return offsets;
}

} // namespace utr59
} // namespace unicodoid
