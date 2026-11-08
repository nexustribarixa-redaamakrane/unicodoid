#include "unicodoid/uax/uax31_identifiers.hpp"
#include "unicodoid/uax/uax15_normalization.hpp"
#include "unicodoid/unicodoid_ucd.h"

#include <cstddef>
#include <cstdint>
#include <string>

namespace unicodoid {
namespace uax31 {

bool is_xid_start(uint32_t cp) {
    return unicodoid_ucd_has_binary_property(cp, UNICODOID_BP_XID_START);
}

bool is_xid_continue(uint32_t cp) {
    return unicodoid_ucd_has_binary_property(cp, UNICODOID_BP_XID_CONTINUE);
}

bool is_id_start(uint32_t cp) {
    return unicodoid_ucd_has_binary_property(cp, UNICODOID_BP_ID_START);
}

bool is_id_continue(uint32_t cp) {
    return unicodoid_ucd_has_binary_property(cp, UNICODOID_BP_ID_CONTINUE);
}

bool is_valid(const std::u32string& cps) {
    if (cps.empty()) return false;
    for (const char32_t ch : cps) {
        const uint32_t cp = static_cast<uint32_t>(ch);
        if (cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF)) return false;
    }
    if (!is_xid_start(static_cast<uint32_t>(cps.front()))) return false;
    for (std::size_t i = 1; i < cps.size(); ++i) {
        const uint32_t cp = static_cast<uint32_t>(cps[i]);
        if (cp == 0x200C || cp == 0x200D) continue;
        if (!is_xid_continue(cp)) return false;
    }
    return true;
}

bool is_nfkc_stable(const std::u32string& cps) {
    if (cps.empty()) return true;
    return uax15::normalize(cps, uax15::Form::Nfkc) == cps;
}

} // namespace uax31
} // namespace unicodoid
