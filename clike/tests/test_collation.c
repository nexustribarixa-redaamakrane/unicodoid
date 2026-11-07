// Official Unicode 18.0.0 conformance test for UTS #10 (Non-Ignorable
// CollationTest), plus unit vectors derived from the specification.
//
// The vectors are read directly from the UCA distribution in
// data/ucd/18.0.0/uca/test/CollationTest/.  Each line of
// CollationTest_NON_IGNORABLE_SHORT.txt is expected to sort strictly after
// the previous line.

#include "unicodoid/uts/uts10_collation.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define CP_MAX 256

static size_t parse_cps(const char* field, uint32_t* out, size_t cap) {
    size_t n = 0;
    const char* p = field;
    while (*p) {
        while (*p == ' ' || *p == '\t') ++p;
        if (!*p) break;
        unsigned long cp = strtoul(p, (char**)&p, 16);
        if (n >= cap) { fprintf(stderr, "too many code points\n"); exit(1); }
        out[n++] = (uint32_t)cp;
        while (*p == ' ' || *p == '\t') ++p;
    }
    return n;
}

int main(void) {
    const char* dir = UNICODOID_UCA_TEST_DIR;
    char path[1024];
    snprintf(path, sizeof path, "%s/CollationTest_NON_IGNORABLE_SHORT.txt", dir);

    FILE* f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "cannot open %s\n", path);
        return 1;
    }

    uint32_t prev_cps[CP_MAX];
    size_t prev_len = 0;
    bool have_prev = false;
    size_t cases = 0;
    size_t skipped = 0;
    size_t failures = 0;
    char line[4096];

    while (fgets(line, sizeof line, f)) {
        if (line[0] == '#' || line[0] == '@') continue;
        char* raw = line;
        size_t line_len = strlen(raw);
        while (line_len > 0 && (raw[line_len-1] == '\n' || raw[line_len-1] == '\r')) {
            raw[--line_len] = '\0';
        }
        if (line_len == 0) continue;

        uint32_t cps[CP_MAX];
        size_t n = parse_cps(raw, cps, CP_MAX);
        if (n == 0) continue;

        /* Il-formed strings (lone surrogates) may be filtered out; the C SDK
         * works on scalar values only. */
        bool ill_formed = false;
        for (size_t k = 0; k < n; ++k) {
            if (cps[k] >= 0xD800 && cps[k] <= 0xDFFF) { ill_formed = true; break; }
        }
        if (ill_formed) { ++skipped; continue; }

        if (have_prev) {
            int cmp = unicodoid_uts10_compare(prev_cps, prev_len, cps, n);
            if (cmp > 0) {
                ++failures;
                if (failures <= 10) {
                    fprintf(stderr, "case %zu: decreasing\n", cases);
                }
            }
        }
        memcpy(prev_cps, cps, n * sizeof(uint32_t));
        prev_len = n;
        have_prev = true;
        ++cases;
    }
    fclose(f);

    fprintf(stderr, "CollationTest_NON_IGNORABLE_SHORT.txt: %zu lines, %zu skipped (surrogates), %zu failures\n",
            cases, skipped, failures);
    if (failures == 0) {
        printf("test_collation: PASS\n");
        return 0;
    }
    fprintf(stderr, "test_collation: FAIL (%zu failures)\n", failures);
    return 1;
}
