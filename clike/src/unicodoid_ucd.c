#include "unicodoid/unicodoid_ucd.h"

#include <string.h>

static uint16_t range_lookup(
    const unicodoid_ucd_range_t* ranges,
    size_t count,
    uint32_t cp,
    uint16_t fallback
) {
    size_t lo = 0;
    size_t hi = count;
    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        if (cp < ranges[mid].start) {
            hi = mid;
        } else if (cp > ranges[mid].end) {
            lo = mid + 1;
        } else {
            return ranges[mid].value;
        }
    }
    return fallback;
}

static const unicodoid_ucd_map_t* map_lookup(
    const unicodoid_ucd_map_t* entries,
    size_t count,
    uint32_t cp
) {
    size_t lo = 0;
    size_t hi = count;
    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        if (cp < entries[mid].cp) {
            hi = mid;
        } else if (cp > entries[mid].cp) {
            lo = mid + 1;
        } else {
            return &entries[mid];
        }
    }
    return NULL;
}

static uint32_t pair_lookup(
    const unicodoid_ucd_pair_t* entries,
    size_t count,
    uint32_t cp,
    uint32_t fallback
) {
    size_t lo = 0;
    size_t hi = count;
    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        if (cp < entries[mid].cp) {
            hi = mid;
        } else if (cp > entries[mid].cp) {
            lo = mid + 1;
        } else {
            return entries[mid].mapped;
        }
    }
    return fallback;
}

#define RANGE_FN(fn, arr, count, fallback)                                    \
    uint8_t fn(uint32_t cp) {                                                   \
        return (uint8_t)range_lookup(                                          \
            (const unicodoid_ucd_range_t*)arr, (size_t)(count), cp,          \
            (uint16_t)(fallback));                                              \
    }

RANGE_FN(unicodoid_ucd_general_category, unicodoid_ucd_gc, unicodoid_ucd_gc_count, UNICODOID_GC_CN)
RANGE_FN(unicodoid_ucd_canonical_combining_class, unicodoid_ucd_ccc, unicodoid_ucd_ccc_count, 0)
RANGE_FN(unicodoid_ucd_bidi_class, unicodoid_ucd_bidi, unicodoid_ucd_bidi_count, UNICODOID_BIDI_L)
RANGE_FN(unicodoid_ucd_east_asian_width, unicodoid_ucd_eaw, unicodoid_ucd_eaw_count, 0)
RANGE_FN(unicodoid_ucd_line_break, unicodoid_ucd_line_break_range, unicodoid_ucd_line_break_range_count, UNICODOID_LB_XX)
RANGE_FN(unicodoid_ucd_grapheme_break, unicodoid_ucd_grapheme, unicodoid_ucd_grapheme_count, UNICODOID_GCB_OTHER)
RANGE_FN(unicodoid_ucd_word_break, unicodoid_ucd_word, unicodoid_ucd_word_count, UNICODOID_WB_OTHER)
RANGE_FN(unicodoid_ucd_sentence_break, unicodoid_ucd_sentence, unicodoid_ucd_sentence_count, UNICODOID_SB_OTHER)
RANGE_FN(unicodoid_ucd_vertical_orientation, unicodoid_ucd_vertical, unicodoid_ucd_vertical_count, UNICODOID_VO_U)
RANGE_FN(unicodoid_ucd_joining_type, unicodoid_ucd_joining, unicodoid_ucd_joining_count, UNICODOID_JT_U)
RANGE_FN(unicodoid_ucd_indic_conjunct_break, unicodoid_ucd_incb, unicodoid_ucd_incb_count, UNICODOID_INCB_NONE)

uint16_t unicodoid_ucd_script(uint32_t cp) {
    return range_lookup(
        unicodoid_ucd_script_range, (size_t)unicodoid_ucd_script_range_count,
        cp, 0
    );
}

uint16_t unicodoid_ucd_block(uint32_t cp) {
    return range_lookup(
        unicodoid_ucd_block_range, (size_t)unicodoid_ucd_block_range_count,
        cp, 0
    );
}

uint8_t unicodoid_ucd_age(uint32_t cp) {
    return (uint8_t)range_lookup(
        unicodoid_ucd_age_range, (size_t)unicodoid_ucd_age_range_count, cp, 0
    );
}

