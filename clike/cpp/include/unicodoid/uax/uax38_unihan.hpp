#ifndef UNICODOID_UAX38_UNIHAN_HPP
#define UNICODOID_UAX38_UNIHAN_HPP

#include <cstdint>
#include <string>
#include <string_view>

namespace unicodoid {
namespace uax38 {

/**
 * @brief UAX #38 Unicode Han Database (Unihan) (Unicode 18.0.0, Rev. 41).
 *
 * The generated tables carry Unified_Ideograph, Radical and the CJK block
 * structure.  The full Unihan field store (k*, u*) is distributed as
 * Unihan.zip and is not embedded; unihan_field() reports that absence.
 */

/** True when @p cp is a Unified_Ideograph. */
bool is_unified_ideograph(uint32_t cp);

/** True when @p cp is a CJK Radical. */
bool is_radical(uint32_t cp);

/** True when @p cp lies in a CJK block. */
bool is_cjk_block(uint32_t cp);

/** 1-based CJKRadicals.txt index, or 0 when @p cp is not a radical. */
int radical_index(uint32_t cp);

/**
 * Look up a Unihan field value.  Returns false and an empty string because
 * the field store is not part of the generated tables.
 */
bool unihan_field(uint32_t cp, std::string_view field, std::string& out);

} // namespace uax38
} // namespace unicodoid

#endif /* UNICODOID_UAX38_UNIHAN_HPP */
