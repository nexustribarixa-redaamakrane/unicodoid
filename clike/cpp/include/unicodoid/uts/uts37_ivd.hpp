#ifndef UNICODOID_UTS37_IVD_HPP
#define UNICODOID_UTS37_IVD_HPP

#include <cstdint>
#include <string>

namespace unicodoid {
namespace uts37 {

/** UTS #37 Unicode Ideographic Variation Database (Revision 16). */

/** True when @p cp is an ideographic variation selector. */
bool is_variation_selector(uint32_t cp);

/** True when @p cps is an IVS (exactly two scalars, second a selector). */
bool is_ivs(const std::u32string& cps);

/** Zero-based selector offset, or -1 when @p cp is not a selector. */
int selector_offset(uint32_t cp);

} // namespace uts37
} // namespace unicodoid

#endif /* UNICODOID_UTS37_IVD_HPP */