bool unicodoid_ucd_has_binary_property(uint32_t cp, unicodoid_binary_property_t property) {
    const unicodoid_ucd_range_t* table = NULL;
    size_t count = 0;
    switch (property) {
        case UNICODOID_BP_ALPHABETIC:
            table = unicodoid_ucd_prop_alphabetic;
            count = unicodoid_ucd_prop_alphabetic_count;
            break;
        case UNICODOID_BP_LOWERCASE:
            table = unicodoid_ucd_prop_lowercase;
            count = unicodoid_ucd_prop_lowercase_count;
            break;
        case UNICODOID_BP_UPPERCASE:
            table = unicodoid_ucd_prop_uppercase;
            count = unicodoid_ucd_prop_uppercase_count;
            break;
        case UNICODOID_BP_CASED:
            table = unicodoid_ucd_prop_cased;
            count = unicodoid_ucd_prop_cased_count;
            break;
        case UNICODOID_BP_CASE_IGNORABLE:
            table = unicodoid_ucd_prop_case_ignorable;
            count = unicodoid_ucd_prop_case_ignorable_count;
            break;
        case UNICODOID_BP_MATH:
            table = unicodoid_ucd_prop_math;
            count = unicodoid_ucd_prop_math_count;
            break;
        case UNICODOID_BP_ID_START:
            table = unicodoid_ucd_prop_id_start;
            count = unicodoid_ucd_prop_id_start_count;
            break;
        case UNICODOID_BP_ID_CONTINUE:
            table = unicodoid_ucd_prop_id_continue;
            count = unicodoid_ucd_prop_id_continue_count;
            break;
        case UNICODOID_BP_XID_START:
            table = unicodoid_ucd_prop_xid_start;
            count = unicodoid_ucd_prop_xid_start_count;
            break;
        case UNICODOID_BP_XID_CONTINUE:
            table = unicodoid_ucd_prop_xid_continue;
            count = unicodoid_ucd_prop_xid_continue_count;
            break;
        case UNICODOID_BP_DEFAULT_IGNORABLE_CODE_POINT:
            table = unicodoid_ucd_prop_default_ignorable_code_point;
            count = unicodoid_ucd_prop_default_ignorable_code_point_count;
            break;
        case UNICODOID_BP_GRAPHEME_EXTEND:
            table = unicodoid_ucd_prop_grapheme_extend;
            count = unicodoid_ucd_prop_grapheme_extend_count;
            break;
        case UNICODOID_BP_GRAPHEME_BASE:
            table = unicodoid_ucd_prop_grapheme_base;
            count = unicodoid_ucd_prop_grapheme_base_count;
            break;
        case UNICODOID_BP_GRAPHEME_LINK:
            table = unicodoid_ucd_prop_grapheme_link;
            count = unicodoid_ucd_prop_grapheme_link_count;
            break;
        case UNICODOID_BP_WHITE_SPACE:
            table = unicodoid_ucd_prop_white_space;
            count = unicodoid_ucd_prop_white_space_count;
            break;
        case UNICODOID_BP_PATTERN_SYNTAX:
            table = unicodoid_ucd_prop_pattern_syntax;
            count = unicodoid_ucd_prop_pattern_syntax_count;
            break;
        case UNICODOID_BP_PATTERN_WHITE_SPACE:
            table = unicodoid_ucd_prop_pattern_white_space;
            count = unicodoid_ucd_prop_pattern_white_space_count;
            break;
        case UNICODOID_BP_DASH:
            table = unicodoid_ucd_prop_dash;
            count = unicodoid_ucd_prop_dash_count;
            break;
        case UNICODOID_BP_HYPHEN:
            table = unicodoid_ucd_prop_hyphen;
            count = unicodoid_ucd_prop_hyphen_count;
            break;
        case UNICODOID_BP_QUOTATION_MARK:
            table = unicodoid_ucd_prop_quotation_mark;
            count = unicodoid_ucd_prop_quotation_mark_count;
            break;
        case UNICODOID_BP_TERMINAL_PUNCTUATION:
            table = unicodoid_ucd_prop_terminal_punctuation;
            count = unicodoid_ucd_prop_terminal_punctuation_count;
            break;
        case UNICODOID_BP_DIACRITIC:
            table = unicodoid_ucd_prop_diacritic;
            count = unicodoid_ucd_prop_diacritic_count;
            break;
        case UNICODOID_BP_EXTENDER:
            table = unicodoid_ucd_prop_extender;
            count = unicodoid_ucd_prop_extender_count;
            break;
        case UNICODOID_BP_SOFT_DOTTED:
            table = unicodoid_ucd_prop_soft_dotted;
            count = unicodoid_ucd_prop_soft_dotted_count;
            break;
        case UNICODOID_BP_JOIN_CONTROL:
            table = unicodoid_ucd_prop_join_control;
            count = unicodoid_ucd_prop_join_control_count;
            break;
        case UNICODOID_BP_BIDI_CONTROL:
            table = unicodoid_ucd_prop_bidi_control;
            count = unicodoid_ucd_prop_bidi_control_count;
            break;
        case UNICODOID_BP_NONCHARACTER_CODE_POINT:
            table = unicodoid_ucd_prop_noncharacter_code_point;
            count = unicodoid_ucd_prop_noncharacter_code_point_count;
            break;
        case UNICODOID_BP_VARIATION_SELECTOR:
            table = unicodoid_ucd_prop_variation_selector;
            count = unicodoid_ucd_prop_variation_selector_count;
            break;
        case UNICODOID_BP_EMOJI:
            table = unicodoid_ucd_prop_emoji;
            count = unicodoid_ucd_prop_emoji_count;
            break;
        case UNICODOID_BP_EMOJI_PRESENTATION:
            table = unicodoid_ucd_prop_emoji_presentation;
            count = unicodoid_ucd_prop_emoji_presentation_count;
            break;
        case UNICODOID_BP_EMOJI_MODIFIER:
            table = unicodoid_ucd_prop_emoji_modifier;
            count = unicodoid_ucd_prop_emoji_modifier_count;
            break;
        case UNICODOID_BP_EMOJI_MODIFIER_BASE:
            table = unicodoid_ucd_prop_emoji_modifier_base;
            count = unicodoid_ucd_prop_emoji_modifier_base_count;
            break;
        case UNICODOID_BP_EMOJI_COMPONENT:
            table = unicodoid_ucd_prop_emoji_component;
            count = unicodoid_ucd_prop_emoji_component_count;
            break;
        case UNICODOID_BP_EXTENDED_PICTOGRAPHIC:
            table = unicodoid_ucd_prop_extended_pictographic;
            count = unicodoid_ucd_prop_extended_pictographic_count;
            break;
        case UNICODOID_BP_UNIFIED_IDEOGRAPH:
            table = unicodoid_ucd_prop_unified_ideograph;
            count = unicodoid_ucd_prop_unified_ideograph_count;
            break;
        case UNICODOID_BP_RADICAL:
            table = unicodoid_ucd_prop_radical;
            count = unicodoid_ucd_prop_radical_count;
            break;
        case UNICODOID_BP_DEPRECATED:
            table = unicodoid_ucd_prop_deprecated;
            count = unicodoid_ucd_prop_deprecated_count;
            break;
        case UNICODOID_BP_FULL_COMPOSITION_EXCLUSION:
            table = unicodoid_ucd_prop_full_composition_exclusion;
            count = unicodoid_ucd_prop_full_composition_exclusion_count;
            break;
        case UNICODOID_BP_SENTENCE_TERMINAL:
            table = unicodoid_ucd_prop_sentence_terminal;
            count = unicodoid_ucd_prop_sentence_terminal_count;
            break;
        case UNICODOID_BP_REGIONAL_INDICATOR:
            table = unicodoid_ucd_prop_regional_indicator;
            count = unicodoid_ucd_prop_regional_indicator_count;
            break;
        case UNICODOID_BP_MODIFIER_COMBINING_MARK:
            table = unicodoid_ucd_prop_modifier_combining_mark;
            count = unicodoid_ucd_prop_modifier_combining_mark_count;
            break;
        default:
            return false;
    }
    return range_lookup(table, count, cp, 0) != 0;
}

