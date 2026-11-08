#include "unicodoid/uts/uts35_cldr/cldr_core.h"

#include <string.h>

static bool is_lower_alpha(const char* s, size_t n) {
    for (size_t i = 0; i < n; ++i) if (s[i] < 'a' || s[i] > 'z') return false;
    return true;
}

static bool is_alpha(const char* s, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        char c = s[i];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))) return false;
    }
    return true;
}

static bool is_alnum(const char* s, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        char c = s[i];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))) return false;
    }
    return true;
}

bool unicodoid_cldr_parse_locale(const char* tag, unicodoid_cldr_locale_t* out) {
    if (!tag || !out) return false;
    memset(out, 0, sizeof(*out));
    strcpy(out->language, "und");
    char buf[64];
    if (strlen(tag) >= sizeof(buf)) return false;
    strcpy(buf, tag);
    /* Split by '-'. */
    char* parts[6];
    int np = 0;
    char* tok = strtok(buf, "-");
    while (tok && np < 6) { parts[np++] = tok; tok = strtok(NULL, "-"); }
    if (np < 1) return false;
    if (strlen(parts[0]) < 2 || strlen(parts[0]) > 3 || !is_lower_alpha(parts[0], strlen(parts[0])))
        return false;
    strcpy(out->language, parts[0]);
    int idx = 1;
    if (idx < np && strlen(parts[idx]) == 4 && is_alpha(parts[idx], 4)) {
        strcpy(out->script, parts[idx]); ++idx;
    }
    if (idx < np && ((strlen(parts[idx]) == 2 && is_alpha(parts[idx], 2)) ||
                     (strlen(parts[idx]) == 3 && is_alnum(parts[idx], 3)))) {
        int i = 0;
        while (parts[idx][i]) { out->region[i] = parts[idx][i]; ++i; }
        out->region[i] = '\0';
        ++idx;
    }
    if (idx < np) {
        if (strlen(parts[idx]) > 8 || !is_alnum(parts[idx], strlen(parts[idx]))) return false;
        strcpy(out->variant, parts[idx]);
    }
    return true;
}

bool unicodoid_cldr_is_valid_locale(const char* tag) {
    unicodoid_cldr_locale_t loc;
    return unicodoid_cldr_parse_locale(tag, &loc);
}

const char* unicodoid_cldr_default_locale(const char* language) {
    /* CLDR fallback: prefer language+region defaults for a few majors. */
    if (language && strcmp(language, "en") == 0) return "en_US";
    if (language && strcmp(language, "de") == 0) return "de_DE";
    if (language && strcmp(language, "zh") == 0) return "zh_Hans_CN";
    return "root";
}

const char* unicodoid_cldr_version(void) { return "49"; }
