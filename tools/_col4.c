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
    dbg("006C FFFE 0066 0032 0061", (uint32_t[]){0x006C,0xFFFE,0x0066,0x0032,0x0061}, 5);
    dbg("006C FFFE 0066 0591 0032 0061", (uint32_t[]){0x006C,0xFFFE,0x0066,0x0591,0x0032,0x0061}, 6);
    dbg("006C 0001 FFFE 0066 0032 0061", (uint32_t[]){0x006C,0x0001,0xFFFE,0x0066,0x0032,0x0061}, 6);
    dbg("006C 0591 FFFE 0066 0032 0061", (uint32_t[]){0x006C,0x0591,0xFFFE,0x0066,0x0032,0x0061}, 6);
    dbg("006C 1D165 FFFE 0066 0032 0061", (uint32_t[]){0x006C,0x1D165,0xFFFE,0x0066,0x0032,0x0061}, 6);
    return 0;
}
