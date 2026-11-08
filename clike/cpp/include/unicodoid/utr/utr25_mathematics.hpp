#ifndef UNICODOID_UTR25_MATHEMATICS_HPP
#define UNICODOID_UTR25_MATHEMATICS_HPP

#include <cstdint>

namespace unicodoid {
namespace utr25 {

/** UTR #25 Unicode Support for Mathematics (Revision 16, Draft 5). */

/** True when @p cp has Math = Yes. */
bool is_math(uint32_t cp);

/** True when @p cp is in the Mathematical Alphanumeric Symbols block. */
bool is_math_alphanumeric(uint32_t cp);

/** True when @p cp is a mathematical operator (Math + Sm/Sc). */
bool is_operator(uint32_t cp);

} // namespace utr25
} // namespace unicodoid

#endif /* UNICODOID_UTR25_MATHEMATICS_HPP */