#define NAME_FN(fn, table, count_name, fallback)                                 \
    const char* fn(uint8_t value) {                                              \
        if (value >= count_name) return fallback;                                \
        return table[value];                                                     \
    }

NAME_FN(unicodoid_ucd_general_category_name, unicodoid_ucd_gc_names,
        unicodoid_ucd_gc_names_count, "Cn")
NAME_FN(unicodoid_ucd_bidi_class_name, unicodoid_ucd_bidi_names,
        unicodoid_ucd_bidi_names_count, "L")
NAME_FN(unicodoid_ucd_east_asian_width_name, unicodoid_ucd_eaw_names,
        unicodoid_ucd_eaw_names_count, "N")
NAME_FN(unicodoid_ucd_line_break_name, unicodoid_ucd_line_break_names,
        unicodoid_ucd_line_break_names_count, "XX")
NAME_FN(unicodoid_ucd_grapheme_break_name, unicodoid_ucd_grapheme_names,
        unicodoid_ucd_grapheme_names_count, "Other")
NAME_FN(unicodoid_ucd_word_break_name, unicodoid_ucd_word_names,
        unicodoid_ucd_word_names_count, "Other")
NAME_FN(unicodoid_ucd_sentence_break_name, unicodoid_ucd_sentence_names,
        unicodoid_ucd_sentence_names_count, "Other")
