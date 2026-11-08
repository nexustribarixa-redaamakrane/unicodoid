// UAX #9 Unicode Bidirectional Algorithm for the C++ API.
//
// The paragraph algorithm and the official BidiMirroring/BidiBrackets data
// live in the shared C reference implementation.  This module exposes the
// same behaviour through idiomatic C++ containers.

#include "unicodoid/uax/uax9_bidi.hpp"
#include "unicodoid/uax/uax9_bidi.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace unicodoid {
namespace uax9 {

uint8_t bidi_class(uint32_t cp) {
    return unicodoid_uax9_bidi_class(cp);
}

uint8_t paragraph_level(const std::u32string& cps, int base_dir) {
    return unicodoid_uax9_paragraph_level(
        cps.empty() ? nullptr
                    : reinterpret_cast<const uint32_t*>(cps.data()),
        cps.size(), base_dir);
}

std::vector<uint8_t> levels(const std::u32string& cps, int base_dir) {
    std::vector<uint8_t> out(cps.size(), 0);
    if (cps.empty()) return out;
    unicodoid_uax9_resolve_levels(
        reinterpret_cast<const uint32_t*>(cps.data()), cps.size(), base_dir,
        out.data(), out.size());
    return out;
}

std::u32string visual_order(const std::u32string& cps, int base_dir) {
    if (cps.empty()) return {};
    std::u32string out(cps.size(), U'\0');
    unicodoid_uax9_visual_order(
        reinterpret_cast<const uint32_t*>(cps.data()), cps.size(), base_dir,
        reinterpret_cast<uint32_t*>(out.data()), out.size());
    return out;
}

bool is_mirrored(uint32_t cp) {
    return unicodoid_uax9_is_mirrored(cp);
}

uint32_t mirrored_glyph(uint32_t cp) {
    return unicodoid_uax9_mirrored_glyph(cp);
}

bool is_paired_bracket(uint32_t cp) {
    return unicodoid_uax9_is_paired_bracket(cp);
}

} // namespace uax9
} // namespace unicodoid
