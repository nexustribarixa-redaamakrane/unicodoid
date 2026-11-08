#ifndef UNICODOID_UTR54_MONGOLIAN_HPP
#define UNICODOID_UTR54_MONGOLIAN_HPP

#include <cstdint>

namespace unicodoid {
namespace utr54 {

/** UTR #54 Unicode Mongolian 12.1 Baseline (Revision 1). */

/** True when @p cp is in the Mongolian block. */
bool is_mongolian(uint32_t cp);

/** True when @p cp is a Mongolian Free Variation Selector. */
bool is_fvs(uint32_t cp);

/** True when @p cp is the Mongolian Vowel Separator. */
bool is_mvs(uint32_t cp);

} // namespace utr54
} // namespace unicodoid

#endif /* UNICODOID_UTR54_MONGOLIAN_HPP */
