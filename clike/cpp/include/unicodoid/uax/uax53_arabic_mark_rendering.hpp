#ifndef UNICODOID_UAX53_ARABIC_MARK_RENDERING_HPP
#define UNICODOID_UAX53_ARABIC_MARK_RENDERING_HPP

#include <cstdint>
#include <string>

namespace unicodoid {
namespace uax53 {

/** UAX #53 Unicode Arabic Mark Rendering (Unicode 18.0.0, Rev. 13). */

/** Joining_Type of @p cp (unicodoid_joining_type_t value). */
uint8_t joining_type(uint32_t cp);

/** True when @p cp joins transparently (Mn, Me or Cf). */
bool is_joining_transparent(uint32_t cp);

/** Shaping integrity check over a scalar sequence. */
bool is_consistent(const std::u32string& cps);

/** Script-derived joining group identifier, 0 when none. */
uint16_t joining_group(uint32_t cp);

} // namespace uax53
} // namespace unicodoid

#endif /* UNICODOID_UAX53_ARABIC_MARK_RENDERING_HPP */
