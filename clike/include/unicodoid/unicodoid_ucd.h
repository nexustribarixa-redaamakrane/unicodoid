#ifndef UNICODOID_UCD_H
#define UNICODOID_UCD_H

#include "unicodoid_ucd_data.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file unicodoid_ucd.h
 * @brief Unicode 18.0.0 Character Database access.
 *
 * This is the generated-data layer used by the UAX/UTS/UTR modules.  All
 * tables are derived from the official Unicode 18.0.0 data files.  The
 * generated source records the Unicode version, generator version and a
 * SHA-256 digest of the source data.
 */

/** Detailed General_Category values (UAX #44). */
typedef enum unicodoid_gc {
    UNICODOID_GC_CN = 0, UNICODOID_GC_CC, UNICODOID_GC_CF, UNICODOID_GC_CO,
    UNICODOID_GC_CS, UNICODOID_GC_LL, UNICODOID_GC_LM, UNICODOID_GC_LO,
    UNICODOID_GC_LT, UNICODOID_GC_LU, UNICODOID_GC_MC, UNICODOID_GC_ME,
    UNICODOID_GC_MN, UNICODOID_GC_ND, UNICODOID_GC_NL, UNICODOID_GC_NO,
    UNICODOID_GC_PC, UNICODOID_GC_PD, UNICODOID_GC_PE, UNICODOID_GC_PF,
    UNICODOID_GC_PI, UNICODOID_GC_PO, UNICODOID_GC_PS, UNICODOID_GC_SC,
    UNICODOID_GC_SK, UNICODOID_GC_SM, UNICODOID_GC_SO, UNICODOID_GC_ZL,
    UNICODOID_GC_ZP, UNICODOID_GC_ZS
} unicodoid_gc_t;

/** Bidirectional_Class values (UAX #9). */
typedef enum unicodoid_bidi_class {
    UNICODOID_BIDI_L = 0, UNICODOID_BIDI_R, UNICODOID_BIDI_EN,
    UNICODOID_BIDI_ES, UNICODOID_BIDI_ET, UNICODOID_BIDI_AN,
    UNICODOID_BIDI_CS, UNICODOID_BIDI_B, UNICODOID_BIDI_S,
    UNICODOID_BIDI_WS, UNICODOID_BIDI_ON, UNICODOID_BIDI_LRE,
    UNICODOID_BIDI_LRO, UNICODOID_BIDI_AL, UNICODOID_BIDI_RLE,
    UNICODOID_BIDI_RLO, UNICODOID_BIDI_PDF, UNICODOID_BIDI_NSM,
    UNICODOID_BIDI_BN, UNICODOID_BIDI_FSI, UNICODOID_BIDI_LRI,
    UNICODOID_BIDI_RLI, UNICODOID_BIDI_PDI
} unicodoid_bidi_class_t;

/** Grapheme_Cluster_Break values (UAX #29). */
typedef enum unicodoid_gcb {
    UNICODOID_GCB_OTHER = 0, UNICODOID_GCB_CR, UNICODOID_GCB_LF,
    UNICODOID_GCB_CONTROL, UNICODOID_GCB_EXTEND, UNICODOID_GCB_ZWJ,
    UNICODOID_GCB_RI, UNICODOID_GCB_PREPEND, UNICODOID_GCB_SPACINGMARK,
    UNICODOID_GCB_L, UNICODOID_GCB_V, UNICODOID_GCB_T,
    UNICODOID_GCB_LV, UNICODOID_GCB_LVT
} unicodoid_gcb_t;

/** Word_Break values (UAX #29). */
typedef enum unicodoid_wb {
    UNICODOID_WB_OTHER = 0, UNICODOID_WB_CR, UNICODOID_WB_LF,
    UNICODOID_WB_NEWLINE, UNICODOID_WB_EXTEND, UNICODOID_WB_ZWJ,
    UNICODOID_WB_RI, UNICODOID_WB_FORMAT, UNICODOID_WB_KATAKANA,
    UNICODOID_WB_HEBREW_LETTER, UNICODOID_WB_ALETTER,
    UNICODOID_WB_SINGLE_QUOTE, UNICODOID_WB_DOUBLE_QUOTE,
    UNICODOID_WB_MIDNUMLET, UNICODOID_WB_MIDLETTER,
    UNICODOID_WB_MIDNUM, UNICODOID_WB_NUMERIC,
    UNICODOID_WB_EXTENDNUMLET, UNICODOID_WB_WSEGSPACE
} unicodoid_wb_t;

