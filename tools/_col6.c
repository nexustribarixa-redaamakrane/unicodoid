#include "unicodoid/unicodoid_ucd.h"
#include <stdio.h>
int main(void) {
    uint32_t cps[] = {0xFDF5, 0xFDFA, 0xFDF3, 0x0334, 0x0001, 0x1CD4, 0x1E8D4};
    for (size_t i = 0; i < sizeof cps/sizeof cps[0]; ++i) {
        unicodoid_ucd_weight_value_t w[16];
        size_t n = unicodoid_ucd_collation_weights(cps[i], w, 16);
        printf("U+%04X -> %zu weights:", cps[i], n);
        for (size_t k = 0; k < n; ++k) printf(" [%04X.%04X.%04X v=%d]", w[k].primary, w[k].secondary, w[k].tertiary, (int)w[k].variable);
        printf("\n");
    }
    return 0;
}
