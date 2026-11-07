#include "unicodoid/unicodoid_properties.h"
#include "unicodoid/unicodoid_ucd.h"
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------------- */
/* Group helpers (UAX #44)                                                   */
/* ------------------------------------------------------------------------- */

unicodoid_category_t unicodoid_group_from_gc(uint8_t gc) {
    switch (gc) {
        case 0: return UNICODOID_CAT_UNASSIGNED;
        case 1: return UNICODOID_CAT_CONTROL;
        case 2: return UNICODOID_CAT_FORMAT;
        case 3: return UNICODOID_CAT_PRIVATE_USE;
        case 4: return UNICODOID_CAT_SURROGATE;
        case 5: return UNICODOID_CAT_LOWERCASE_LETTER;
        case 6: return UNICODOID_CAT_MODIFIER_LETTER;
        case 7: return UNICODOID_CAT_OTHER_LETTER;
        case 8: return UNICODOID_CAT_TITLECASE_LETTER;
        case 9: return UNICODOID_CAT_UPPERCASE_LETTER;
        case 10: case 11: case 12: return UNICODOID_CAT_MARK;
        case 13: return UNICODOID_CAT_DECIMAL_NUMBER;
        case 14: return UNICODOID_CAT_LETTER_NUMBER;
        case 15: return UNICODOID_CAT_OTHER_NUMBER;
        case 16: return UNICODOID_CAT_PUNCTUATION;
        case 17: return UNICODOID_CAT_PUNCTUATION;
        case 18: return UNICODOID_CAT_PUNCTUATION;
        case 19: return UNICODOID_CAT_PUNCTUATION;
        case 20: return UNICODOID_CAT_PUNCTUATION;
        case 21: return UNICODOID_CAT_PUNCTUATION;
        case 22: return UNICODOID_CAT_PUNCTUATION;
        case 23: return UNICODOID_CAT_SYMBOL;
        case 24: return UNICODOID_CAT_SYMBOL;
        case 25: return UNICODOID_CAT_SYMBOL;
        case 26: return UNICODOID_CAT_SYMBOL;
        case 27: return UNICODOID_CAT_LINE_SEPARATOR;
        case 28: return UNICODOID_CAT_PARAGRAPH_SEPARATOR;
        case 29: return UNICODOID_CAT_SPACE_SEPARATOR;
        default: return UNICODOID_CAT_OTHER;
    }
}


unicodoid_category_t unicodoid_unicode_general_category(uint32_t cp) {
    return unicodoid_group_from_gc((uint8_t)unicodoid_ucd_general_category(cp));
}

unicodoid_ea_width_t unicodoid_unicode_ea_width(uint32_t cp) {
    uint8_t value = unicodoid_ucd_east_asian_width(cp);
    switch (value) {
        case 0: return UNICODOID_EA_NEUTRAL;
        case 1: return UNICODOID_EA_AMBIGUOUS;
        case 2: return UNICODOID_EA_HALFWIDTH;
        case 3: return UNICODOID_EA_FULLWIDTH;
        case 4: return UNICODOID_EA_NARROW;
        case 5: return UNICODOID_EA_WIDE;
        default: return UNICODOID_EA_NEUTRAL;
    }
}

bool unicodoid_unicode_is_alphabetic(uint32_t cp) {
    unicodoid_category_t cat = unicodoid_unicode_general_category(cp);
    return cat == UNICODOID_CAT_UPPERCASE_LETTER ||
           cat == UNICODOID_CAT_LOWERCASE_LETTER ||
           cat == UNICODOID_CAT_TITLECASE_LETTER ||
           cat == UNICODOID_CAT_MODIFIER_LETTER ||
           cat == UNICODOID_CAT_OTHER_LETTER;
}

bool unicodoid_unicode_is_numeric(uint32_t cp) {
    unicodoid_category_t cat = unicodoid_unicode_general_category(cp);
    return cat == UNICODOID_CAT_DECIMAL_NUMBER ||
           cat == UNICODOID_CAT_LETTER_NUMBER ||
           cat == UNICODOID_CAT_OTHER_NUMBER;
}

bool unicodoid_unicode_is_whitespace(uint32_t cp) {
    unicodoid_category_t cat = unicodoid_unicode_general_category(cp);
    return cat == UNICODOID_CAT_SPACE_SEPARATOR ||
           cat == UNICODOID_CAT_LINE_SEPARATOR ||
           cat == UNICODOID_CAT_PARAGRAPH_SEPARATOR ||
           cp == '\t' || cp == '\n' || cp == '\r' || cp == '\f' || cp == '\v';
}

bool unicodoid_unicode_is_control(uint32_t cp) {
    return unicodoid_unicode_general_category(cp) == UNICODOID_CAT_CONTROL;
}

bool unicodoid_unicode_is_punctuation(uint32_t cp) {
    return unicodoid_unicode_general_category(cp) == UNICODOID_CAT_PUNCTUATION;
}

bool unicodoid_unicode_is_mark(uint32_t cp) {
    return unicodoid_unicode_general_category(cp) == UNICODOID_CAT_MARK;
}

