#include "unicodoid/uts/uts18_regex.h"
#include "unicodoid/unicodoid_ucd.h"

#include <string.h>

/* Minimal UTS #18 \p{...} evaluator over the generated tables.
 * Supported names: General_Category long/short values, Script names,
 * binary properties by UCD name, East_Asian_Width values. */

static bool starts(const char* pat, const char* pre) {
    return strncmp(pat, pre, strlen(pre)) == 0;
}

size_t unicodoid_uts18_property_escape_length(const char* pat) {
    if (!pat) return 0;
    if (!starts(pat, "\\p{") && !starts(pat, "\\P{")) return 0;
    const char* close = strchr(pat, '}');
    if (!close) return 0;
    return (size_t)(close - pat) + 1;
}

bool unicodoid_uts18_is_valid_escape(const char* pat) {
    return unicodoid_uts18_property_escape_length(pat) != 0;
}

static int match_property(const char* name, uint32_t cp) {
    /* Known-property names return their kind when the scalar is a member,
     * NONE when the property is known but the scalar is not a member, and
     * INVALID when the name is not a known property. */
    if (strcmp(name, "Alphabetic") == 0 || strcmp(name, "Alpha") == 0)
        return unicodoid_ucd_has_binary_property(cp, UNICODOID_BP_ALPHABETIC) ? UNICODOID_UTS18_BINARY : UNICODOID_UTS18_NONE;
    if (strcmp(name, "Uppercase") == 0)
        return unicodoid_ucd_has_binary_property(cp, UNICODOID_BP_UPPERCASE) ? UNICODOID_UTS18_BINARY : UNICODOID_UTS18_NONE;
    if (strcmp(name, "Lowercase") == 0)
        return unicodoid_ucd_has_binary_property(cp, UNICODOID_BP_LOWERCASE) ? UNICODOID_UTS18_BINARY : UNICODOID_UTS18_NONE;
    if (strcmp(name, "Emoji") == 0)
        return unicodoid_ucd_has_binary_property(cp, UNICODOID_BP_EMOJI) ? UNICODOID_UTS18_BINARY : UNICODOID_UTS18_NONE;
    /* General categories by long and short names (subset). */
    const char* gc_name = unicodoid_ucd_general_category_name(unicodoid_ucd_general_category(cp));
    if (strcmp(name, gc_name) == 0) return UNICODOID_UTS18_GENERAL_CATEGORY;
    if (strcmp(name, "Letter") == 0) {
        uint8_t gc = unicodoid_ucd_general_category(cp);
        bool is_letter = (gc >= (uint8_t)UNICODOID_GC_LL && gc <= (uint8_t)UNICODOID_GC_LU);
        return is_letter ? UNICODOID_UTS18_GENERAL_CATEGORY : UNICODOID_UTS18_NONE;
    }
    if (strcmp(name, "Uppercase_Letter") == 0 || strcmp(name, "Lu") == 0)
        return unicodoid_ucd_general_category(cp) == (uint8_t)UNICODOID_GC_LU ? UNICODOID_UTS18_GENERAL_CATEGORY : UNICODOID_UTS18_NONE;
    if (strcmp(name, "Lowercase_Letter") == 0 || strcmp(name, "Ll") == 0)
        return unicodoid_ucd_general_category(cp) == (uint8_t)UNICODOID_GC_LL ? UNICODOID_UTS18_GENERAL_CATEGORY : UNICODOID_UTS18_NONE;
    /* General-category short names that are known (for invalid-name detection). */
    if (strlen(name) == 2) {
        /* Any 2-letter name not matching the scalar's own GC is a known-but-
         * non-member if it is a valid GC short name. */
        static const char* k_gc_shorts[] = {
            "Cn","Cc","Cf","Co","Cs","Ll","Lm","Lo","Lt","Lu","Mc","Me","Mn",
            "Nd","Nl","No","Pc","Pd","Pe","Pf","Pi","Po","Ps","Sc","Sk","Sm",
            "So","Zl","Zp","Zs", NULL
        };
        for (int i = 0; k_gc_shorts[i]; ++i) {
            if (strcmp(name, k_gc_shorts[i]) == 0) return UNICODOID_UTS18_NONE;
        }
    }
    /* Script. */
    const char* sc = unicodoid_ucd_script_name(unicodoid_ucd_script(cp));
    if (sc && strcmp(sc, name) == 0) return UNICODOID_UTS18_SCRIPT;
    /* Known script names that do not match. */
    for (size_t i = 0; i < unicodoid_ucd_script_names_count; ++i) {
        if (strcmp(unicodoid_ucd_script_names[i], name) == 0) return UNICODOID_UTS18_NONE;
    }
    /* East Asian Width values. */
    const char* ea = unicodoid_ucd_east_asian_width_name(unicodoid_ucd_east_asian_width(cp));
    if (ea && strcmp(ea, name) == 0) return UNICODOID_UTS18_EAW;
    if (strcmp(name, "Wide") == 0 || strcmp(name, "Narrow") == 0 ||
        strcmp(name, "Neutral") == 0 || strcmp(name, "Ambiguous") == 0 ||
        strcmp(name, "Halfwidth") == 0 || strcmp(name, "Fullwidth") == 0)
        return UNICODOID_UTS18_NONE;
    return UNICODOID_UTS18_INVALID;
}

int unicodoid_uts18_property_match(const char* pat, uint32_t cp) {
    if (!pat) return UNICODOID_UTS18_NONE;
    bool neg = false;
    const char* name_start = NULL;
    if (starts(pat, "\\p{")) name_start = pat + 3;
    else if (starts(pat, "\\P{")) { name_start = pat + 3; neg = true; }
    else return UNICODOID_UTS18_NONE;
    const char* close = strchr(name_start, '}');
    if (!close || close == name_start) return UNICODOID_UTS18_INVALID;
    char name[64];
    size_t n = (size_t)(close - name_start);
    if (n >= sizeof(name)) return UNICODOID_UTS18_INVALID;
    memcpy(name, name_start, n); name[n] = '\0';
    int m = match_property(name, cp);
    if (m == UNICODOID_UTS18_INVALID) return UNICODOID_UTS18_INVALID;
    if (neg) {
        /* Negated escape: member-of-negated-set is reported with BINARY kind
         * (the exact family kind is not retained for the negated form). */
        return (m == UNICODOID_UTS18_NONE) ? UNICODOID_UTS18_BINARY : UNICODOID_UTS18_NONE;
    }
    return m;
}
