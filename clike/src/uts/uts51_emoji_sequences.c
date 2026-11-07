#include "unicodoid/uts/uts51_emoji_sequences.h"
#include "unicodoid/unicodoid_ucd.h"

#include <string.h>

static bool valid_scalar(uint32_t cp) {
    return cp <= 0x10FFFF && !(cp >= 0xD800 && cp <= 0xDFFF);
}

static bool is_regional_indicator(uint32_t cp) {
    return cp >= 0x1F1E6 && cp <= 0x1F1FF;
}

static bool is_emoji_modifier(uint32_t cp) {
    return cp >= 0x1F3FB && cp <= 0x1F3FF;
}

static bool is_tag_char(uint32_t cp) {
    return cp >= 0xE0020 && cp <= 0xE007E;
}

static bool is_extended_pictographic(uint32_t cp) {
    return unicodoid_ucd_has_binary_property(
        cp, UNICODOID_BP_EXTENDED_PICTOGRAPHIC);
}

static bool is_emoji_modifier_base(uint32_t cp) {
    return unicodoid_ucd_has_binary_property(
        cp, UNICODOID_BP_EMOJI_MODIFIER_BASE);
}

size_t unicodoid_uts51_count(void) {
    return unicodoid_ucd_emoji_sequences_count();
}

const char* unicodoid_uts51_type(size_t index) {
    return unicodoid_ucd_emoji_seq_type(index);
}

size_t unicodoid_uts51_sequence(size_t index, uint32_t* out, size_t cap) {
    return unicodoid_ucd_emoji_seq(index, out, cap);
}

bool unicodoid_uts51_is_rgi(
    const uint32_t* cps, size_t length, const char** type) {
    if (!cps || length == 0) return false;
    for (size_t i = 0; i < length; ++i) {
        if (!valid_scalar(cps[i])) return false;
    }

    size_t count = unicodoid_uts51_count();
    uint32_t buffer[64];
    for (size_t i = 0; i < count; ++i) {
        size_t n = unicodoid_uts51_sequence(i, buffer, 64);
        if (n != length) continue;
        if (memcmp(buffer, cps, length * sizeof(uint32_t)) == 0) {
            if (type) *type = unicodoid_uts51_type(i);
            return true;
        }
    }
    return false;
}

bool unicodoid_uts51_is_well_formed(const uint32_t* cps, size_t length) {
    if (!cps || length == 0) return false;
    for (size_t i = 0; i < length; ++i) {
        if (!valid_scalar(cps[i])) return false;
    }

    /* Emoji_Keycap_Sequence: [0-9#*] FE0F 20E3. */
    if (length == 3 && cps[1] == 0xFE0F && cps[2] == 0x20E3) {
        uint32_t base = cps[0];
        return (base >= '0' && base <= '9') || base == '#' || base == '*';
    }

    /* RGI_Emoji_Flag_Sequence: exactly two regional indicators. */
    if (length == 2 && is_regional_indicator(cps[0]) &&
        is_regional_indicator(cps[1])) {
        return true;
    }

    /* RGI_Emoji_Tag_Sequence: 1F3F4 (tag chars)* E007F. */
    if (length >= 3 && cps[0] == 0x1F3F4 && cps[length - 1] == 0xE007F) {
        for (size_t i = 1; i + 1 < length; ++i) {
            if (!is_tag_char(cps[i])) return false;
        }
        return true;
    }

    /* RGI_Emoji_Modifier_Sequence: modifier base + modifier. */
    if (length == 2 && is_emoji_modifier_base(cps[0]) &&
        is_emoji_modifier(cps[1])) {
        return true;
    }

    /* RGI_Emoji_ZWJ_Sequence: ExtPict (modifier/VS16)* (ZWJ ExtPict ...). */
    bool saw_zwj = false;
    for (size_t i = 0; i < length; ++i) {
        if (cps[i] == 0x200D) {
            saw_zwj = true;
            break;
        }
    }
    if (saw_zwj) {
        if (!is_extended_pictographic(cps[0])) return false;
        size_t i = 1;
        while (i < length) {
            if (cps[i] == 0xFE0F || is_emoji_modifier(cps[i])) {
                ++i;
                continue;
            }
            if (cps[i] != 0x200D) return false;
            ++i;
            if (i >= length || !is_extended_pictographic(cps[i])) return false;
            ++i;
        }
        return true;
    }

    /* Basic_Emoji: a single emoji code point, optionally followed by VS16.
     * Emoji modifiers are Basic_Emoji in their own right even though they are
     * not Extended_Pictographic. */
    if (length == 1 || (length == 2 && cps[1] == 0xFE0F)) {
        uint32_t cp = cps[0];
        if (is_extended_pictographic(cp) ||
            unicodoid_ucd_has_binary_property(cp, UNICODOID_BP_EMOJI)) {
            return true;
        }
    }

    return false;
}
