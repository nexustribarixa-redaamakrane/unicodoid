#include "unicodoid/uts/uts10_collation.h"
#include <stdio.h>
#include <string.h>

static void dbg(const char* label, const uint32_t* cps, size_t n) {
    unicodoid_uts10_unit_t key[64];
    size_t len = unicodoid_uts10_sortkey(cps, n, key, 64);
    printf("%s: ", label);
    for (size_t i = 0; i < len; ++i) {
        if (key[i] == 0) printf("00  ");
        else printf("%04X ", key[i]);
    }
    printf("\n");
}

int main(void) {
    const uint32_t a[] = {0x61};      /* a */
    const uint32_t b[] = {0x62};      /* b */
    const uint32_t dot_above[] = {0x0307};
    const uint32_t dot_below[] = {0x0323};
    const uint32_t ccc038[] = {0x0338};
    const uint32_t c2334[] = {0x0334};
    const uint32_t c2336[] = {0x0336};
    const uint32_t c2337[] = {0x0337};
    dbg("0338", ccc038, 1);
    dbg("0336", c2336, 1);
    dbg("0337", c2337, 1);
    dbg("0334", c2334, 1);
    dbg("a", a, 1);
    dbg("b", b, 1);
    dbg("dot_above", dot_above, 1);
    dbg("dot_below", dot_below, 1);
    dbg("0338 0334", (uint32_t[]){0x0338,0x0334}, 2);
    dbg("0336 0334", (uint32_t[]){0x0336,0x0334}, 2);
    dbg("0337 0334", (uint32_t[]){0x0337,0x0334}, 2);
    return 0;
}
