#include "unicodoid/unicodoid.h"
#include "unicodoid/unicodoid_normalization.h"
#include "unicodoid/systems/unicodoid_utf8.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
int main(int argc, char** argv) {
    unsigned cp = (unsigned)strtoul(argv[1], 0, 16);
    uint8_t src[4]; size_t n = 0;
    if (cp < 0x800) { src[0]=0xC0|(cp>>6); src[1]=0x80|(cp&0x3F); n=2; }
    else { src[0]=0xE0|(cp>>12); src[1]=0x80|((cp>>6)&0x3F); src[2]=0x80|(cp&0x3F); n=3; }
    const unicodoid_system_t* sys = unicodoid_system_get_utf8();
    uint8_t out[64]; size_t w = 0;
    unicodoid_status_t st = unicodoid_normalize(sys, src, n, UNICODOID_NORM_NFKC, out, sizeof out, &w);
    printf("cp=%04X st=%d outlen=%zu\n", cp, (int)st, w);
    return 0;
}