/** Sentence_Break values (UAX #29). */
typedef enum unicodoid_sb {
    UNICODOID_SB_OTHER = 0, UNICODOID_SB_CR, UNICODOID_SB_LF,
    UNICODOID_SB_EXTEND, UNICODOID_SB_SEP, UNICODOID_SB_FORMAT,
    UNICODOID_SB_SP, UNICODOID_SB_LOWER, UNICODOID_SB_UPPER,
    UNICODOID_SB_OLETTER, UNICODOID_SB_NUMERIC, UNICODOID_SB_ATERM,
    UNICODOID_SB_SCONTINUE, UNICODOID_SB_STERM, UNICODOID_SB_CLOSE
} unicodoid_sb_t;

/** Line_Break values (UAX #14). */
typedef enum unicodoid_lb {
    UNICODOID_LB_XX = 0, UNICODOID_LB_AI, UNICODOID_LB_AK, UNICODOID_LB_AL,
    UNICODOID_LB_AP, UNICODOID_LB_AS, UNICODOID_LB_B2, UNICODOID_LB_BA,
    UNICODOID_LB_BB, UNICODOID_LB_BK, UNICODOID_LB_CB, UNICODOID_LB_CJ,
    UNICODOID_LB_CL, UNICODOID_LB_CM, UNICODOID_LB_CP, UNICODOID_LB_CR,
    UNICODOID_LB_EB, UNICODOID_LB_EM, UNICODOID_LB_EX, UNICODOID_LB_GL,
    UNICODOID_LB_H2, UNICODOID_LB_H3, UNICODOID_LB_HH, UNICODOID_LB_HL,
    UNICODOID_LB_HY, UNICODOID_LB_ID, UNICODOID_LB_IN, UNICODOID_LB_IS,
    UNICODOID_LB_JL, UNICODOID_LB_JT, UNICODOID_LB_JV, UNICODOID_LB_LF,
    UNICODOID_LB_NL, UNICODOID_LB_NS, UNICODOID_LB_NU, UNICODOID_LB_OP,
    UNICODOID_LB_PO, UNICODOID_LB_PR, UNICODOID_LB_QU, UNICODOID_LB_RI,
    UNICODOID_LB_SA, UNICODOID_LB_SG, UNICODOID_LB_SP, UNICODOID_LB_SY,
    UNICODOID_LB_VF, UNICODOID_LB_VI, UNICODOID_LB_WJ, UNICODOID_LB_ZW,
    UNICODOID_LB_ZWJ
} unicodoid_lb_t;

/** Vertical_Orientation values (UAX #50). */
typedef enum unicodoid_vo {
    UNICODOID_VO_U = 0, UNICODOID_VO_R, UNICODOID_VO_TU, UNICODOID_VO_TR
} unicodoid_vo_t;

/** Indic_Conjunct_Break values (UAX #29). */
typedef enum unicodoid_incb {
    UNICODOID_INCB_NONE = 0, UNICODOID_INCB_LINKER,
    UNICODOID_INCB_CONSONANT, UNICODOID_INCB_EXTEND
} unicodoid_incb_t;

/** Joining_Type values (UAX #53 / UAX #9). */
typedef enum unicodoid_joining_type {
    UNICODOID_JT_U = 0, UNICODOID_JT_C, UNICODOID_JT_D,
    UNICODOID_JT_L, UNICODOID_JT_R, UNICODOID_JT_T
} unicodoid_joining_type_t;

