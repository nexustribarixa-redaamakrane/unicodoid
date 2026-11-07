// Official Unicode 18.0.0 normalization conformance test (UAX #15).
//
// The vectors are read directly from the UCD distribution; nothing is copied or
// curated.  The directory is provided by the build system through
// UNICODOID_UCD_DIR.

#include "unicodoid/unicodoid.h"
#include "unicodoid/unicodoid_normalization.h"
#include "unicodoid/systems/unicodoid_utf8.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CP_MAX 64

typedef struct {
    uint32_t source[CP_MAX];
    uint32_t nfc[CP_MAX];
    uint32_t nfd[CP_MAX];
    uint32_t nfkc[CP_MAX];
    uint32_t nfkd[CP_MAX];
    size_t source_len, nfc_len, nfd_len, nfkc_len, nfkd_len;
} norm_case_t;

static int utf8_encode(const uint32_t* cps, size_t count, uint8_t* out, size_t cap) {
    size_t off = 0;
    for (size_t i = 0; i < count; ++i) {
        uint32_t cp = cps[i];
        if (cp < 0x80) {
            if (off >= cap) return -1;
            out[off++] = (uint8_t)cp;
        } else if (cp < 0x800) {
            if (off + 2 > cap) return -1;
            out[off++] = (uint8_t)(0xC0 | (cp >> 6));
            out[off++] = (uint8_t)(0x80 | (cp & 0x3F));
        } else if (cp < 0x10000) {
            if (off + 3 > cap) return -1;
            out[off++] = (uint8_t)(0xE0 | (cp >> 12));
            out[off++] = (uint8_t)(0x80 | ((cp >> 6) & 0x3F));
            out[off++] = (uint8_t)(0x80 | (cp & 0x3F));
        } else {
            if (off + 4 > cap) return -1;
            out[off++] = (uint8_t)(0xF0 | (cp >> 18));
            out[off++] = (uint8_t)(0x80 | ((cp >> 12) & 0x3F));
            out[off++] = (uint8_t)(0x80 | ((cp >> 6) & 0x3F));
            out[off++] = (uint8_t)(0x80 | (cp & 0x3F));
        }
    }
    return (int)off;
}

static size_t parse_cps(const char* field, uint32_t* out, size_t cap) {
    size_t n = 0;
    const char* p = field;
    while (*p) {
        while (*p == ' ') ++p;
        if (!*p) break;
        unsigned long cp = strtoul(p, (char**)&p, 16);
        if (n >= cap) { fprintf(stderr, "too many code points\n"); exit(1); }
        out[n++] = (uint32_t)cp;
        while (*p == ' ') ++p;
    }
    return n;
}

static int normalize_to_buf(const uint8_t* in, size_t in_len, unicodoid_norm_form_t form,
                            uint8_t* out, size_t out_cap) {
    const unicodoid_system_t* sys = unicodoid_system_get_utf8();
    size_t written = 0;
    unicodoid_status_t st = unicodoid_normalize(sys, in, in_len, form, out, out_cap, &written);
    if (st != UNICODOID_OK) {
        fprintf(stderr, "normalize failed with status %d\n", (int)st);
        exit(1);
    }
    return (int)written;
}

static const char* form_name(unicodoid_norm_form_t form) {
    switch (form) {
        case UNICODOID_NORM_NFC: return "NFC";
        case UNICODOID_NORM_NFD: return "NFD";
        case UNICODOID_NORM_NFKC: return "NFKC";
        default: return "NFKD";
    }
}

static void check_form(const norm_case_t* tc, unicodoid_norm_form_t form,
                       const uint32_t* expected, size_t expected_len, int* failures, int* checks) {
    uint8_t src_bytes[CP_MAX * 4];
    uint8_t out_bytes[CP_MAX * 4];
    uint8_t expected_bytes[CP_MAX * 4];
    int src_len = utf8_encode(tc->source, tc->source_len, src_bytes, sizeof src_bytes);
    int exp_len = utf8_encode(expected, expected_len, expected_bytes, sizeof expected_bytes);
    if (src_len < 0 || exp_len < 0) {
        fprintf(stderr, "FAIL %s: buffer overflow while encoding test vector\n", form_name(form));
        exit(1);
    }
    int out_len = normalize_to_buf(src_bytes, (size_t)src_len, form, out_bytes, sizeof out_bytes);

    ++*checks;
    if ((size_t)out_len == (size_t)exp_len && (exp_len == 0 || memcmp(out_bytes, expected_bytes, (size_t)exp_len) == 0)) {
        return;
    }
    ++*failures;
    if (*failures <= 5) {
        fprintf(stderr, "FAIL %s: source=[", form_name(form));
        for (size_t i = 0; i < tc->source_len; ++i) fprintf(stderr, "%s U+%04X", i ? " " : "", tc->source[i]);
        fprintf(stderr, "]\n  expected=[");
        for (size_t i = 0; i < expected_len; ++i) fprintf(stderr, "%s U+%04X", i ? " " : "", expected[i]);
        fprintf(stderr, "]\n");
    }
}

static void check_input(const norm_case_t* tc, unicodoid_norm_form_t form,
                        const uint32_t* input, size_t input_len,
                        const uint32_t* expected, size_t expected_len,
                        int* failures, int* checks) {
    norm_case_t t = *tc;
    memcpy(t.source, input, input_len * sizeof(uint32_t));
    t.source_len = input_len;
    check_form(&t, form, expected, expected_len, failures, checks);
}

