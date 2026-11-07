// Conformance and algorithm tests for UTR #59 East Asian Spacing.
//
// The property is validated against the official derived data file
// data/ucd/18.0.0/tr59/east-asian-spacing.txt for every listed code point.
// The spacing algorithm is exercised on the cases described in UTR59 §3.3.

#include "unicodoid/utr/utr59_east_asian_spacing.h"
#include "unicodoid/unicodoid_ucd.h"

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

static void check_value(uint32_t cp, uint8_t expected, const char* what) {
    ++checks;
    uint8_t actual = unicodoid_utr59_east_asian_spacing(cp);
    if (actual != expected) {
        ++failures;
        fprintf(stderr, "FAIL: %s U+%04X expected %u got %u\n",
                what, cp, (unsigned)expected, (unsigned)actual);
    }
}

static int eas_from_letter(char c) {
    switch (c) {
        case 'W': return UNICODOID_EAS_W;
        case 'N': return UNICODOID_EAS_N;
        case 'O': return UNICODOID_EAS_O;
        case 'C': return UNICODOID_EAS_C;
        default: return -1;
    }
}

static void test_data_file(void) {
    const char* path = UNICODOID_TR59_DATA_FILE;
    FILE* f = fopen(path, "r");
    if (!f) {
        ++failures;
        fprintf(stderr, "FAIL: cannot open %s\n", path);
        return;
    }

    char line[1024];
    size_t entries = 0;
    while (fgets(line, sizeof line, f)) {
        if (line[0] == '#' || line[0] == '@') continue;
        char* semi = strchr(line, ';');
        if (!semi) continue;
        *semi = '\0';
        char* value_field = semi + 1;
        while (*value_field == ' ' || *value_field == '\t') ++value_field;
        int expected = eas_from_letter(*value_field);
        if (expected < 0) continue;

        char* range = line;
        while (*range == ' ' || *range == '\t') ++range;
        char* end = range;
        while (*end && *end != '.' && *end != ' ' && *end != '\t' &&
               *end != '\n' && *end != '\r') {
            ++end;
        }
        char saved = *end;
        *end = '\0';
        unsigned long first = strtoul(range, NULL, 16);
        unsigned long last = first;
        if (saved == '.') {
            char* second = end + 1;
            if (*second == '.') ++second;
            last = strtoul(second, NULL, 16);
        }

        for (unsigned long cp = first; cp <= last; ++cp) {
            ++checks;
            if (unicodoid_utr59_east_asian_spacing((uint32_t)cp) != (uint8_t)expected) {
                ++failures;
                fprintf(stderr,
                        "FAIL: data file U+%04lX expected %d got %u\n",
                        cp, expected,
                        (unsigned)unicodoid_utr59_east_asian_spacing((uint32_t)cp));
            }
        }
        ++entries;
    }
    fclose(f);
    printf("UTR59 data file: %zu ranges validated\n", entries);
}

static size_t collect(const uint32_t* cps, size_t n, bool chinese, bool vertical,
                      size_t* out, size_t cap) {
    return unicodoid_utr59_spacing_offsets(cps, n, chinese, vertical, out, cap);
}

static void test_algorithm(void) {
    size_t out[8];

    /* Wide / Narrow adjacency in both directions. */
    {
        uint32_t s[] = { 0x41, 0x6F22 }; /* A + HAN */
        size_t n = collect(s, 2, false, false, out, 8);
        check(n == 1 && out[0] == 1, "A+Han inserts before Han");
    }
    {
        uint32_t s[] = { 0x6F22, 0x41 }; /* HAN + A */
        size_t n = collect(s, 2, false, false, out, 8);
        check(n == 1 && out[0] == 1, "Han+A inserts before A");
    }
    {
        uint32_t s[] = { 0x6F22, 0x3002 }; /* HAN + IDEOGRAPHIC FULL STOP (O) */
        size_t n = collect(s, 2, false, false, out, 8);
        check(n == 0, "Han+O does not insert");
    }
    {
        uint32_t s[] = { 0x6F22, 0x31 }; /* HAN + digit 1 */
        size_t n = collect(s, 2, false, false, out, 8);
        check(n == 1 && out[0] == 1, "Han+digit inserts");
    }

    /* Conditional value depends on the language context. */
    {
        uint32_t s[] = { 0x6F22, 0x25 }; /* HAN + PERCENT SIGN */
        size_t n = collect(s, 2, false, false, out, 8);
        check(n == 0, "Han+percent non-Chinese does not insert");
        n = collect(s, 2, true, false, out, 8);
        check(n == 1 && out[0] == 1, "Han+percent Chinese inserts");
    }

    /* Combining marks stay in the preceding cluster. */
    {
        uint32_t s[] = { 0x41, 0x0301, 0x6F22 }; /* A + acute + HAN */
        size_t n = collect(s, 3, false, false, out, 8);
        check(n == 1 && out[0] == 2, "combining mark stays in cluster");
    }

    /* Enclosing mark (Me) forces the cluster to Other. */
    {
        uint32_t s[] = { 0x6F22, 0x20DD, 0x41 }; /* HAN + enclosing circle + A */
        size_t n = collect(s, 3, false, false, out, 8);
        check(n == 0, "Me cluster is Other");
    }

    /* Multiple insertions. */
    {
        uint32_t s[] = { 0x6F22, 0x41, 0x6F22 }; /* HAN A HAN */
        size_t n = collect(s, 3, false, false, out, 8);
        check(n == 2 && out[0] == 1 && out[1] == 2, "alternating insertions");
    }

    /* Buffer query semantics. */
    {
        uint32_t s[] = { 0x6F22, 0x41, 0x6F22 };
        size_t n = unicodoid_utr59_spacing_offsets(s, 3, false, false, NULL, 0);
        check(n == 2, "NULL out returns count");
        n = collect(s, 3, false, false, out, 1);
        check(n == 2, "cap limits writes but not count");
    }

    /* Empty input. */
    {
        size_t n = unicodoid_utr59_spacing_offsets(NULL, 0, false, false, out, 8);
        check(n == 0, "empty input has no spacing");
    }

    /* Cluster resolution: vertical upright Narrow becomes Other. */
    {
        uint32_t s[] = { 0x41 };
        check(unicodoid_utr59_cluster_value(s, 1, false, false) == UNICODOID_EAS_N,
              "A is Narrow");
        check(unicodoid_utr59_cluster_value(s, 1, false, true) == UNICODOID_EAS_N,
              "A stays Narrow when rotated");
    }
}

static void test_defaults(void) {
    /* UTR59 data file omits code points whose value is O and EAW is N/Na. */
    check_value(0x0020, UNICODOID_EAS_O, "SPACE is Other");
    check_value(0x4E00, UNICODOID_EAS_W, "HAN is Wide");
    check_value(0x0041, UNICODOID_EAS_N, "LATIN A is Narrow");
    check_value(0x0025, UNICODOID_EAS_C, "PERCENT is Conditional");
    check_value(0x3002, UNICODOID_EAS_O, "IDEOGRAPHIC FULL STOP is Other");
}

int main(void) {
    test_data_file();
    test_algorithm();
    test_defaults();

    printf("test_utr59: %zu checks, %zu failures\n", checks, failures);
    if (failures == 0) {
        printf("test_utr59: PASS\n");
        return 0;
    }
    printf("test_utr59: FAIL\n");
    return 1;
}
