// Conformance test for UAX #34 Unicode Named Character Sequences.
//
// Every entry of the official Unicode 18.0.0 NamedSequences.txt is looked up
// by name and by sequence membership.

#include "unicodoid/uax/uax34_named_sequences.h"

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

int main(void) {
    const char* path = UNICODOID_NAMED_SEQUENCES_FILE;
    FILE* f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "FAIL: cannot open %s\n", path);
        return 1;
    }

    size_t lines = 0;
    char line[1024];
    while (fgets(line, sizeof line, f)) {
        if (line[0] == '#' || line[0] == '@' || line[0] == '\n' || line[0] == '\r') {
            continue;
        }
        char* semi = strchr(line, ';');
        if (!semi) continue;
        *semi = '\0';

        /* Trim the name. */
        char* name = line;
        while (*name == ' ' || *name == '\t') ++name;
        char* name_end = name + strlen(name);
        while (name_end > name && (name_end[-1] == ' ' || name_end[-1] == '\t' ||
                                   name_end[-1] == '\r' || name_end[-1] == '\n')) {
            *--name_end = '\0';
        }

        uint32_t expected[64];
        size_t expected_len = 0;
        char* p = semi + 1;
        while (*p) {
            while (*p == ' ' || *p == '\t') ++p;
            if (!*p || *p == '\n' || *p == '\r') break;
            unsigned long cp = strtoul(p, &p, 16);
            if (expected_len < 64) expected[expected_len++] = (uint32_t)cp;
        }

        uint32_t actual[64];
        size_t actual_len = 0;
        check(unicodoid_uax34_find(name, actual, 64, &actual_len),
              "named sequence found by name");
        check(actual_len == expected_len, "named sequence length");
        check(actual_len == expected_len &&
              memcmp(actual, expected, actual_len * sizeof(uint32_t)) == 0,
              "named sequence code points");

        check(unicodoid_uax34_is_registered(expected, expected_len),
              "named sequence is registered");
        ++lines;
    }
    fclose(f);

    check(unicodoid_uax34_count() == lines, "count matches data file");

    uint32_t bad[] = { 0xD800, 0x0020 };
    check(!unicodoid_uax34_is_registered(bad, 2), "surrogate rejected");
    check(!unicodoid_uax34_is_registered(NULL, 0), "empty rejected");

    size_t dummy_len = 0;
    check(!unicodoid_uax34_find("NOT A REAL SEQUENCE NAME", NULL, 0, &dummy_len),
          "unknown name rejected");

    printf("UAX34: %zu sequences validated\n", lines);
    printf("test_uax34: %zu checks, %zu failures\n", checks, failures);
    if (failures == 0) {
        printf("test_uax34: PASS\n");
        return 0;
    }
    printf("test_uax34: FAIL\n");
    return 1;
}