static int run_field_checks(const norm_case_t* tc, int* failures, int* checks) {
    /* UAX #15 conformance invariants (20 per case). */
    check_input(tc, UNICODOID_NORM_NFC, tc->source, tc->source_len, tc->nfc, tc->nfc_len, failures, checks);
    check_input(tc, UNICODOID_NORM_NFC, tc->nfc, tc->nfc_len, tc->nfc, tc->nfc_len, failures, checks);
    check_input(tc, UNICODOID_NORM_NFC, tc->nfd, tc->nfd_len, tc->nfc, tc->nfc_len, failures, checks);
    check_input(tc, UNICODOID_NORM_NFC, tc->nfkc, tc->nfkc_len, tc->nfkc, tc->nfkc_len, failures, checks);
    check_input(tc, UNICODOID_NORM_NFC, tc->nfkd, tc->nfkd_len, tc->nfkc, tc->nfkc_len, failures, checks);

    check_input(tc, UNICODOID_NORM_NFD, tc->source, tc->source_len, tc->nfd, tc->nfd_len, failures, checks);
    check_input(tc, UNICODOID_NORM_NFD, tc->nfc, tc->nfc_len, tc->nfd, tc->nfd_len, failures, checks);
    check_input(tc, UNICODOID_NORM_NFD, tc->nfd, tc->nfd_len, tc->nfd, tc->nfd_len, failures, checks);
    check_input(tc, UNICODOID_NORM_NFD, tc->nfkc, tc->nfkc_len, tc->nfkd, tc->nfkd_len, failures, checks);
    check_input(tc, UNICODOID_NORM_NFD, tc->nfkd, tc->nfkd_len, tc->nfkd, tc->nfkd_len, failures, checks);

    check_input(tc, UNICODOID_NORM_NFKC, tc->source, tc->source_len, tc->nfkc, tc->nfkc_len, failures, checks);
    check_input(tc, UNICODOID_NORM_NFKC, tc->nfc, tc->nfc_len, tc->nfkc, tc->nfkc_len, failures, checks);
    check_input(tc, UNICODOID_NORM_NFKC, tc->nfd, tc->nfd_len, tc->nfkc, tc->nfkc_len, failures, checks);
    check_input(tc, UNICODOID_NORM_NFKC, tc->nfkc, tc->nfkc_len, tc->nfkc, tc->nfkc_len, failures, checks);
    check_input(tc, UNICODOID_NORM_NFKC, tc->nfkd, tc->nfkd_len, tc->nfkc, tc->nfkc_len, failures, checks);

    check_input(tc, UNICODOID_NORM_NFKD, tc->source, tc->source_len, tc->nfkd, tc->nfkd_len, failures, checks);
    check_input(tc, UNICODOID_NORM_NFKD, tc->nfc, tc->nfc_len, tc->nfkd, tc->nfkd_len, failures, checks);
    check_input(tc, UNICODOID_NORM_NFKD, tc->nfd, tc->nfd_len, tc->nfkd, tc->nfkd_len, failures, checks);
    check_input(tc, UNICODOID_NORM_NFKD, tc->nfkc, tc->nfkc_len, tc->nfkd, tc->nfkd_len, failures, checks);
    check_input(tc, UNICODOID_NORM_NFKD, tc->nfkd, tc->nfkd_len, tc->nfkd, tc->nfkd_len, failures, checks);
    return 0;
}

int main(void) {
    setvbuf(stderr, NULL, _IONBF, 0);
    const char* dir = UNICODOID_UCD_DIR;
    char path[1024];
    snprintf(path, sizeof path, "%s/NormalizationTest.txt", dir);

    FILE* f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "cannot open %s\n", path);
        return 1;
    }

    int failures = 0;
    int checks = 0;
    size_t cases = 0;
    char line[4096];

    while (fgets(line, sizeof line, f)) {
        if (line[0] == '#' || line[0] == '@') continue;
        /* Strip comment. */
        char* hash = strchr(line, '#');
        if (hash) *hash = '\0';

        char* fields[5] = { 0 };
        size_t nf = 0;
        char* tok = strtok(line, ";");
        while (tok && nf < 5) { fields[nf++] = tok; tok = strtok(NULL, ";"); }
        if (nf < 5) continue;

        norm_case_t tc;
        memset(&tc, 0, sizeof tc);
        tc.source_len = parse_cps(fields[0], tc.source, CP_MAX);
        tc.nfc_len = parse_cps(fields[1], tc.nfc, CP_MAX);
        tc.nfd_len = parse_cps(fields[2], tc.nfd, CP_MAX);
        tc.nfkc_len = parse_cps(fields[3], tc.nfkc, CP_MAX);
        tc.nfkd_len = parse_cps(fields[4], tc.nfkd, CP_MAX);
        if (tc.source_len == 0) continue;

        ++cases;
        if (cases % 2000 == 0) fprintf(stderr, ".... case %zu\n", cases);
        run_field_checks(&tc, &failures, &checks);
    }
    fclose(f);

    printf("NormalizationTest.txt: %zu cases, %d checks, %d failures\n", cases, checks, failures);
    if (failures == 0) {
        printf("test_normalization: PASS\n");
        return 0;
    }
    fprintf(stderr, "test_normalization: %d checks failed\n", failures);
    return 1;
}