NAME_FN(unicodoid_ucd_vertical_orientation_name, unicodoid_ucd_vertical_names,
        unicodoid_ucd_vertical_names_count, "U")
NAME_FN(unicodoid_ucd_joining_type_name, unicodoid_ucd_joining_names,
        unicodoid_ucd_joining_names_count, "U")
NAME_FN(unicodoid_ucd_indic_conjunct_break_name, unicodoid_ucd_incb_names,
        unicodoid_ucd_incb_names_count, "None")

const char* unicodoid_ucd_script_name(uint16_t value) {
    if (value >= unicodoid_ucd_script_names_count) return "Unknown";
    return unicodoid_ucd_script_names[value];
}

const char* unicodoid_ucd_block_name(uint16_t value) {
    if (value >= unicodoid_ucd_block_names_count) return "No_Block";
    return unicodoid_ucd_block_names[value];
}

const char* unicodoid_ucd_age_name(uint8_t value) {
    if (value >= unicodoid_ucd_age_names_count) return "Unassigned";
    return unicodoid_ucd_age_names[value];
}

bool unicodoid_ucd_numeric_value(uint32_t cp, int64_t* numerator, int64_t* denominator) {
    if (!numerator || !denominator) return false;
    size_t lo = 0;
    size_t hi = unicodoid_ucd_numeric_count;
    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        if (cp < unicodoid_ucd_numeric[mid].cp) {
            hi = mid;
        } else if (cp > unicodoid_ucd_numeric[mid].cp) {
            lo = mid + 1;
        } else {
            *numerator = unicodoid_ucd_numeric[mid].numerator;
            *denominator = unicodoid_ucd_numeric[mid].denominator;
            return true;
        }
    }
    return false;
}

int32_t unicodoid_ucd_decimal_digit(uint32_t cp) {
    size_t lo = 0;
    size_t hi = unicodoid_ucd_numeric_count;
    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        if (cp < unicodoid_ucd_numeric[mid].cp) {
            hi = mid;
        } else if (cp > unicodoid_ucd_numeric[mid].cp) {
            lo = mid + 1;
        } else {
            int64_t digit = unicodoid_ucd_numeric[mid].digit;
            return digit >= 0 ? (int32_t)digit : -1;
        }
    }
    return -1;
}

static size_t copy_mapping(const uint32_t* pool, uint32_t offset, uint16_t length,
                           uint32_t* out, size_t cap) {
    if (!out || cap < length) return length;
    for (uint16_t i = 0; i < length; ++i) out[i] = pool[offset + i];
    return length;
}

