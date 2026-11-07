#include "unicodoid/uts/uts10_collation.h"
#include <stdio.h>
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
    dbg("321D", (uint32_t[]){0x321D}, 1);
    dbg("321E", (uint32_t[]){0x321E}, 1);
    dbg("3208", (uint32_t[]){0x3208}, 1);
    dbg("33E1", (uint32_t[]){0x33E1}, 1);
    dbg("1F19C", (uint32_t[]){0x1F19C}, 1);
    dbg("1F19C 003F", (uint32_t[]){0x1F19C,0x3F}, 2);
    dbg("1F19C 0061", (uint32_t[]){0x1F19C,0x61}, 2);
    dbg("33E1 003F", (uint32_t[]){0x33E1,0x3F}, 2);
    dbg("33E1 0061", (uint32_t[]){0x33E1,0x61}, 2);
    return 0;
}
