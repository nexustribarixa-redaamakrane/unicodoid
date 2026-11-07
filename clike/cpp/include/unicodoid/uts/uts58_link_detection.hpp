#ifndef UNICODOID_UTS58_LINK_DETECTION_HPP
#define UNICODOID_UTS58_LINK_DETECTION_HPP

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace unicodoid {
namespace uts58 {

/**
 * @brief UTS #58 Link Detection and Formatting (Unicode 18.0.0, Revision 3).
 *
 * Implements the post-host code point machinery: link termination (§3.5.1),
 * minimal escaping (§4.1) and the email local-part scan (§5.2).  URL
 * initiation is out of scope, as it is in the specification itself.
 */

/** URL parts for minimal escaping (UTS58 §4.1). */
enum class UrlPart {
    Path = 0,
    Query = 1,
    Fragment = 2,
    FragmentDirective = 3
};

/**
 * @brief Link detection termination algorithm (UTS58 §3.5.1).
 *
 * @param cps   code points of the text,
 * @param start index immediately after the host.
 * @return offset one past the end of the final URL part.
 */
std::size_t link_detection_end(const std::u32string& cps, std::size_t start);

/**
 * @brief Email local-part detection (UTS58 §5.2).
 *
 * @param cps code points of the text,
 * @param at  index of the '@' sign.
 * @return offset of the beginning of the local part, optionally including a
 *         "mailto:" prefix, or nullopt when there is no valid local part.
 */
std::optional<std::size_t> email_local_part_start(
    const std::u32string& cps, std::size_t at);

/**
 * @brief URL Minimal Escaping Algorithm (UTS58 §4.1).
 *
 * @param part     URL part being escaped,
 * @param cps      decoded code points of the part, without the initiator,
 * @param is_last  true when this is the last part of the whole URL.
 * @return the minimally escaped UTF-8 bytes, including the initiator.
 */
std::vector<uint8_t> minimal_escape_part(
    UrlPart part, const std::u32string& cps, bool is_last);

} // namespace uts58
} // namespace unicodoid

#endif /* UNICODOID_UTS58_LINK_DETECTION_HPP */
