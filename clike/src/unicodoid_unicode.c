#include "unicodoid/unicodoid_unicode.h"

/*
 * Built-in catalog of the current Unicode publications relevant to this SDK,
 * plus the one Unicode Technical Note it references.  Titles, URLs and
 * statuses are taken verbatim from docs/unicode18/inventory.json (generated
 * from the official Unicode reports index and report pages) and the official
 * Unicode Technical Notes index.  Reports that are stabilized, superseded,
 * withdrawn, suspended or otherwise not current are intentionally not
 * listed; their dispositions are documented under docs/unicode18/.
 */
static const unicodoid_unicode_spec_ref_t s_specs[] = {
    /* UAX: Unicode Standard Annexes (current in Unicode 18.0.0) */
    { UNICODOID_DOC_UAX, 9,  "Unicode Bidirectional Algorithm", "https://www.unicode.org/reports/tr9/tr9-52.html", "Current" },
    { UNICODOID_DOC_UAX, 11, "East Asian Width", "https://www.unicode.org/reports/tr11/tr11-46.html", "Current" },
    { UNICODOID_DOC_UAX, 14, "Unicode Line Breaking Algorithm", "https://www.unicode.org/reports/tr14/tr14-57.html", "Current" },
    { UNICODOID_DOC_UAX, 15, "Unicode Normalization Forms", "https://www.unicode.org/reports/tr15/tr15-58.html", "Current" },
    { UNICODOID_DOC_UAX, 24, "Unicode Script Property", "https://www.unicode.org/reports/tr24/tr24-41.html", "Current" },
    { UNICODOID_DOC_UAX, 29, "Unicode Text Segmentation", "https://www.unicode.org/reports/tr29/tr29-49.html", "Current" },
    { UNICODOID_DOC_UAX, 31, "Unicode Identifiers and Syntax", "https://www.unicode.org/reports/tr31/tr31-45.html", "Current" },
    { UNICODOID_DOC_UAX, 34, "Unicode Named Character Sequences", "https://www.unicode.org/reports/tr34/tr34-33.html", "Current" },
    { UNICODOID_DOC_UAX, 38, "Unicode Han Database (Unihan)", "https://www.unicode.org/reports/tr38/tr38-41.html", "Current" },
    { UNICODOID_DOC_UAX, 41, "Common References for UAXs", "https://www.unicode.org/reports/tr41/tr41-38.html", "Current" },
    { UNICODOID_DOC_UAX, 42, "Unicode Character Database in XML", "https://www.unicode.org/reports/tr42/tr42-40.html", "Current" },
    { UNICODOID_DOC_UAX, 44, "Unicode Character Database", "https://www.unicode.org/reports/tr44/tr44-38.html", "Current" },
    { UNICODOID_DOC_UAX, 45, "U-source Ideographs", "https://www.unicode.org/reports/tr45/tr45-34.html", "Current" },
    { UNICODOID_DOC_UAX, 50, "Unicode Vertical Text Layout", "https://www.unicode.org/reports/tr50/tr50-35.html", "Current" },
    { UNICODOID_DOC_UAX, 53, "Unicode Arabic Mark Rendering", "https://www.unicode.org/reports/tr53/tr53-13.html", "Current" },
    { UNICODOID_DOC_UAX, 57, "Unicode Egyptian Hieroglyph Database", "https://www.unicode.org/reports/tr57/tr57-7.html", "Current" },
    { UNICODOID_DOC_UAX, 60, "Data for East Asian Scripts", "https://www.unicode.org/reports/tr60/tr60-3.html", "Current" },

    /* UTS: Unicode Technical Standards (current in Unicode 18.0.0) */
    { UNICODOID_DOC_UTS, 10, "Unicode Collation Algorithm", "https://www.unicode.org/reports/tr10/tr10-55.html", "Current" },
    { UNICODOID_DOC_UTS, 18, "Unicode Regular Expressions", "https://www.unicode.org/reports/tr18/tr18-25.html", "Current" },
    { UNICODOID_DOC_UTS, 35, "Unicode Common Locale Data & Rules (CLDR) Part 1: Core", "https://www.unicode.org/reports/tr35/", "Current" },
    { UNICODOID_DOC_UTS, 37, "Unicode Ideographic Variation Database", "https://www.unicode.org/reports/tr37/tr37-16.html", "Current" },
    { UNICODOID_DOC_UTS, 39, "Unicode Security Mechanisms", "https://www.unicode.org/reports/tr39/tr39-34.html", "Current" },
    { UNICODOID_DOC_UTS, 46, "Unicode IDNA Compatibility Processing", "https://www.unicode.org/reports/tr46/tr46-36.html", "Current" },
    { UNICODOID_DOC_UTS, 51, "Unicode Emoji", "https://www.unicode.org/reports/tr51/tr51-31.html", "Current" },
    { UNICODOID_DOC_UTS, 55, "Unicode Source Code Handling", "https://www.unicode.org/reports/tr55/tr55-5.html", "Current" },
    { UNICODOID_DOC_UTS, 58, "Unicode Link Detection and Formatting: URLs and Email Addresses", "https://www.unicode.org/reports/tr58/tr58-3.html", "Current" },
    { UNICODOID_DOC_UTS, 61, "Unicode Set Notation", "https://www.unicode.org/reports/tr61/tr61-2.html", "Draft" },

    /* UTR: Unicode Technical Reports (current in Unicode 18.0.0) */
    { UNICODOID_DOC_UTR, 17, "Unicode Character Encoding Model", "https://www.unicode.org/reports/tr17/tr17-9.html", "Current" },
    { UNICODOID_DOC_UTR, 23, "The Unicode Character Property Model", "https://www.unicode.org/reports/tr23/tr23-15.html", "Current" },
    { UNICODOID_DOC_UTR, 25, "Unicode Support for Mathematics", "https://www.unicode.org/reports/tr25/tr25-16.html", "Proposed Update" },
    { UNICODOID_DOC_UTR, 33, "Conformance Model", "https://www.unicode.org/reports/tr33/tr33-7.html", "Current" },
    { UNICODOID_DOC_UTR, 54, "Unicode Mongolian 12.1 Baseline", "https://www.unicode.org/reports/tr54/tr54-1.html", "Current" },
    { UNICODOID_DOC_UTR, 56, "Unicode Cuneiform Sign Lists", "https://www.unicode.org/reports/tr56/tr56-5.html", "Current" },
    { UNICODOID_DOC_UTR, 59, "East Asian Spacing", "https://www.unicode.org/reports/tr59/tr59-1.html", "Current" },

    /* UTN: the one Unicode Technical Note referenced by this SDK (docs only).
     * UTNs are independent author works, not specifications; see docs/unicode18/utn/. */
    { UNICODOID_DOC_UTN, 1,  "Issues in Indic Language Collation", "https://www.unicode.org/notes/tn1/", "Technical Note" }
};

static const size_t s_spec_count = sizeof(s_specs) / sizeof(s_specs[0]);

unicodoid_status_t unicodoid_get_unicode_spec_ref(
    unicodoid_unicode_doc_category_t category,
    uint32_t number,
    unicodoid_unicode_spec_ref_t* out_ref
) {
    if (!out_ref) return UNICODOID_ERR_INVALID_PARAM;

    for (size_t i = 0; i < s_spec_count; ++i) {
        if (s_specs[i].category == category && s_specs[i].number == number) {
            *out_ref = s_specs[i];
            return UNICODOID_OK;
        }
    }
    return UNICODOID_ERR_NOT_FOUND;
}

size_t unicodoid_unicode_spec_count(void) {
    return s_spec_count;
}

unicodoid_status_t unicodoid_unicode_spec_at(
    size_t index,
    unicodoid_unicode_spec_ref_t* out_ref
) {
    if (!out_ref) return UNICODOID_ERR_INVALID_PARAM;
    if (index >= s_spec_count) return UNICODOID_ERR_NOT_FOUND;
    *out_ref = s_specs[index];
    return UNICODOID_OK;
}
