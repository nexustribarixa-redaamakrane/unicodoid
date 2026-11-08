#ifndef UNICODOID_UTS46_IDNA_COMPAT_HPP
#define UNICODOID_UTS46_IDNA_COMPAT_HPP

#include <cstdint>
#include <string>

namespace unicodoid {
namespace uts46 {

/** UTS #46 Unicode IDNA Compatibility Processing (18.0.0, Rev. 36). */

/** IDNA disposition. */
enum class Disposition : int {
    Valid = 0,
    Mapped = 1,
    Deviation = 2,
    Ignored = 3,
    Disallowed = 4
};

/** Disposition of one scalar (from the generated table). */
Disposition disposition(uint32_t cp);

/** Map a scalar sequence (UTS #46 steps 4..6: map + NFC). */
std::u32string map(const std::u32string& input);

/** True when @p label survives mapping and contains no disallowed scalars. */
bool check_label(const std::u32string& label);

/** True when @p cp must not appear in a transitional label. */
bool is_transitional_disallowed(uint32_t cp);

} // namespace uts46
} // namespace unicodoid

#endif /* UNICODOID_UTS46_IDNA_COMPAT_HPP */
