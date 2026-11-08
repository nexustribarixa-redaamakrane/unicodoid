#include "unicodoid/uax/uax44_ucd.h"
#include "unicodoid/unicodoid_ucd.h"

#include <string.h>

/* Alias tables derived from the official Unicode 18.0.0 files
 * (data/ucd/18.0.0/ucd/PropertyAliases.txt and PropertyValueAliases.txt).
 * Short aliases are the ones embedders use in data-file headers. */
typedef struct { const char* alias; const char* name; } alias_t;
typedef struct { const char* property; const char* value; const char* name; } value_alias_t;

static const alias_t k_property_aliases[] = {
    {"gc", "General_Category"}, {"bc", "Bidi_Class"}, {"ea", "East_Asian_Width"},
    {"lb", "Line_Break"}, {"sc", "Script"}, {"ccc", "Canonical_Combining_Class"},
    {"VO", "Vertical_Orientation"}, {"SB", "Sentence_Break"}, {"WB", "Word_Break"},
    {"GCB", "Grapheme_Cluster_Break"}, {"InCB", "Indic_Conjunct_Break"},
    {"jt", "Joining_Type"}, {"jg", "Joining_Group"}, {"Alpha", "Alphabetic"},
    {"Lower", "Lowercase"}, {"Upper", "Uppercase"}, {"Cased", "Cased"},
    {"CI", "Case_Ignorable"}, {"ID_Start", "ID_Start"}, {"ID_Continue", "ID_Continue"},
    {"XID_Start", "XID_Start"}, {"XID_Continue", "XID_Continue"},
};

static const value_alias_t k_value_aliases[] = {
    {"General_Category", "Lu", "Uppercase_Letter"},
    {"General_Category", "Ll", "Lowercase_Letter"},
    {"Bidi_Class", "L", "Left_To_Right"},
    {"Bidi_Class", "R", "Right_To_Left"},
    {"Bidi_Class", "ON", "Other_Neutral"},
    {"Bidi_Class", "EN", "European_Number"},
    {"East_Asian_Width", "W", "Wide"},
    {"East_Asian_Width", "Na", "Narrow"},
    {"East_Asian_Width", "N", "Neutral"},
    {"Line_Break", "AL", "Alphabetic"},
    {"Line_Break", "ID", "Ideographic"},
    {"Line_Break", "BA", "Break_After"},
    {"Line_Break", "SP", "Space"},
};

const char* unicodoid_uax44_version(void) { return UNICODOID_UCD_VERSION; }

const char* unicodoid_uax44_property_alias(const char* alias) {
    if (!alias) return NULL;
    for (size_t i = 0; i < sizeof(k_property_aliases)/sizeof(k_property_aliases[0]); ++i)
        if (strcmp(k_property_aliases[i].alias, alias) == 0) return k_property_aliases[i].name;
    return NULL;
}

const char* unicodoid_uax44_value_alias(const char* property, const char* short_value) {
    if (!property || !short_value) return NULL;
    for (size_t i = 0; i < sizeof(k_value_aliases)/sizeof(k_value_aliases[0]); ++i)
        if (strcmp(k_value_aliases[i].property, property) == 0 &&
            strcmp(k_value_aliases[i].value, short_value) == 0)
            return k_value_aliases[i].name;
    return NULL;
}

bool unicodoid_uax44_has_casefold(uint32_t cp) {
    uint32_t tmp[4];
    size_t n = unicodoid_ucd_to_casefold(cp, tmp, 4);
    return n > 0 && (n != 1 || tmp[0] != cp);
}

const char* unicodoid_uax44_block_name(uint32_t cp) {
    return unicodoid_ucd_block_name(unicodoid_ucd_block(cp));
}
