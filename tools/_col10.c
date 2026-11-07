#include "unicodoid/unicodoid_ucd.h"
#include <stdio.h>

#define HANGUL_SBASE 0xAC00u
#define HANGUL_SCOUNT 11172u
#define NFD_MAX 512

static void dbg(const char* label, const uint32_t* cps, size_t n) {
    printf("%s: cps=[", label);
    for (size_t i = 0; i < n; ++i) {
        uint8_t cc = unicodoid_ucd_canonical_combining_class(cps[i]);
        printf("%s U+%04X(ccc=%u)", i ? " " : "", cps[i], cc);
    }
    printf("]\n");

    unicodoid_ucd_weight_value_t w[24];
    size_t nw = unicodoid_ucd_collation_weights(cps[0], w, 24);
    printf("  cp0 weights=%zu:", nw);
    for (size_t i = 0; i < nw; ++i) printf(" [%04X.%04X.%04X v=%d]", w[i].primary, w[i].secondary, w[i].tertiary, (int)w[i].variable);
    printf("\n");
    nw = unicodoid_ucd_collation_weights(cps[n-1], w, 24);
    printf("  cpN weights=%zu:", nw);
    for (size_t i = 0; i < nw; ++i) printf(" [%04X.%04X.%04X v=%d]", w[i].primary, w[i].secondary, w[i].tertiary, (int)w[i].variable);
    printf("\n");
}

int main(void) {
    uint32_t a[] = {0x1CD4, 0x0334};
    uint32_t b[] = {0x0334, 0x1CD4};
    dbg("1CD4 0334", a, 2);
    dbg("0334 1CD4", b, 2);
    return 0;
}