int unicodoid_unicode_combining_class(uint32_t cp) {
    /* Canonical combining classes from the generated UCD table; 0 means a starter. */
    return (int)unicodoid_ucd_canonical_combining_class(cp);
}
uint32_t unicodoid_unicode_to_uppercase(uint32_t cp) {
    uint32_t out[3];
    size_t n = unicodoid_ucd_to_uppercase(cp, out, 3);
    return n > 0 ? out[0] : cp;
}

uint32_t unicodoid_unicode_to_lowercase(uint32_t cp) {
    uint32_t out[3];
    size_t n = unicodoid_ucd_to_lowercase(cp, out, 3);
    return n > 0 ? out[0] : cp;
}

static unicodoid_status_t native_to_scalar(
    const unicodoid_system_t* sys,
    unicodoid_char64_t ch,
    uint32_t* out_scalar
) {
    if (!sys || !out_scalar) return UNICODOID_ERR_INVALID_PARAM;
    if (!unicodoid_system_has_unicode_profile(sys)) return UNICODOID_ERR_UNSUPPORTED;
    return sys->unicode_profile->to_unicode(sys->unicode_profile, ch, out_scalar);
}

static unicodoid_status_t scalar_to_native(
    const unicodoid_system_t* sys,
    uint32_t scalar,
    unicodoid_char64_t* out_ch
) {
    if (!sys || !out_ch) return UNICODOID_ERR_INVALID_PARAM;
    if (!unicodoid_system_has_unicode_profile(sys)) return UNICODOID_ERR_UNSUPPORTED;
    return sys->unicode_profile->from_unicode(sys->unicode_profile, scalar, out_ch);
}

unicodoid_status_t unicodoid_get_general_category(
    const unicodoid_system_t* sys,
    unicodoid_char64_t ch,
    unicodoid_category_t* out_category
) {
    if (!out_category) return UNICODOID_ERR_INVALID_PARAM;
    uint32_t cp = 0;
    unicodoid_status_t st = native_to_scalar(sys, ch, &cp);
    if (st != UNICODOID_OK) return st;
    *out_category = unicodoid_unicode_general_category(cp);
    return UNICODOID_OK;
}

unicodoid_status_t unicodoid_get_ea_width(
    const unicodoid_system_t* sys,
    unicodoid_char64_t ch,
    unicodoid_ea_width_t* out_width
) {
    if (!out_width) return UNICODOID_ERR_INVALID_PARAM;
    uint32_t cp = 0;
    unicodoid_status_t st = native_to_scalar(sys, ch, &cp);
    if (st != UNICODOID_OK) return st;
    *out_width = unicodoid_unicode_ea_width(cp);
    return UNICODOID_OK;
}

unicodoid_status_t unicodoid_is_alphabetic(
    const unicodoid_system_t* sys,
    unicodoid_char64_t ch,
    bool* out_value
) {
    if (!out_value) return UNICODOID_ERR_INVALID_PARAM;
    uint32_t cp = 0;
    unicodoid_status_t st = native_to_scalar(sys, ch, &cp);
    if (st != UNICODOID_OK) return st;
    *out_value = unicodoid_unicode_is_alphabetic(cp);
    return UNICODOID_OK;
}

unicodoid_status_t unicodoid_is_numeric(
    const unicodoid_system_t* sys,
    unicodoid_char64_t ch,
    bool* out_value
) {
    if (!out_value) return UNICODOID_ERR_INVALID_PARAM;
    uint32_t cp = 0;
    unicodoid_status_t st = native_to_scalar(sys, ch, &cp);
    if (st != UNICODOID_OK) return st;
    *out_value = unicodoid_unicode_is_numeric(cp);
    return UNICODOID_OK;
}

unicodoid_status_t unicodoid_is_whitespace(
    const unicodoid_system_t* sys,
    unicodoid_char64_t ch,
    bool* out_value
) {
    if (!out_value) return UNICODOID_ERR_INVALID_PARAM;
    uint32_t cp = 0;
    unicodoid_status_t st = native_to_scalar(sys, ch, &cp);
    if (st != UNICODOID_OK) return st;
    *out_value = unicodoid_unicode_is_whitespace(cp);
    return UNICODOID_OK;
}

unicodoid_status_t unicodoid_to_uppercase(
    const unicodoid_system_t* sys,
    unicodoid_char64_t ch,
    unicodoid_char64_t* out_ch
) {
    uint32_t cp = 0;
    unicodoid_status_t st = native_to_scalar(sys, ch, &cp);
    if (st != UNICODOID_OK) return st;
    return scalar_to_native(sys, unicodoid_unicode_to_uppercase(cp), out_ch);
}

unicodoid_status_t unicodoid_to_lowercase(
    const unicodoid_system_t* sys,
    unicodoid_char64_t ch,
    unicodoid_char64_t* out_ch
) {
    uint32_t cp = 0;
    unicodoid_status_t st = native_to_scalar(sys, ch, &cp);
    if (st != UNICODOID_OK) return st;
    return scalar_to_native(sys, unicodoid_unicode_to_lowercase(cp), out_ch);
}