static size_t full_mapping(
    const unicodoid_ucd_map_t* entries, size_t count,
    uint32_t cp, uint32_t* out, size_t cap
) {
    const unicodoid_ucd_map_t* entry = map_lookup(entries, count, cp);
    if (entry) return copy_mapping(unicodoid_ucd_pool, entry->offset, entry->length, out, cap);
    if (!out || cap < 1) return 1;
    out[0] = cp;
    return 1;
}

size_t unicodoid_ucd_to_uppercase(uint32_t cp, uint32_t* out, size_t cap) {
    size_t n = full_mapping(unicodoid_ucd_upper_full, unicodoid_ucd_upper_full_count, cp, out, cap);
    if (n == 1 && out && cap >= 1 && out[0] == cp) {
        uint32_t mapped = pair_lookup(
            unicodoid_ucd_simple_upper, unicodoid_ucd_simple_upper_count, cp, cp
        );
        out[0] = mapped;
    }
    return n;
}

size_t unicodoid_ucd_to_lowercase(uint32_t cp, uint32_t* out, size_t cap) {
    size_t n = full_mapping(unicodoid_ucd_lower_full, unicodoid_ucd_lower_full_count, cp, out, cap);
    if (n == 1 && out && cap >= 1 && out[0] == cp) {
        uint32_t mapped = pair_lookup(
            unicodoid_ucd_simple_lower, unicodoid_ucd_simple_lower_count, cp, cp
        );
        out[0] = mapped;
    }
    return n;
}

size_t unicodoid_ucd_to_titlecase(uint32_t cp, uint32_t* out, size_t cap) {
    size_t n = full_mapping(unicodoid_ucd_title_full, unicodoid_ucd_title_full_count, cp, out, cap);
    if (n == 1 && out && cap >= 1 && out[0] == cp) {
        uint32_t mapped = pair_lookup(
            unicodoid_ucd_simple_title, unicodoid_ucd_simple_title_count, cp, cp
        );
        out[0] = mapped;
    }
    return n;
}

size_t unicodoid_ucd_to_casefold(uint32_t cp, uint32_t* out, size_t cap) {
    size_t n = full_mapping(unicodoid_ucd_fold_full, unicodoid_ucd_fold_full_count, cp, out, cap);
    if (n == 1 && out && cap >= 1 && out[0] == cp) {
        uint32_t mapped = pair_lookup(
            unicodoid_ucd_simple_fold, unicodoid_ucd_simple_fold_count, cp, cp
        );
        out[0] = mapped;
    }
    return n;
}

uint32_t unicodoid_ucd_simple_uppercase(uint32_t cp) {
    return pair_lookup(unicodoid_ucd_simple_upper, unicodoid_ucd_simple_upper_count, cp, cp);
}

uint32_t unicodoid_ucd_simple_lowercase(uint32_t cp) {
    return pair_lookup(unicodoid_ucd_simple_lower, unicodoid_ucd_simple_lower_count, cp, cp);
}

uint32_t unicodoid_ucd_simple_titlecase(uint32_t cp) {
    return pair_lookup(unicodoid_ucd_simple_title, unicodoid_ucd_simple_title_count, cp, cp);
}

size_t unicodoid_ucd_decomposition(uint32_t cp, bool compatibility, uint32_t* out, size_t cap) {
    const unicodoid_ucd_map_t* entry = map_lookup(
        unicodoid_ucd_decomp, unicodoid_ucd_decomp_count, cp
    );
    if (!entry) return 0;
    if (entry->kind == 1 && !compatibility) return 0;
    return copy_mapping(unicodoid_ucd_pool, entry->offset, entry->length, out, cap);
}

bool unicodoid_ucd_has_decomposition(uint32_t cp, bool compatibility) {
    const unicodoid_ucd_map_t* entry = map_lookup(
        unicodoid_ucd_decomp, unicodoid_ucd_decomp_count, cp
    );
    return entry != NULL && (compatibility || entry->kind == 0);
}

bool unicodoid_ucd_compose_pair(uint32_t first, uint32_t second, uint32_t* out) {
    if (!out) return false;
    size_t lo = 0;
    size_t hi = unicodoid_ucd_composition_count;
    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        const unicodoid_ucd_comp_t* e = &unicodoid_ucd_composition[mid];
        if (first < e->first || (first == e->first && second < e->second)) {
            hi = mid;
        } else if (first > e->first || (first == e->first && second > e->second)) {
            lo = mid + 1;
        } else {
            *out = e->composed;
            return true;
        }
    }
    return false;
}