/** Binary properties exposed by the generated data layer. */
typedef enum unicodoid_binary_property {
    UNICODOID_BP_ALPHABETIC = 0,
    UNICODOID_BP_LOWERCASE,
    UNICODOID_BP_UPPERCASE,
    UNICODOID_BP_CASED,
    UNICODOID_BP_CASE_IGNORABLE,
    UNICODOID_BP_MATH,
    UNICODOID_BP_ID_START,
    UNICODOID_BP_ID_CONTINUE,
    UNICODOID_BP_XID_START,
    UNICODOID_BP_XID_CONTINUE,
    UNICODOID_BP_DEFAULT_IGNORABLE_CODE_POINT,
    UNICODOID_BP_GRAPHEME_EXTEND,
    UNICODOID_BP_GRAPHEME_BASE,
    UNICODOID_BP_GRAPHEME_LINK,
    UNICODOID_BP_WHITE_SPACE,
    UNICODOID_BP_PATTERN_SYNTAX,
    UNICODOID_BP_PATTERN_WHITE_SPACE,
    UNICODOID_BP_DASH,
    UNICODOID_BP_HYPHEN,
    UNICODOID_BP_QUOTATION_MARK,
    UNICODOID_BP_TERMINAL_PUNCTUATION,
    UNICODOID_BP_DIACRITIC,
    UNICODOID_BP_EXTENDER,
    UNICODOID_BP_SOFT_DOTTED,
    UNICODOID_BP_JOIN_CONTROL,
    UNICODOID_BP_BIDI_CONTROL,
    UNICODOID_BP_NONCHARACTER_CODE_POINT,
    UNICODOID_BP_VARIATION_SELECTOR,
    UNICODOID_BP_EMOJI,
    UNICODOID_BP_EMOJI_PRESENTATION,
    UNICODOID_BP_EMOJI_MODIFIER,
    UNICODOID_BP_EMOJI_MODIFIER_BASE,
    UNICODOID_BP_EMOJI_COMPONENT,
    UNICODOID_BP_EXTENDED_PICTOGRAPHIC,
    UNICODOID_BP_UNIFIED_IDEOGRAPH,
    UNICODOID_BP_RADICAL,
    UNICODOID_BP_DEPRECATED,
    UNICODOID_BP_FULL_COMPOSITION_EXCLUSION,
    UNICODOID_BP_SENTENCE_TERMINAL,
    UNICODOID_BP_REGIONAL_INDICATOR,
    UNICODOID_BP_MODIFIER_COMBINING_MARK
} unicodoid_binary_property_t;

/* ---- Scalar property queries ------------------------------------------ */

uint8_t unicodoid_ucd_general_category(uint32_t cp);
uint8_t unicodoid_ucd_canonical_combining_class(uint32_t cp);
uint8_t unicodoid_ucd_bidi_class(uint32_t cp);
uint8_t unicodoid_ucd_east_asian_width(uint32_t cp);
uint8_t unicodoid_ucd_line_break(uint32_t cp);
uint8_t unicodoid_ucd_grapheme_break(uint32_t cp);
uint8_t unicodoid_ucd_word_break(uint32_t cp);
uint8_t unicodoid_ucd_sentence_break(uint32_t cp);
uint8_t unicodoid_ucd_vertical_orientation(uint32_t cp);
uint8_t unicodoid_ucd_joining_type(uint32_t cp);
uint8_t unicodoid_ucd_indic_conjunct_break(uint32_t cp);
uint16_t unicodoid_ucd_script(uint32_t cp);
uint16_t unicodoid_ucd_block(uint32_t cp);
uint8_t unicodoid_ucd_age(uint32_t cp);
bool unicodoid_ucd_has_binary_property(uint32_t cp, unicodoid_binary_property_t property);

const char* unicodoid_ucd_general_category_name(uint8_t value);
const char* unicodoid_ucd_bidi_class_name(uint8_t value);
const char* unicodoid_ucd_east_asian_width_name(uint8_t value);
const char* unicodoid_ucd_line_break_name(uint8_t value);
const char* unicodoid_ucd_grapheme_break_name(uint8_t value);
const char* unicodoid_ucd_word_break_name(uint8_t value);
const char* unicodoid_ucd_sentence_break_name(uint8_t value);
const char* unicodoid_ucd_vertical_orientation_name(uint8_t value);
const char* unicodoid_ucd_joining_type_name(uint8_t value);
const char* unicodoid_ucd_indic_conjunct_break_name(uint8_t value);
const char* unicodoid_ucd_script_name(uint16_t value);
const char* unicodoid_ucd_block_name(uint16_t value);
const char* unicodoid_ucd_age_name(uint8_t value);

/** Numeric_Value; returns false for characters without a numeric value. */
bool unicodoid_ucd_numeric_value(uint32_t cp, int64_t* numerator, int64_t* denominator);
/** Decimal_Digit_Value, or -1. */
int32_t unicodoid_ucd_decimal_digit(uint32_t cp);

/* ---- Case mappings ----------------------------------------------------- */

