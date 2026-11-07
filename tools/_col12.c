#include "unicodoid/unicodoid_ucd.h"
#include <stdio.h>
int main(void) {
    uint32_t seq[] = {0x0FB2, 0x0F71, 0x0F72};
    uint32_t seq2[] = {0x0F71, 0x0F72};
    uint32_t seq3[] = {0x0FB2, 0x0F73};
    uint32_t seq4[] = {0x0FB2, 0x0F71};
    unicodoid_ucd_weight_value_t w[8];
    size_t matched = 0;
    size_t n;
    n = unicodoid_ucd_collation_contraction(seq2, 2, &matched, w, 8);
    printf("0F71 0F72: n=%zu matched=%zu:", n, matched);
    for (size_t i = 0; i < n; ++i) printf(" [%04X.%04X.%04X v=%d]", w[i].primary, w[i].secondary, w[i].tertiary, (int)w[i].variable);
    printf("\n");
    n = unicodoid_ucd_collation_contraction(seq3, 2, &matched, w, 8);
    printf("0FB2 0F73: n=%zu matched=%zu:", n, matched);
    for (size_t i = 0; i < n; ++i) printf(" [%04X.%04X.%04X v=%d]", w[i].primary, w[i].secondary, w[i].tertiary, (int)w[i].variable);
    printf("\n");
    n = unicodoid_ucd_collation_contraction(seq4, 2, &matched, w, 8);
    printf("0FB2 0F71: n=%zu matched=%zu:", n, matched);
    for (size_t i = 0; i < n; ++i) printf(" [%04X.%04X.%04X v=%d]", w[i].primary, w[i].secondary, w[i].tertiary, (int)w[i].variable);
    printf("\n");
    n = unicodoid_ucd_collation_contraction(seq, 3, &matched, w, 8);
    printf("0FB2 0F71 0F72: n=%zu matched=%zu:", n, matched);
    for (size_t i = 0; i < n; ++i) printf(" [%04X.%04X.%04X v=%d]", w[i].primary, w[i].secondary, w[i].tertiary, (int)w[i].variable);
    printf("\n");
    return 0;
}
