#ifndef UNICODOID_UAX9_BIDI_HPP
#define UNICODOID_UAX9_BIDI_HPP

#include <cstdint>
#include <string>
#include <vector>

namespace unicodoid {
namespace uax9 {

/**
 * @brief UAX #9 Unicode Bidirectional Algorithm (Unicode 18.0.0, Rev. 52).
 *
 * Paragraph direction (P2/P3), weak/neutral resolution, embedded levels
 * (X1..X10), visual reordering (L2) and mirroring (L3/L4).  The scalar
 * property data and the official BidiMirroring / BidiBrackets tables are
 * shared with the C reference implementation so both APIs agree by
 * construction.
 */

/** Paragraph base direction. */
enum Direction : int {
    Ltr = 0,  /**< Left-to-right paragraph. */
    Rtl = 1,  /**< Right-to-left paragraph. */
    Auto = -1 /**< First strong character, else LTR (P2/P3). */
};

/** Bidirectional_Class of a scalar (unicodoid_bidi_class_t value). */
uint8_t bidi_class(uint32_t cp);

/** Paragraph embedding level (0 for LTR, 1 for RTL) per P2/P3. */
uint8_t paragraph_level(const std::u32string& cps, int base_dir);

/** Resolve one embedding level per scalar. */
std::vector<uint8_t> levels(const std::u32string& cps, int base_dir);

/** Reorder a paragraph to visual order (L2), applying mirroring (L3/L4). */
std::u32string visual_order(const std::u32string& cps, int base_dir);

/** True when @p cp has a mirrored counterpart per BidiMirroring.txt. */
bool is_mirrored(uint32_t cp);

/** Mirrored counterpart of @p cp, or @p cp when there is none. */
uint32_t mirrored_glyph(uint32_t cp);

/** True when @p cp is a paired bracket per BidiBrackets.txt. */
bool is_paired_bracket(uint32_t cp);

} // namespace uax9
} // namespace unicodoid

#endif /* UNICODOID_UAX9_BIDI_HPP */