size_t unicodoid_ucd_confusable(uint32_t cp, uint32_t* out, size_t cap) {
    const unicodoid_ucd_map_t* entry = map_lookup(
        unicodoid_ucd_confusables, unicodoid_ucd_confusables_count, cp
    );
    if (!entry) return 0;
    return copy_mapping(unicodoid_ucd_pool, entry->offset, entry->length, out, cap);
}

int unicodoid_ucd_idna_status(uint32_t cp, uint32_t* out, size_t* out_len) {
    int status = (int)range_lookup(
        unicodoid_ucd_idna_status_range, (size_t)unicodoid_ucd_idna_status_range_count,
        cp, 4
    );
    size_t n = 0;
    if (status == 1 || status == 2) {
        const unicodoid_ucd_map_t* entry = map_lookup(
            unicodoid_ucd_idna_mappings, unicodoid_ucd_idna_mappings_count, cp
        );
        if (entry) {
            n = entry->length;
            if (out && out_len && *out_len >= n) {
                for (size_t i = 0; i < n; ++i) out[i] = unicodoid_ucd_pool[entry->offset + i];
            }
        }
    }
    if (out_len) *out_len = n;
    return status;
}

bool unicodoid_ucd_idna_nv8(uint32_t cp) {
    return range_lookup(
        unicodoid_ucd_idna_nv8_range, (size_t)unicodoid_ucd_idna_nv8_range_count,
        cp, 0
    ) != 0;
}

size_t unicodoid_ucd_collation_weights(uint32_t cp, unicodoid_ucd_weight_value_t* out, size_t cap) {
    size_t lo = 0;
    size_t hi = unicodoid_ucd_collation_single_count;
    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        if (cp < unicodoid_ucd_collation_single[mid].cp) {
            hi = mid;
        } else if (cp > unicodoid_ucd_collation_single[mid].cp) {
            lo = mid + 1;
        } else {
            const unicodoid_ucd_seq_t* e = &unicodoid_ucd_collation_single[mid];
            if (out && cap >= e->length) {
                for (uint16_t i = 0; i < e->length; ++i) {
                    const uint32_t* w = &unicodoid_ucd_collation_keys[(e->offset + i) * 4];
                    out[i].primary = (uint16_t)w[0];
                    out[i].secondary = (uint16_t)w[1];
                    out[i].tertiary = (uint16_t)w[2];
                    out[i].variable = w[3] != 0;
                }
            }
            return e->length;
        }
    }
    return 0;
}

size_t unicodoid_ucd_collation_contraction(
    const uint32_t* cps, size_t length, size_t* matched,
    unicodoid_ucd_weight_value_t* out, size_t cap
) {
    if (matched) *matched = 0;
    size_t best = 0;
    const unicodoid_ucd_seq_t* best_entry = NULL;
    for (size_t i = 0; i < unicodoid_ucd_collation_multi_count; ++i) {
        const unicodoid_ucd_seq_t* e = &unicodoid_ucd_collation_multi[i];
        size_t n = (size_t)(e->cp >> 24);
        size_t off = (size_t)(e->cp & 0xFFFFFFu);
        if (n == 0 || n > length || n <= best) continue;
        bool equal = true;
        for (size_t j = 0; j < n; ++j) {
            if (cps[j] != unicodoid_ucd_pool[off + j]) { equal = false; break; }
        }
        if (equal) {
            best = n;
            best_entry = e;
        }
    }
    if (!best_entry) return 0;
    if (matched) *matched = best;
    if (out && cap >= best_entry->length) {
        for (uint16_t i = 0; i < best_entry->length; ++i) {
            const uint32_t* w = &unicodoid_ucd_collation_keys[(best_entry->offset + i) * 4];
            out[i].primary = (uint16_t)w[0];
            out[i].secondary = (uint16_t)w[1];
            out[i].tertiary = (uint16_t)w[2];
            out[i].variable = w[3] != 0;
        }
    }
    return best_entry->length;
}

