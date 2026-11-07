// Native UTS #58 Link Detection and Formatting for the C++ API.
//
// Implements the post-host code point machinery of UTS #58 on top of the
// generated Unicode 18.0.0 property tables.

#include "unicodoid/uts/uts58_link_detection.hpp"
#include "unicodoid/unicodoid_ucd.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace unicodoid {
namespace uts58 {
namespace {

constexpr uint8_t kLtInclude = 1;
constexpr uint8_t kLtHard = 2;
constexpr uint8_t kLtSoft = 3;
constexpr uint8_t kLtClose = 4;
constexpr uint8_t kLtOpen = 5;

constexpr int kPartNone = -1;
constexpr int kPartPath = 0;
constexpr int kPartQuery = 1;
constexpr int kPartFragment = 2;
constexpr int kPartDirective = 3;

constexpr std::size_t kStackCap = 125;

bool is_colon_tilde_colon(const std::u32string& cps, std::size_t i) {
    return i + 2 < cps.size() && cps[i] == U':' && cps[i + 1] == U'~' &&
           cps[i + 2] == U':';
}

bool match_terminator(
    const std::u32string& cps, std::size_t i, int part, std::size_t& advance) {
    advance = 0;
    const char32_t c = cps[i];
    switch (part) {
        case kPartPath:
            if (c == U'?' || c == U'#') {
                advance = 1;
                return true;
            }
            return false;
        case kPartQuery:
            if (c == U'#') {
                advance = 1;
                return true;
            }
            return false;
        case kPartFragment:
            if (is_colon_tilde_colon(cps, i)) {
                advance = 3;
                return true;
            }
            return false;
        default:
            return false;
    }
}

bool match_initiator(
    const std::u32string& cps,
    std::size_t i,
    int previous_part,
    std::size_t& advance,
    int& new_part) {
    advance = 0;
    new_part = kPartNone;
    if (i < cps.size() && cps[i] == U'/') {
        advance = 1;
        new_part = kPartPath;
        return true;
    }
    if (i < cps.size() && cps[i] == U'?') {
        advance = 1;
        new_part = kPartQuery;
        return true;
    }
    if (i < cps.size() && cps[i] == U'#') {
        advance = 1;
        new_part = kPartFragment;
        return true;
    }
    if (is_colon_tilde_colon(cps, i) &&
        (previous_part == kPartFragment || previous_part == kPartDirective)) {
        advance = 3;
        new_part = kPartDirective;
        return true;
    }
    return false;
}

bool match_clear_stack_open(
    const std::u32string& cps, std::size_t i, int part, std::size_t& advance) {
    advance = 0;
    const char32_t c = cps[i];
    switch (part) {
        case kPartPath:
            if (c == U'/') {
                advance = 1;
                return true;
            }
            return false;
        case kPartQuery:
            if (c == U'=' || c == U'&') {
                advance = 1;
                return true;
            }
            return false;
        case kPartDirective:
            if (c == U'&' || c == U',') {
                advance = 1;
                return true;
            }
            if (is_colon_tilde_colon(cps, i)) {
                advance = 3;
                return true;
            }
            return false;
        default:
            return false;
    }
}

int part_number(UrlPart part) {
    switch (part) {
        case UrlPart::Path: return kPartPath;
        case UrlPart::Query: return kPartQuery;
        case UrlPart::Fragment: return kPartFragment;
        case UrlPart::FragmentDirective: return kPartDirective;
    }
    return kPartPath;
}

const char* part_initiator(UrlPart part, std::size_t& length) {
    switch (part) {
        case UrlPart::Path: length = 1; return "/";
        case UrlPart::Query: length = 1; return "?";
        case UrlPart::Fragment: length = 1; return "#";
        case UrlPart::FragmentDirective: length = 3; return ":~:";
    }
    length = 0;
    return "";
}

std::size_t utf8_encode(uint32_t cp, uint8_t out[4]) {
    if (cp < 0x80) {
        out[0] = static_cast<uint8_t>(cp);
        return 1;
    }
    if (cp < 0x800) {
        out[0] = static_cast<uint8_t>(0xC0 | (cp >> 6));
        out[1] = static_cast<uint8_t>(0x80 | (cp & 0x3F));
        return 2;
    }
    if (cp < 0x10000) {
        out[0] = static_cast<uint8_t>(0xE0 | (cp >> 12));
        out[1] = static_cast<uint8_t>(0x80 | ((cp >> 6) & 0x3F));
        out[2] = static_cast<uint8_t>(0x80 | (cp & 0x3F));
        return 3;
    }
    out[0] = static_cast<uint8_t>(0xF0 | (cp >> 18));
    out[1] = static_cast<uint8_t>(0x80 | ((cp >> 12) & 0x3F));
    out[2] = static_cast<uint8_t>(0x80 | ((cp >> 6) & 0x3F));
    out[3] = static_cast<uint8_t>(0x80 | (cp & 0x3F));
    return 4;
}

void push_cp(std::vector<uint8_t>& out, uint32_t cp) {
    uint8_t buffer[4];
    const std::size_t n = utf8_encode(cp, buffer);
    out.insert(out.end(), buffer, buffer + n);
}

void push_percent(std::vector<uint8_t>& out, uint32_t cp) {
    uint8_t buffer[4];
    const std::size_t n = utf8_encode(cp, buffer);
    constexpr char hex[] = "0123456789ABCDEF";
    for (std::size_t i = 0; i < n; ++i) {
        out.push_back(static_cast<uint8_t>('%'));
        out.push_back(static_cast<uint8_t>(hex[buffer[i] >> 4]));
        out.push_back(static_cast<uint8_t>(hex[buffer[i] & 0x0F]));
    }
}

} // namespace

std::size_t link_detection_end(const std::u32string& cps, std::size_t start) {
    std::size_t last_safe = start;
    int part = kPartNone;
    int previous_part = kPartNone;
    std::size_t i = start;
    uint32_t stack[kStackCap];
    std::size_t stack_len = 0;

    while (i < cps.size()) {
        std::size_t advance = 0;

        // Step (a): a terminator ends the current part.
        if (part != kPartNone && match_terminator(cps, i, part, advance)) {
            previous_part = part;
            part = kPartNone;
            // Do not advance i; the code point is reconsidered below.
        }

        // Step (b): with no part active, try to start one.
        if (part == kPartNone) {
            int new_part = kPartNone;
            if (match_initiator(cps, i, previous_part, advance, new_part)) {
                part = new_part;
                i += advance;
                last_safe = i;
                stack_len = 0;
                continue;
            }
            return last_safe;
        }

        // Step (c): a clearStackOpen element clears the bracket stack.
        if (match_clear_stack_open(cps, i, part, advance)) {
            i += advance;
            last_safe = i;
            stack_len = 0;
            continue;
        }

        // Steps (d)-(i): classify the code point with Link_Term.
        switch (unicodoid_ucd_link_term(static_cast<uint32_t>(cps[i]))) {
            case kLtInclude:
                last_safe = i + 1;
                ++i;
                break;
            case kLtSoft:
                ++i;
                break;
            case kLtHard:
                return last_safe;
            case kLtOpen:
                if (stack_len == kStackCap) return last_safe;
                stack[stack_len++] = static_cast<uint32_t>(cps[i]);
                last_safe = i + 1;
                ++i;
                break;
            case kLtClose: {
                uint32_t expected = 0;
                if (stack_len == 0) return last_safe;
                const uint32_t last_open = stack[--stack_len];
                if (unicodoid_ucd_link_bracket(
                        static_cast<uint32_t>(cps[i]), &expected) &&
                    expected == last_open) {
                    last_safe = i + 1;
                    ++i;
                } else {
                    return last_safe;
                }
                break;
            }
            default:
                return last_safe;
        }
    }

    return last_safe;
}

std::optional<std::size_t> email_local_part_start(
    const std::u32string& cps, std::size_t at) {
    if (at == 0 || at > cps.size()) return std::nullopt;
    if (cps[at - 1] == U'.') return std::nullopt;

    std::size_t start = at;
    for (std::size_t i = at; i-- > 0;) {
        const char32_t cp = cps[i];
        if (cp == U'.') {
            if (i + 1 < at && cps[i + 1] == U'.') return std::nullopt;
            continue;
        }
        if (!unicodoid_ucd_link_email(static_cast<uint32_t>(cp))) {
            start = i + 1;
            break;
        }
        start = i;
    }

    if (start < at && cps[start] == U'.') return std::nullopt;
    if (start == at) return std::nullopt;

    constexpr char32_t mailto[] = {U'm', U'a', U'i', U'l', U't', U'o', U':'};
    if (start >= 7) {
        bool match = true;
        for (std::size_t k = 0; k < 7; ++k) {
            if (cps[start - 7 + k] != mailto[k]) {
                match = false;
                break;
            }
        }
        if (match) start -= 7;
    }

    return start;
}

std::vector<uint8_t> minimal_escape_part(
    UrlPart part, const std::u32string& cps, bool is_last) {
    std::vector<uint8_t> out;

    std::size_t init_len = 0;
    const char* init = part_initiator(part, init_len);
    out.insert(out.end(), init, init + init_len);

    std::size_t copied_already = 0;
    const int p = part_number(part);
    uint32_t stack[kStackCap];
    std::size_t stack_len = 0;

    for (std::size_t i = 0; i < cps.size(); ++i) {
        std::size_t advance = 0;
        const bool is_terminator = match_terminator(cps, i, p, advance);
        const uint8_t lt = is_terminator
                               ? kLtHard
                               : unicodoid_ucd_link_term(
                                     static_cast<uint32_t>(cps[i]));

        if (match_clear_stack_open(cps, i, p, advance)) stack_len = 0;

        switch (lt) {
            case kLtInclude:
                for (std::size_t k = copied_already; k < i; ++k) {
                    push_cp(out, static_cast<uint32_t>(cps[k]));
                }
                push_cp(out, static_cast<uint32_t>(cps[i]));
                copied_already = i + 1;
                break;
            case kLtHard:
                for (std::size_t k = copied_already; k < i; ++k) {
                    push_cp(out, static_cast<uint32_t>(cps[k]));
                }
                push_percent(out, static_cast<uint32_t>(cps[i]));
                copied_already = i + 1;
                break;
            case kLtSoft:
                break;
            case kLtOpen:
                if (stack_len == kStackCap) {
                    for (std::size_t k = copied_already; k < i; ++k) {
                        push_cp(out, static_cast<uint32_t>(cps[k]));
                    }
                    push_percent(out, static_cast<uint32_t>(cps[i]));
                } else {
                    stack[stack_len++] = static_cast<uint32_t>(cps[i]);
                    for (std::size_t k = copied_already; k < i; ++k) {
                        push_cp(out, static_cast<uint32_t>(cps[k]));
                    }
                    push_cp(out, static_cast<uint32_t>(cps[i]));
                }
                copied_already = i + 1;
                break;
            case kLtClose: {
                bool matched = false;
                if (stack_len > 0) {
                    const uint32_t last_open = stack[stack_len - 1];
                    uint32_t expected = 0;
                    matched = unicodoid_ucd_link_bracket(
                                  static_cast<uint32_t>(cps[i]), &expected) &&
                              expected == last_open;
                }
                if (matched) {
                    --stack_len;
                    for (std::size_t k = copied_already; k < i; ++k) {
                        push_cp(out, static_cast<uint32_t>(cps[k]));
                    }
                    push_cp(out, static_cast<uint32_t>(cps[i]));
                } else {
                    for (std::size_t k = copied_already; k < i; ++k) {
                        push_cp(out, static_cast<uint32_t>(cps[k]));
                    }
                    push_percent(out, static_cast<uint32_t>(cps[i]));
                }
                copied_already = i + 1;
                break;
            }
            default:
                break;
        }
    }

    if (!is_last) {
        for (std::size_t k = copied_already; k < cps.size(); ++k) {
            push_cp(out, static_cast<uint32_t>(cps[k]));
        }
    } else if (copied_already < cps.size()) {
        for (std::size_t k = copied_already; k < cps.size() - 1; ++k) {
            push_cp(out, static_cast<uint32_t>(cps[k]));
        }
        push_percent(out, static_cast<uint32_t>(cps[cps.size() - 1]));
    }

    return out;
}

} // namespace uts58
} // namespace unicodoid
