#ifndef UNICODOID_UAX41_COMMON_REFS_HPP
#define UNICODOID_UAX41_COMMON_REFS_HPP

#include <string_view>

namespace unicodoid {
namespace uax41 {

/** UAX #41 Common References for UAXs (Unicode 18.0.0, Revision 38). */

/** Revision number. */
std::string_view revision();

/** Publication date (YYYY-MM-DD). */
std::string_view date();

/** Canonical URL. */
std::string_view url();

} // namespace uax41
} // namespace unicodoid

#endif /* UNICODOID_UAX41_COMMON_REFS_HPP */
