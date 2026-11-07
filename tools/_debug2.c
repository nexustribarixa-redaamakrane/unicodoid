#include "unicodoid/unicodoid.h"
#include "unicodoid/unicodoid_normalization.h"
#include "unicodoid/systems/unicodoid_utf8.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static size_t utf8_encode(const uint32_t* cps, size_t count, uint8_t* out, size_t cap) {
    size_t off = 0;
    for (size_t i = 0; i < count; ++i) {
        uint32_t cp = cps[i];
        if (cp < 0x80) out[off++] = (uint8_t)cp;
        else if (cp < 0x800) { out[off++] = 0xC0|(cp>>6); out[off++] = 0x80|(cp&0x3F); }
        else if (cp < 0x10000) { out[off++] = 0xE0|(cp>>12); out[off++] = 0x80|((cp>>6)&0x3F); out[off++] = 0x80|(cp&0x3F); }
        else { out[off++] = 0xF0|(cp>>18); out[off++] = 0x80|((cp>>12)&0x3F); out[off++] = 0x80|((cp>>6)&0x3F); out[off++] = 0x80|(cp&0x3F); }
        if (off > cap) abort();
    }
    return off;
}

int main(int argc, char** argv) {
    setvbuf(stderr, NULL, _IONBF, 0);
    size_t from = (size_t)atoi(argv[1]);
    size_t to = (size_t)atoi(argv[2]);
    FILE* f = fopen(argv[3], "r");
    char line[4096];
    size_t n = 0;
    const unicodoid_system_t* sys = unicodoid_system_get_utf8();
    while (fgets(line, sizeof line, f)) {
        if (line[0] == '#' || line[0] == '@') continue;
        char* hash = strchr(line, '#');
        if (hash) *hash = '\0';
        char* fields[5] = {0};
        size_t nf = 0;
        char* save = NULL;
        for (char* tok = strtok_r(line, ";", &save); tok && nf < 5; tok = strtok_r(NULL, ";", &save)) fields[nf++] = tok;
        if (nf < 5) continue;
        n++;
        if (n < from) continue;
        if (n >= to) break;
        /* encode source */
        uint32_t cps[64];
        size_t nc = 0;
        char* p = fields[0];
        while (*p) {
            while (*p == ' ') ++p;
            if (!*p) break;
            char* end = NULL;
            unsigned long cp = strtoul(p, &end, 16);
            cps[nc++] = (uint32_t)cp;
            p = end;
        }
        uint8_t src[256];
        size_t src_len = utf8_encode(cps, nc, src, sizeof src);
        uint8_t out[256];
        size_t written = 0;
        unicodoid_status_t st = unicodoid_normalize(sys, src, src_len, UNICODOID_NORM_NFKC, out, sizeof out, &written);
        if (st != UNICODOID_OK) {
            fprintf(stderr, "case %zu: status %d\n", n, (int)st);
            continue;
        }
        if (n % 100 == 0) fprintf(stderr, "... %zu\n", n);
    }
    fclose(f);
    fprintf(stderr, "done\n");
    return 0;
}
