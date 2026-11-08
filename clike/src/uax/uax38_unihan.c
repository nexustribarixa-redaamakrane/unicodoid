#include "unicodoid/uax/uax38_unihan.h"
#include "unicodoid/unicodoid_ucd.h"

bool unicodoid_uax38_is_unified_ideograph(uint32_t cp) {
    return unicodoid_ucd_has_binary_property(cp, UNICODOID_BP_UNIFIED_IDEOGRAPH);
}

bool unicodoid_uax38_is_radical(uint32_t cp) {
    if (unicodoid_ucd_has_binary_property(cp, UNICODOID_BP_RADICAL)) return true;
    return (cp >= 0x2F00 && cp <= 0x2FDF) || (cp >= 0x2E80 && cp <= 0x2FFF);
}

bool unicodoid_uax38_is_cjk_block(uint32_t cp) {
    if (cp >= 0x3000 && cp <= 0x303F) return true;   /* CJK Symbols and Punctuation */
    if (cp >= 0x3040 && cp <= 0x309F) return true;   /* Hiragana */
    if (cp >= 0x30A0 && cp <= 0x30FF) return true;   /* Katakana */
    if (cp >= 0x3100 && cp <= 0x312F) return true;   /* Bopomofo */
    if (cp >= 0x3130 && cp <= 0x318F) return true;   /* Hangul Compatibility Jamo */
    if (cp >= 0x3190 && cp <= 0x31FF) return true;   /* Kanbun / Bopomofo Ext / CJK Strokes / Katakana Ext */
    if (cp >= 0x3200 && cp <= 0x32FF) return true;   /* Enclosed CJK */
    if (cp >= 0x3300 && cp <= 0x33FF) return true;   /* CJK Compatibility */
    if (cp >= 0x3400 && cp <= 0x4DBF) return true;   /* CJK Ext A */
    if (cp >= 0x4E00 && cp <= 0x9FFF) return true;   /* CJK Unified Ideographs */
    if (cp >= 0xA000 && cp <= 0xA4CF) return true;   /* Yi */
    if (cp >= 0xAC00 && cp <= 0xD7AF) return true;   /* Hangul Syllables */
    if (cp >= 0xF900 && cp <= 0xFAFF) return true;   /* CJK Compatibility Ideographs */
    if (cp >= 0xFE30 && cp <= 0xFE4F) return true;   /* CJK Compatibility Forms */
    if (cp >= 0xFF00 && cp <= 0xFF60) return true;   /* Fullwidth Forms */
    if (cp >= 0x20000 && cp <= 0x2A6DF) return true; /* CJK Ext B */
    if (cp >= 0x2A700 && cp <= 0x2CEAF) return true; /* CJK Ext C..F */
    if (cp >= 0x2CEB0 && cp <= 0x2EBEF) return true; /* CJK Ext E.. */
    if (cp >= 0x30000 && cp <= 0x323AF) return true; /* CJK Ext G.. */
    return false;
}

int unicodoid_uax38_radical_index(uint32_t cp) {
    /* CJKRadicals.txt numbering: U+2F00 is radical 1. */
    if (cp >= 0x2F00 && cp <= 0x2FDF) return (int)(cp - 0x2F00) + 1;
    if (!unicodoid_uax38_is_radical(cp)) return 0;
    return 1;
}

bool unicodoid_uax38_unihan_field(
    uint32_t cp, const char* field, char* out, size_t cap, size_t* out_len) {
    (void)cp;
    (void)field;
    if (out_len) *out_len = 0;
    (void)out;
    (void)cap;
    return false; /* k*/ /* u* */ /* store is not in the generated tables. */
}
