// Conformance test for UTS #51 RGI emoji sequence data.
//
// Every entry of the official Unicode 18.0.0 emoji-sequences.txt and
// emoji-zwj-sequences.txt is recognised by the validator, including the
// Basic_Emoji ranges which are expanded to individual code points.

#include "unicodoid/uts/uts51_emoji_sequences.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static size_t failures = 0;
static size_t checks = 0;

static void check(bool condition, const char* what) {
    ++checks;
    if (!condition) {
        ++failures;
        fprintf(stderr, "FAIL: %s\n", what);
    }
}

static void validate_sequence(const uint32_t* cps, size_t n, const char* expected_type) {
    const char* actual_type = NULL;
    bool ok = unicodoid_uts51_is_rgi(cps, n, &actual_type);
    check(ok, "RGI sequence recognised");
    check(ok && actual_type && strcmp(actual_type, expected_type) == 0,
          "RGI sequence type matches data file");
    if (!unicodoid_uts51_is_well_formed(cps, n)) {
        ++failures;
        fprintf(stderr, "FAIL: RGI sequence is well formed: ");
        for (size_t i = 0; i < n; ++i) fprintf(stderr, "%04X ", cps[i]);
        fprintf(stderr, "(type %s)\n", expected_type);
    }
    ++checks;
}

static size_t validate_file(const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "FAIL: cannot open %s\n", path);
        ++failures;
        return 0;
    }

    size_t sequences = 0;
    char line[2048];
    while (fgets(line, sizeof line, f)) {
        if (line[0] == '#' || line[0] == '@' || line[0] == '\n' || line[0] == '\r') {
            continue;
        }
        char* semi = strchr(line, ';');
        if (!semi) continue;
        *semi = '\0';
        char* type_field = semi + 1;
        while (*type_field == ' ' || *type_field == '\t') ++type_field;
        char* type_end = type_field;
        while (*type_end && *type_end != ' ' && *type_end != '\t' &&
               *type_end != '\n' && *type_end != '\r' && *type_end != ';') {
            ++type_end;
        }
        *type_end = '\0';

        /* A lone range (only Basic_Emoji uses this form) denotes one RGI
         * sequence per code point. */
        const char* p = line;
        while (*p == ' ' || *p == '\t') ++p;
        char* end = NULL;
        unsigned long first = strtoul(p, &end, 16);
        if (end[0] == '.' && end[1] == '.') {
            unsigned long last = strtoul(end + 2, NULL, 16);
            for (unsigned long cp = first; cp <= last; ++cp) {
                uint32_t seq[1] = { (uint32_t)cp };
                validate_sequence(seq, 1, type_field);
                ++sequences;
            }
            continue;
        }

        uint32_t cps[64];
        size_t n = 0;
        p = line;
        while (*p) {
            while (*p == ' ' || *p == '\t') ++p;
            if (!*p || *p == '\n' || *p == '\r') break;
            unsigned long cp = strtoul(p, &end, 16);
            if (n < 64) cps[n++] = (uint32_t)cp;
            p = end;
        }
        if (n == 0) continue;
        validate_sequence(cps, n, type_field);
        ++sequences;
    }
    fclose(f);
    return sequences;
}

int main(void) {
    char path[1024];
    snprintf(path, sizeof path, "%s/emoji-sequences.txt",
             UNICODOID_EMOJI_DIR);
    size_t a = validate_file(path);
    snprintf(path, sizeof path, "%s/emoji-zwj-sequences.txt",
             UNICODOID_EMOJI_DIR);
    size_t b = validate_file(path);

    check(unicodoid_uts51_count() == a + b, "count matches data files");

    /* Keycap sequence: '1' FE0F 20E3. */
    uint32_t keycap[] = { 0x31, 0xFE0F, 0x20E3 };
    const char* type = NULL;
    check(unicodoid_uts51_is_rgi(keycap, 3, &type), "keycap is RGI");
    check(type && strcmp(type, "Emoji_Keycap_Sequence") == 0,
          "keycap type");
    check(unicodoid_uts51_is_well_formed(keycap, 3), "keycap well formed");

    /* Flag sequence: two regional indicators. */
    uint32_t flag[] = { 0x1F1EB, 0x1F1F7 };
    check(unicodoid_uts51_is_rgi(flag, 2, &type), "flag is RGI");
    check(type && strcmp(type, "RGI_Emoji_Flag_Sequence") == 0, "flag type");

    /* Family ZWJ sequence. */
    uint32_t family[] = { 0x1F468, 0x200D, 0x1F469, 0x200D, 0x1F467 };
    check(unicodoid_uts51_is_rgi(family, 5, &type), "ZWJ family is RGI");
    check(type && strcmp(type, "RGI_Emoji_ZWJ_Sequence") == 0, "ZWJ type");

    /* Negative cases. */
    uint32_t bogus[] = { 0x41, 0x200D, 0x42 };
    check(!unicodoid_uts51_is_rgi(bogus, 3, NULL), "non-emoji ZWJ rejected");
    check(!unicodoid_uts51_is_well_formed(bogus, 3),
          "non-emoji ZWJ not well formed");
    uint32_t surrogate[] = { 0xD83D, 0xDE00 };
    check(!unicodoid_uts51_is_well_formed(surrogate, 2),
          "surrogate rejected");
    check(!unicodoid_uts51_is_well_formed(NULL, 0), "empty rejected");

    printf("UTS51: %zu RGI sequences validated\n", a + b);
    printf("test_uts51: %zu checks, %zu failures\n", checks, failures);
    if (failures == 0) {
        printf("test_uts51: PASS\n");
        return 0;
    }
    printf("test_uts51: FAIL\n");
    return 1;
}
