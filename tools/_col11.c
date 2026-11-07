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
    dbg("0FB2 0F73 003F", (uint32_t[]){0x0FB2,0x0F73,0x003F}, 3);
    dbg("0FB2 0001 0F71 0F72 0061", (uint32_t[]){0x0FB2,0x0001,0x0F71,0x0F72,0x0061}, 5);
    dbg("0FB2 0F71 0F72 0061", (uint32_t[]){0x0FB2,0x0F71,0x0F72,0x0061}, 4);
    dbg("0FB2 0F71 0F72 003F", (uint32_t[]){0x0FB2,0x0F71,0x0F72,0x003F}, 4);
    dbg("0FB2 0F73 0061", (uint32_t[]){0x0FB2,0x0F73,0x0061}, 3);
    return 0;
}
