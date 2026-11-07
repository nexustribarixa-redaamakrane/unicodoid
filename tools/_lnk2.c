#include "unicodoid/unicodoid.h"
#include "unicodoid/unicodoid_ucd.h"
#include <stdio.h>
int main(void) {
    uint32_t cps[] = {0x21, 0x28, 0x29, 0x23, 0x25, 0x2C, 0x3F, 0x3A, 0x30, 0x41};
    for (size_t i = 0; i < 10; ++i) printf("U+%04X -> %u\n", cps[i], unicodoid_ucd_link_term(cps[i]));
    return 0;
}
