#ifndef UNICODOID_UNICODE_H
#define UNICODOID_UNICODE_H

#include "unicodoid_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file unicodoid_unicode.h
 * @brief Optional Unicode compatibility layer.
 *
 * Unicode support is not a prerequisite for an encoding system.  A system may
 * declare UNICODOID_COMPAT_NONE and never expose a profile.  Systems that do
 * expose a profile can participate in the opt-in Unicode algorithms.
 */

/** Official Unicode publication categories. */
typedef enum unicodoid_unicode_doc_category {
    UNICODOID_DOC_UAX = 1, /**< Unicode Standard Annex (integral to the standard) */
    UNICODOID_DOC_UTS = 2, /**< Unicode Technical Standard */
    UNICODOID_DOC_UTR = 3, /**< Unicode Technical Report */
    UNICODOID_DOC_UTN = 4  /**< Unicode Technical Note */
} unicodoid_unicode_doc_category_t;

/** Bitmask of Unicode Standard Annexes understood by this SDK. */
typedef enum unicodoid_uax_flags {
    UNICODOID_UAX_NONE               = 0,
    UNICODOID_UAX_9_BIDI             = (1u << 0), /**< UAX #9  Bidirectional Algorithm */
    UNICODOID_UAX_11_EAST_ASIAN_W    = (1u << 1), /**< UAX #11 East Asian Width */
    UNICODOID_UAX_14_LINEBREAK       = (1u << 2), /**< UAX #14 Line Breaking Algorithm */
    UNICODOID_UAX_15_NORMALIZATION   = (1u << 3), /**< UAX #15 Normalization Forms */
    UNICODOID_UAX_24_SCRIPT_PROP     = (1u << 4), /**< UAX #24 Script Property */
    UNICODOID_UAX_29_TEXT_SEGMENT    = (1u << 5), /**< UAX #29 Text Segmentation */
    UNICODOID_UAX_31_ID_SYNTAX       = (1u << 6), /**< UAX #31 Unicode Identifiers and Syntax */
    UNICODOID_UAX_44_UCD             = (1u << 7), /**< UAX #44 Unicode Character Database */
    UNICODOID_UAX_34_NAMED_SEQS      = (1u << 8), /**< UAX #34 Unicode Named Character Sequences */
    UNICODOID_UAX_38_UNIHAN          = (1u << 9), /**< UAX #38 Unicode Han Database (Unihan) */
    UNICODOID_UAX_41_REFERENCES      = (1u << 10),/**< UAX #41 Common References for Unicode Standard Annexes */
    UNICODOID_UAX_42_UCD_XML         = (1u << 11),/**< UAX #42 Unicode Character Database in XML */
    UNICODOID_UAX_45_U_SOURCE        = (1u << 12),/**< UAX #45 U-source Ideographs */
    UNICODOID_UAX_50_VERTICAL_TEXT   = (1u << 13),/**< UAX #50 Unicode Vertical Text Layout */
    UNICODOID_UAX_53_ARABIC_MARK     = (1u << 14),/**< UAX #53 Unicode Arabic Mark Rendering */
    UNICODOID_UAX_57_UNIKEMET        = (1u << 15),/**< UAX #57 Unicode Egyptian Hieroglyph Database (Unikemet) */
    UNICODOID_UAX_60_EAST_ASIAN_DATA = (1u << 16) /**< UAX #60 Data for East Asian Scripts */
} unicodoid_uax_flags_t;

/** Bitmask of Unicode Technical Standards understood by this SDK. */
typedef enum unicodoid_uts_flags {
    UNICODOID_UTS_NONE               = 0,
    UNICODOID_UTS_10_UCA             = (1u << 0), /**< UTS #10 Unicode Collation Algorithm */
    UNICODOID_UTS_18_REGEX           = (1u << 1), /**< UTS #18 Unicode Regular Expressions */
    UNICODOID_UTS_35_LDML            = (1u << 2), /**< UTS #35 Unicode Common Locale Data & Rules (CLDR) Part 1: Core */
    UNICODOID_UTS_39_SECURITY        = (1u << 3), /**< UTS #39 Unicode Security Mechanisms */
    UNICODOID_UTS_46_IDNA            = (1u << 4), /**< UTS #46 Unicode IDNA Compatibility Processing */
    UNICODOID_UTS_51_EMOJI           = (1u << 5), /**< UTS #51 Unicode Emoji */
    UNICODOID_UTS_37_IVD             = (1u << 6), /**< UTS #37 Unicode Ideographic Variation Database */
    UNICODOID_UTS_55_SOURCE_CODE     = (1u << 7), /**< UTS #55 Unicode Source Code Handling */
    UNICODOID_UTS_58_LINKS           = (1u << 8), /**< UTS #58 Unicode Link Detection and Formatting */
    UNICODOID_UTS_61_SET_NOTATION    = (1u << 9)  /**< UTS #61 Unicode Set Notation (draft) */
} unicodoid_uts_flags_t;

