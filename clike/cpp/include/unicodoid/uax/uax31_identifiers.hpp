#ifndef UNICODOID_UAX31_IDENTIFIERS_HPP
#define UNICODOID_UAX31_IDENTIFIERS_HPP

#include <cstdint>
#include <string>

namespace unicodoid {
namespace uax31 {

/**
 * @brief UAX #31 Unicode Identifiers and Syntax (Unicode 18.0.0, Rev. 45).
 *
 * Default identifier profile: XID_Start (XID_Continue | ZWNJ | ZWJ)*.
 */

/** True when @p cp is XID_Start. */
bool is_xid_start(uint32_t cp);

/** True when @p cp is XID_Continue. */
bool is_xid_continue(uint32_t cp);

/** True when @p cp is ID_Start. */
bool is_id_start(uint32_t cp);

/** True when @p cp is ID_Continue. */
bool is_id_continue(uint32_t cp);

/** Validate a whole identifier against the default profile. */
bool is_valid(const std::u32string& cps);

/** True when @p cps is NFKC-stable (UAX #31 stability check). */
bool is_nfkc_stable(const std::u32string& cps);

} // namespace uax31
} // namespace unicodoid

#endif /* UNICODOID_UAX31_IDENTIFIERS_HPP */
