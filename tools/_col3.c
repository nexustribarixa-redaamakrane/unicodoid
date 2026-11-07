#include "unicodoid/unicodoid_ucd.h"
#include <stdio.h>
int main(void) {
    uint32_t cps[] = {0x321D, 0x321E, 0x3208, 0x33E1, 0x1F19C, 0x0041, 0x0061};
    for (size_t i = 0; i < sizeof cps/sizeof cps[0]; ++i) {
        unicodoid_ucd_weight_value_t w[8];
        size_t n = unicodoid_ucd_collation_weights(cps[i], w, 8);
        printf("U+%04X -> %zu weights:", cps[i], n);
        for (size_t k = 0; k < n; ++k) printf(" [%04X.%04X.%04X v=%d]", w[k].primary, w[k].secondary, w[k].tertiary, (int)w[k].variable);
        printf("\n");
    }
    return 0;
}
