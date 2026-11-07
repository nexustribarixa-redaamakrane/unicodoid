#include "unicodoid/uts/uts10_collation.h"
#include <stdio.h>
#include <string.h>
static void dbg(const char* label, const uint32_t* cps, size_t n) {
    unicodoid_uts10_unit_t key[128];
    size_t len = unicodoid_uts10_sortkey(cps, n, key, 128);
    printf("%s: ", label);
    for (size_t i = 0; i < len; ++i) {
        if (key[i] == 0) printf("00  ");
        else printf("%04X ", key[i]);
    }
    printf("\n");
}
int main(void) {
    dbg("FDF5 0062", (uint32_t[]){0xFDF5,0x0062}, 2);
    dbg("FDFA 0021", (uint32_t[]){0xFDFA,0x0021}, 2);
    dbg("FDF5", (uint32_t[]){0xFDF5}, 1);
    dbg("FDFA", (uint32_t[]){0xFDFA}, 1);
    return 0;
}
