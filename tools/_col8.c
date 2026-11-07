#include "unicodoid/unicodoid_ucd.h"
#include <stdio.h>
int main(void) {
    unicodoid_ucd_weight_value_t w[8];
    size_t n = unicodoid_ucd_collation_weights(0xFDFA, w, 8);
    printf("n=%zu\n", n);
    for (size_t k = 0; k < (n < 8 ? n : 8); ++k)
        printf("  [%zu] %04X.%04X.%04X v=%d\n", k, w[k].primary, w[k].secondary, w[k].tertiary, (int)w[k].variable);
    return 0;
}
