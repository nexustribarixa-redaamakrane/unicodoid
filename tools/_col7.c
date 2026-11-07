#include "unicodoid/unicodoid_ucd_data.h"
#include <stdio.h>
int main(void) {
    for (size_t i = 0; i < unicodoid_ucd_collation_single_count; ++i) {
        const unicodoid_ucd_seq_t* e = &unicodoid_ucd_collation_single[i];
        if (e->cp == 0xFDFA) {
            printf("found at %zu: cp=%04X offset=%04X length=%u\n", i, e->cp, e->offset, e->length);
            const uint32_t* w = &unicodoid_ucd_collation_keys[e->offset * 4];
            for (size_t k = 0; k < 8; ++k) printf("  [%u] = %04X %04X %04X %d\n", (unsigned)k, w[k*4], w[k*4+1], w[k*4+2], w[k*4+3]);
        }
    }
    return 0;
}
