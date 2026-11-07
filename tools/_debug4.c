#include "unicodoid/unicodoid.h"
#include "unicodoid/unicodoid_normalization.h"
#include "unicodoid/systems/unicodoid_utf8.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(int argc, char** argv) {
    unsigned cp = (unsigned)strtoul(argv[1], 0, 16);
    uint8_t src[8]; size_t n = 0;
    src[0]=0xEF; src[1]=0xB7; src[2]=0xBA; n=3;
    if (cp != 0xFDFA) {
      if (cp < 0x800) { src[0]=0xC0|(cp>>6); src[1]=0x80|(cp&0x3F); n=2; }
      else if (cp < 0x10000) { src[0]=0xE0|(cp>>12); src[1]=0x80|((cp>>6)&0x3F); src[2]=0x80|(cp&0x3F); n=3; }
      else { src[0]=0xF0|(cp>>18); src[1]=0x80|((cp>>12)&0x3F); src[2]=0x80|((cp>>6)&0x3F); src[3]=0x80|(cp&0x3F); n=4; }
    }
    const unicodoid_system_t* sys = unicodoid_system_get_utf8();
    uint8_t out[256]; size_t w = 0;
    unicodoid_status_t st = unicodoid_normalize(sys, src, n, UNICODOID_NORM_NFKC, out, sizeof out, &w);
    printf("cp=%04X st=%d outlen=%zu\n", cp, (int)st, w);
    if (w > 0) { for (size_t i = 0; i < w; ++i) printf("%02X ", out[i]); printf("\n"); }
    return 0;
}