bool unicodoid_ucd_implicit_weight(uint32_t cp, uint16_t* base, uint32_t* offset) {
    for (size_t i = 0; i < unicodoid_ucd_implicit_count; ++i) {
        if (cp >= unicodoid_ucd_implicit[i].start && cp <= unicodoid_ucd_implicit[i].end) {
            if (base) *base = (uint16_t)unicodoid_ucd_implicit[i].base;
            if (offset) *offset = cp - unicodoid_ucd_implicit[i].offset_base;
            return true;
        }
    }
    return false;
}

uint8_t unicodoid_ucd_link_term(uint32_t cp) {
    return (uint8_t)range_lookup(
        unicodoid_ucd_link_term_range, (size_t)unicodoid_ucd_link_term_range_count, cp, 2);
}

bool unicodoid_ucd_link_bracket(uint32_t cp, uint32_t* open_cp) {
    size_t lo = 0;
    size_t hi = unicodoid_ucd_link_bracket_map_count;
    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        if (cp < unicodoid_ucd_link_bracket_map[mid].cp) {
            hi = mid;
        } else if (cp > unicodoid_ucd_link_bracket_map[mid].cp) {
            lo = mid + 1;
        } else {
            if (open_cp) *open_cp = unicodoid_ucd_link_bracket_map[mid].mapped;
            return true;
        }
    }
    return false;
}

bool unicodoid_ucd_link_email(uint32_t cp) {
    return range_lookup(
        unicodoid_ucd_link_email_range, (size_t)unicodoid_ucd_link_email_range_count,
        cp, 0) != 0;
}

uint8_t unicodoid_ucd_east_asian_spacing(uint32_t cp) {
    return (uint8_t)range_lookup(
        unicodoid_ucd_east_asian_spacing_range,
        (size_t)unicodoid_ucd_east_asian_spacing_range_count, cp, 3);
}

size_t unicodoid_ucd_named_sequences_count(void) {
    return (size_t)unicodoid_ucd_named_seq_count;
}

const char* unicodoid_ucd_named_seq_name(size_t index) {
    if (index >= unicodoid_ucd_named_seq_count) return NULL;
    return unicodoid_ucd_named_seq_names[index];
}

size_t unicodoid_ucd_named_seq(size_t index, uint32_t* out, size_t cap) {
    if (index >= unicodoid_ucd_named_seq_count) return (size_t)-1;
    const unicodoid_ucd_named_seq_t* e = &unicodoid_ucd_named_seq_index[index];
    size_t n = e->length;
    if (out && cap >= n) {
        for (size_t i = 0; i < n; ++i) out[i] = unicodoid_ucd_pool[e->offset + i];
    }
    return n;
}

bool unicodoid_ucd_named_seq_find(const char* name, size_t* index) {
    if (!name) return false;
    for (size_t i = 0; i < unicodoid_ucd_named_seq_count; ++i) {
        if (strcmp(unicodoid_ucd_named_seq_names[i], name) == 0) {
            if (index) *index = i;
            return true;
        }
    }
    return false;
}

size_t unicodoid_ucd_emoji_sequences_count(void) {
    return (size_t)unicodoid_ucd_emoji_seq_count;
}

const char* unicodoid_ucd_emoji_seq_type(size_t index) {
    if (index >= unicodoid_ucd_emoji_seq_count) return NULL;
    return unicodoid_ucd_emoji_seq_types[index];
}

size_t unicodoid_ucd_emoji_seq(size_t index, uint32_t* out, size_t cap) {
    if (index >= unicodoid_ucd_emoji_seq_count) return (size_t)-1;
    const unicodoid_ucd_named_seq_t* e = &unicodoid_ucd_emoji_seq_index[index];
    size_t n = e->length;
    if (out && cap >= n) {
        for (size_t i = 0; i < n; ++i) out[i] = unicodoid_ucd_pool[e->offset + i];
    }
    return n;
}

const char* unicodoid_ucd_version(void) { return UNICODOID_UCD_VERSION; }
const char* unicodoid_ucd_generator_version(void) { return UNICODOID_UCD_GENERATOR; }
const char* unicodoid_ucd_source_checksum(void) { return UNICODOID_UCD_CHECKSUM; }