/** Bitmask of Unicode Technical Reports referenced by this SDK. */
typedef enum unicodoid_utr_flags {
    UNICODOID_UTR_NONE               = 0,
    UNICODOID_UTR_17_ENCODING_MODEL  = (1u << 0), /**< UTR #17 Unicode Character Encoding Model */
    UNICODOID_UTR_23_PROP_MODEL      = (1u << 1), /**< UTR #23 The Unicode Character Property Model */
    UNICODOID_UTR_25_MATH            = (1u << 2), /**< UTR #25 Unicode Support for Mathematics */
    UNICODOID_UTR_33_CONFORMANCE     = (1u << 3), /**< UTR #33 Unicode Conformance Model */
    UNICODOID_UTR_54_MONGOLIAN       = (1u << 4), /**< UTR #54 Unicode Mongolian 12.1 Baseline */
    UNICODOID_UTR_56_CUNEIFORM       = (1u << 5), /**< UTR #56 Unicode Cuneiform Sign Lists */
    UNICODOID_UTR_59_EAST_ASIAN_SP   = (1u << 6)  /**< UTR #59 East Asian Spacing (proposed draft) */
} unicodoid_utr_flags_t;

/** Metadata for one Unicode specification. */
typedef struct unicodoid_unicode_spec_ref {
    unicodoid_unicode_doc_category_t category;
    uint32_t                         number;  /**< Document number, e.g. 15 for UAX #15 */
    const char*                      title;   /**< Official document title */
    const char*                      url;     /**< Official report URL */
    const char*                      status;  /**< Publication status */
} unicodoid_unicode_spec_ref_t;

/* Forward declaration for the self-referential profile callbacks. */
typedef struct unicodoid_unicode_profile unicodoid_unicode_profile_t;

/**
 * @brief Optional Unicode compatibility profile attached to an encoding system.
 *
 * The profile owns the mapping between native character values and Unicode
 * scalar values.  The mapping callbacks receive the profile pointer so that a
 * profile can carry implementation context in @ref context.
 */
struct unicodoid_unicode_profile {
    uint32_t unicode_major;
    uint32_t unicode_minor;
    uint32_t unicode_patch;

    uint32_t supported_uax; /**< Bitwise OR of unicodoid_uax_flags_t */
    uint32_t supported_uts; /**< Bitwise OR of unicodoid_uts_flags_t */
    uint32_t supported_utr; /**< Bitwise OR of unicodoid_utr_flags_t */

    void* context; /**< Optional implementation-defined context */

    /**
     * @brief Map a native character value to a Unicode scalar value.
     *
     * @return UNICODOID_OK on success, UNICODOID_ERR_UNMAPPABLE_CHAR when the
     *         native value has no Unicode equivalent.
     */
    unicodoid_status_t (*to_unicode)(
        const unicodoid_unicode_profile_t* profile,
        unicodoid_char64_t native_val,
        uint32_t* out_unicode
    );

    /**
     * @brief Map a Unicode scalar value to a native character value.
     */
    unicodoid_status_t (*from_unicode)(
        const unicodoid_unicode_profile_t* profile,
        uint32_t unicode_val,
        unicodoid_char64_t* out_native
    );
};

/**
 * @brief Retrieve specification details for a documented Unicode standard.
 */
unicodoid_status_t unicodoid_get_unicode_spec_ref(
    unicodoid_unicode_doc_category_t category,
    uint32_t number,
    unicodoid_unicode_spec_ref_t* out_ref
);

/**
 * @brief Number of entries in the built-in Unicode specification catalog.
 */
size_t unicodoid_unicode_spec_count(void);

/**
 * @brief Retrieve a catalog entry by index.
 *
 * @return UNICODOID_OK, or UNICODOID_ERR_NOT_FOUND if index is out of range.
 */
unicodoid_status_t unicodoid_unicode_spec_at(
    size_t index,
    unicodoid_unicode_spec_ref_t* out_ref
);

#ifdef __cplusplus
}
#endif

#endif /* UNICODOID_UNICODE_H */
