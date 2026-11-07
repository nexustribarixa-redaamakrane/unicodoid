#ifndef UNICODOID_PROPERTIES_H
#define UNICODOID_PROPERTIES_H

#include "unicodoid_system.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file unicodoid_properties.h
 * @brief Unicode character properties.
 *
 * The scalar-level queries operate directly on Unicode scalar values and are
 * used by the Unicode algorithms.  The system-level queries require a Unicode
 * profile and translate native values through to_unicode().
 */

/** General category groups (UAX #44). */
typedef enum unicodoid_category {
    UNICODOID_CAT_OTHER                  = 0,
    UNICODOID_CAT_UPPERCASE_LETTER       = 1,
    UNICODOID_CAT_LOWERCASE_LETTER       = 2,
    UNICODOID_CAT_TITLECASE_LETTER       = 3,
    UNICODOID_CAT_MODIFIER_LETTER        = 4,
    UNICODOID_CAT_OTHER_LETTER           = 5,
    UNICODOID_CAT_DECIMAL_NUMBER         = 6,
    UNICODOID_CAT_LETTER_NUMBER          = 7,
    UNICODOID_CAT_OTHER_NUMBER           = 8,
    UNICODOID_CAT_SPACE_SEPARATOR        = 9,
    UNICODOID_CAT_LINE_SEPARATOR         = 10,
    UNICODOID_CAT_PARAGRAPH_SEPARATOR    = 11,
    UNICODOID_CAT_CONTROL                = 12,
    UNICODOID_CAT_FORMAT                 = 13,
    UNICODOID_CAT_SURROGATE              = 14,
    UNICODOID_CAT_PRIVATE_USE            = 15,
    UNICODOID_CAT_UNASSIGNED             = 16,
    UNICODOID_CAT_PUNCTUATION            = 17,
    UNICODOID_CAT_SYMBOL                 = 18,
    UNICODOID_CAT_MARK                   = 19
} unicodoid_category_t;

/** East Asian Width classes (UAX #11). */
typedef enum unicodoid_ea_width {
    UNICODOID_EA_NEUTRAL     = 0,
    UNICODOID_EA_AMBIGUOUS   = 1,
    UNICODOID_EA_HALFWIDTH   = 2,
    UNICODOID_EA_FULLWIDTH   = 3,
    UNICODOID_EA_NARROW      = 4,
    UNICODOID_EA_WIDE        = 5
} unicodoid_ea_width_t;

/* ---- Unicode scalar utilities ------------------------------------------ */

unicodoid_category_t unicodoid_unicode_general_category(uint32_t cp);
unicodoid_ea_width_t unicodoid_unicode_ea_width(uint32_t cp);
bool unicodoid_unicode_is_alphabetic(uint32_t cp);
bool unicodoid_unicode_is_numeric(uint32_t cp);
bool unicodoid_unicode_is_whitespace(uint32_t cp);
bool unicodoid_unicode_is_control(uint32_t cp);
bool unicodoid_unicode_is_punctuation(uint32_t cp);
bool unicodoid_unicode_is_mark(uint32_t cp);
uint32_t unicodoid_unicode_to_uppercase(uint32_t cp);
uint32_t unicodoid_unicode_to_lowercase(uint32_t cp);
int unicodoid_unicode_combining_class(uint32_t cp);

/* ---- System-aware queries (require a Unicode profile) ------------------ */

unicodoid_status_t unicodoid_get_general_category(
    const unicodoid_system_t* sys,
    unicodoid_char64_t ch,
    unicodoid_category_t* out_category
);

unicodoid_status_t unicodoid_get_ea_width(
    const unicodoid_system_t* sys,
    unicodoid_char64_t ch,
    unicodoid_ea_width_t* out_width
);

unicodoid_status_t unicodoid_is_alphabetic(
    const unicodoid_system_t* sys,
    unicodoid_char64_t ch,
    bool* out_value
);

unicodoid_status_t unicodoid_is_numeric(
    const unicodoid_system_t* sys,
    unicodoid_char64_t ch,
    bool* out_value
);

unicodoid_status_t unicodoid_is_whitespace(
    const unicodoid_system_t* sys,
    unicodoid_char64_t ch,
    bool* out_value
);

unicodoid_status_t unicodoid_to_uppercase(
    const unicodoid_system_t* sys,
    unicodoid_char64_t ch,
    unicodoid_char64_t* out_ch
);

unicodoid_status_t unicodoid_to_lowercase(
    const unicodoid_system_t* sys,
    unicodoid_char64_t ch,
    unicodoid_char64_t* out_ch
);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_PROPERTIES_H */
