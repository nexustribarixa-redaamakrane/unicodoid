#include "unicodoid/unicodoid.h"
#include "unicodoid/unicodoid_normalization.h"
#include "unicodoid/systems/unicodoid_utf8.h"
#include <stdio.h>
#include <string.h>

static void test_cp(uint32_t cp) {
    uint8_t src[4];
    size_t n = 0;
    if (cp < 0x80) { src[0] = (uint8_t)cp; n = 1; }
    else if (cp < 0x800) { src[0] = 0xC0 | (cp >> 6); src[1] = 0x80 | (cp & 0x3F); n = 2; }
    else if (cp < 0x10000) { src[0] = 0xE0 | (cp >> 12); src[1] = 0x80 | ((cp >> 6) & 0x3F); src[2] = 0x80 | (cp & 0x3F); n = 3; }
    else { src[0] = 0xF0 | (cp >> 18); src[1] = 0x80 | ((cp >> 12) & 0x3F); src[2] = 0x80 | ((cp >> 6) & 0x3F); src[3] = 0x80 | (cp & 0x3F); n = 4; }

    const unicodoid_system_t* sys = unicodoid_system_get_utf8();
    for (int f = 0; f < 4; ++f) {
        uint8_t out[64];
        size_t written = 0;
        unicodoid_status_t st = unicodoid_normalize(sys, src, n, (unicodoid_norm_form_t)f, out, sizeof out, &written);
        fprintf(stderr, "cp=%04X form=%d st=%d len=%zu\n", cp, f, (int)st, written);
    }
}

int main(int argc, char** argv) {
    uint32_t cps[] = { 0xFAAD, 0xFAAE, 0xFAC1, 0x3307, 0x1E14, 0x05B8, 0x05B9, 0x05B1, 0x0591, 0x05C3, 0x05B0, 0x05AC, 0x059F };
    for (size_t i = 0; i < sizeof cps / sizeof cps[0]; ++i) test_cp(cps[i]);
    return 0;
}