/** Full uppercase mapping; writes at most @p cap scalars and returns length. */
size_t unicodoid_ucd_to_uppercase(uint32_t cp, uint32_t* out, size_t cap);
size_t unicodoid_ucd_to_lowercase(uint32_t cp, uint32_t* out, size_t cap);
size_t unicodoid_ucd_to_titlecase(uint32_t cp, uint32_t* out, size_t cap);
size_t unicodoid_ucd_to_casefold(uint32_t cp, uint32_t* out, size_t cap);
uint32_t unicodoid_ucd_simple_uppercase(uint32_t cp);
uint32_t unicodoid_ucd_simple_lowercase(uint32_t cp);
uint32_t unicodoid_ucd_simple_titlecase(uint32_t cp);

/* ---- Normalization data (UAX #15) -------------------------------------- */

/** Raw decomposition mapping.  Returns the number of scalars written. */
size_t unicodoid_ucd_decomposition(uint32_t cp, bool compatibility, uint32_t* out, size_t cap);
bool unicodoid_ucd_has_decomposition(uint32_t cp, bool compatibility);
bool unicodoid_ucd_compose_pair(uint32_t first, uint32_t second, uint32_t* out);

/* ---- Security and IDNA data ------------------------------------------- */

/** Confusable mapping (UTS #39); returns length or 0 when none. */
size_t unicodoid_ucd_confusable(uint32_t cp, uint32_t* out, size_t cap);
/** IDNA mapping status: 0 valid, 1 mapped, 2 deviation, 3 ignored, 4 disallowed. */
int unicodoid_ucd_idna_status(uint32_t cp, uint32_t* out, size_t* out_len);
bool unicodoid_ucd_idna_nv8(uint32_t cp);

/* ---- Collation data (UTS #10) ----------------------------------------- */

typedef struct unicodoid_ucd_weight_value {
    uint16_t primary;
    uint16_t secondary;
    uint16_t tertiary;
    bool     variable;
} unicodoid_ucd_weight_value_t;

/**
 * Look up a collation element.  For a single scalar, returns the number of
 * weights written.  For contractions, use the longest-match helper below.
 */
size_t unicodoid_ucd_collation_weights(
    uint32_t cp, unicodoid_ucd_weight_value_t* out, size_t cap
);
size_t unicodoid_ucd_collation_contraction(
    const uint32_t* cps, size_t length, size_t* matched,
    unicodoid_ucd_weight_value_t* out, size_t cap
);
bool unicodoid_ucd_implicit_weight(
    uint32_t cp, uint16_t* base, uint32_t* offset
);

/* ---- Linkification and spacing data (UTS #58, UTR #59) ------------------- */

/** UTS #58 Link_Term values: 1 Include, 2 Hard, 3 Soft, 4 Close, 5 Open.
 *  Code points not listed have the value 2 (Hard). */
uint8_t unicodoid_ucd_link_term(uint32_t cp);
/** UTS #58 Link_Bracket: maps a Close bracket code point to its opening
 *  counterpart.  Returns false when there is no mapping. */
bool unicodoid_ucd_link_bracket(uint32_t cp, uint32_t* open_cp);
/** UTS #58 Link_Email binary property. */
bool unicodoid_ucd_link_email(uint32_t cp);
/** UTR #59 East_Asian_Spacing values: 1 W, 2 N, 3 O, 4 C.
 *  Code points not listed have the value 3 (O). */
uint8_t unicodoid_ucd_east_asian_spacing(uint32_t cp);

/* ---- Named character sequences and emoji sequences ------------------------ */

/** UAX #34 named sequences: index ranges over the shared pool. */
size_t unicodoid_ucd_named_sequences_count(void);
const char* unicodoid_ucd_named_seq_name(size_t index);
/** Writes at most @p cap scalars; returns the full length (or SIZE_MAX on bad index). */
size_t unicodoid_ucd_named_seq(size_t index, uint32_t* out, size_t cap);
/** Find a named sequence by exact name; returns false when absent. */
bool unicodoid_ucd_named_seq_find(const char* name, size_t* index);

/** UTS #51 emoji sequences: RGI emoji from emoji-sequences.txt and
 *  emoji-zwj-sequences.txt. */
size_t unicodoid_ucd_emoji_sequences_count(void);
const char* unicodoid_ucd_emoji_seq_type(size_t index);
size_t unicodoid_ucd_emoji_seq(size_t index, uint32_t* out, size_t cap);

/* ---- Metadata ---------------------------------------------------------- */

const char* unicodoid_ucd_version(void);
const char* unicodoid_ucd_generator_version(void);
const char* unicodoid_ucd_source_checksum(void);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_UCD_H */
