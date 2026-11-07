// Native UTS #51 emoji sequence validation for the C++ API.

#include "unicodoid/uts/uts51_emoji_sequences.hpp"
#include "unicodoid/unicodoid_ucd.h"

#include <cstdint>
#include <string>
#include <vector>

namespace unicodoid {
namespace uts51 {
namespace {

bool valid_scalar(uint32_t cp) {
    return cp <= 0x10FFFF && !(cp >= 0xD800 && cp <= 0xDFFF);
}

bool is_regional_indicator(uint32_t cp) {
    return cp >= 0x1F1E6 && cp <= 0x1F1FF;
}

bool is_emoji_modifier(uint32_t cp) {
    return cp >= 0x1F3FB && cp <= 0x1F3FF;
}

bool is_tag_char(uint32_t cp) {
    return cp >= 0xE0020 && cp <= 0xE007E;
}

bool is_extended_pictographic(uint32_t cp) {
    return unicodoid_ucd_has_binary_property(
        cp, UNICODOID_BP_EXTENDED_PICTOGRAPHIC);
}

bool is_emoji(uint32_t cp) {
    return unicodoid_ucd_has_binary_property(cp, UNICODOID_BP_EMOJI);
}

bool is_emoji_modifier_base(uint32_t cp) {
    return unicodoid_ucd_has_binary_property(
        cp, UNICODOID_BP_EMOJI_MODIFIER_BASE);
}

std::vector<char32_t> fetch(std::size_t index) {
    const std::size_t n = unicodoid_ucd_emoji_seq(index, nullptr, 0);
    if (n == static_cast<std::size_t>(-1)) return {};
    std::vector<uint32_t> buffer(n);
    if (n > 0) unicodoid_ucd_emoji_seq(index, buffer.data(), buffer.size());
    std::vector<char32_t> out;
    out.reserve(n);
    for (const uint32_t cp : buffer) out.push_back(static_cast<char32_t>(cp));
    return out;
}

} // namespace

std::size_t count() {
    return unicodoid_ucd_emoji_sequences_count();
}

std::string_view type(std::size_t index) {
    const char* value = unicodoid_ucd_emoji_seq_type(index);
    return value ? std::string_view(value) : std::string_view();
}

std::optional<std::vector<char32_t>> sequence(std::size_t index) {
    if (index >= count()) return std::nullopt;
    return fetch(index);
}

std::optional<std::string_view> is_rgi(const std::u32string& cps) {
    if (cps.empty()) return std::nullopt;
    for (std::size_t k = 0; k < cps.size(); ++k) {
        if (!valid_scalar(static_cast<uint32_t>(cps[k]))) return std::nullopt;
    }

    const std::size_t total = count();
    std::vector<uint32_t> buffer;
    for (std::size_t i = 0; i < total; ++i) {
        const std::size_t n = unicodoid_ucd_emoji_seq(i, nullptr, 0);
        if (n != cps.size()) continue;
        buffer.resize(n);
        if (n > 0) unicodoid_ucd_emoji_seq(i, buffer.data(), buffer.size());
        bool equal = true;
        for (std::size_t k = 0; k < n; ++k) {
            if (buffer[k] != static_cast<uint32_t>(cps[k])) {
                equal = false;
                break;
            }
        }
        if (equal) return type(i);
    }
    return std::nullopt;
}

bool is_well_formed(const std::u32string& cps) {
    if (cps.empty()) return false;
    for (const char32_t cp : cps) {
        if (!valid_scalar(static_cast<uint32_t>(cp))) return false;
    }

    const std::size_t length = cps.size();
    auto at = [&](std::size_t i) { return static_cast<uint32_t>(cps[i]); };

    // Emoji_Keycap_Sequence: [0-9#*] FE0F 20E3.
    if (length == 3 && at(1) == 0xFE0F && at(2) == 0x20E3) {
        const uint32_t base = at(0);
        return (base >= '0' && base <= '9') || base == '#' || base == '*';
    }

    // RGI_Emoji_Flag_Sequence: exactly two regional indicators.
    if (length == 2 && is_regional_indicator(at(0)) &&
        is_regional_indicator(at(1))) {
        return true;
    }

    // RGI_Emoji_Tag_Sequence: 1F3F4 (tag chars)* E007F.
    if (length >= 3 && at(0) == 0x1F3F4 && at(length - 1) == 0xE007F) {
        for (std::size_t i = 1; i + 1 < length; ++i) {
            if (!is_tag_char(at(i))) return false;
        }
        return true;
    }

    // RGI_Emoji_Modifier_Sequence: modifier base + modifier.
    if (length == 2 && is_emoji_modifier_base(at(0)) &&
        is_emoji_modifier(at(1))) {
        return true;
    }

    // RGI_Emoji_ZWJ_Sequence: ExtPict (modifier/VS16)* (ZWJ ExtPict ...).
    bool saw_zwj = false;
    for (std::size_t i = 0; i < length; ++i) {
        if (at(i) == 0x200D) {
            saw_zwj = true;
            break;
        }
    }
    if (saw_zwj) {
        if (!is_extended_pictographic(at(0))) return false;
        std::size_t i = 1;
        while (i < length) {
            if (at(i) == 0xFE0F || is_emoji_modifier(at(i))) {
                ++i;
                continue;
            }
            if (at(i) != 0x200D) return false;
            ++i;
            if (i >= length || !is_extended_pictographic(at(i))) return false;
            ++i;
        }
        return true;
    }

    // Basic_Emoji: one emoji code point, optionally followed by VS16.
    if (length == 1 || (length == 2 && at(1) == 0xFE0F)) {
        const uint32_t cp = at(0);
        if (is_extended_pictographic(cp) || is_emoji(cp)) return true;
    }

    return false;
}

} // namespace uts51
} // namespace unicodoid
