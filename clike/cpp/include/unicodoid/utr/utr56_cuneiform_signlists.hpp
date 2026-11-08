#ifndef UNICODOID_UTR56_CUNEIFORM_SIGNLISTS_HPP
#define UNICODOID_UTR56_CUNEIFORM_SIGNLISTS_HPP

#include <cstdint>

namespace unicodoid {
namespace utr56 {

/** UTR #56 Unicode Cuneiform Sign Lists (Revision 5). */

/** True when @p cp is in Cuneiform or its extensions. */
bool is_cuneiform(uint32_t cp);

/** True when @p cp is a Cuneiform number. */
bool is_number(uint32_t cp);

} // namespace utr56
} // namespace unicodoid

#endif /* UNICODOID_UTR56_CUNEIFORM_SIGNLISTS_HPP */
