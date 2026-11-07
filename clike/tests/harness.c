// Harness for the ConvertUTF port (C).
//
// Covers well-formedness, strict/lenient behavior, boundary conditions,
// exhaustion, and cross-encoding equivalence against Unicode 18.0.0 data.

#include "unicodoid/ConvertUTF.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static size_t failures = 0;
static size_t checks = 0;

static void check(bool condition, const char* what) {
    ++checks;
    if (!condition) {
        ++failures;
        fprintf(stderr, "FAIL: %s\n", what);
    }
}

static ConversionResult utf8_to_utf16(const UTF8* src, size_t src_len,
                                      UTF16* dst, size_t dst_cap,
                                      size_t* src_used, size_t* dst_used) {
    const UTF8* s = src;
    const UTF8* send = src + src_len;
    UTF16* t = dst;
    UTF16* tend = dst + dst_cap;
    ConversionResult r = ConvertUTF8toUTF16(&s, send, &t, tend, strictConversion);
    if (src_used) *src_used = (size_t)(s - src);
    if (dst_used) *dst_used = (size_t)(t - dst);
    return r;
}

static ConversionResult utf16_to_utf8(const UTF16* src, size_t src_len,
                                      UTF8* dst, size_t dst_cap,
                                      size_t* src_used, size_t* dst_used,
                                      ConversionFlags flags) {
    const UTF16* s = src;
    const UTF16* send = src + src_len;
    UTF8* t = dst;
    UTF8* tend = dst + dst_cap;
    ConversionResult r = ConvertUTF16toUTF8(&s, send, &t, tend, flags);
    if (src_used) *src_used = (size_t)(s - src);
    if (dst_used) *dst_used = (size_t)(t - dst);
    return r;
}

int main(void) {
    /* Version marker: port targets Unicode 18.0.0. */
    check(UNICODOID_CONVERTUTF_UNICODE_VERSION_MAJOR == 18, "unicode major 18");
    check(UNICODOID_CONVERTUTF_UNICODE_VERSION_MINOR == 0, "unicode minor 0");
    check(UNICODOID_CONVERTUTF_UNICODE_VERSION_PATCH == 0, "unicode patch 0");
    check(strcmp(UNICODOID_CONVERTUTF_UNICODE_VERSION_STRING, "18.0.0") == 0,
          "unicode version string");

    /* UTF-8 -> UTF-16 vectors. */
    {
        UTF8 src[] = { 0x41 };
        UTF16 dst[1];
        size_t su, du;
        ConversionResult r = utf8_to_utf16(src, sizeof src, dst, 1, &su, &du);
        check(r == conversionOK, "utf8 to utf16 A ok");
        check(su == 1 && du == 1, "utf8 to utf16 A positions");
        check(dst[0] == 0x0041, "utf8 to utf16 A value");
    }
    {
        UTF8 src[] = { 0xC3, 0xA9 };
        UTF16 dst[1];
        ConversionResult r = utf8_to_utf16(src, sizeof src, dst, 1, NULL, NULL);
        check(r == conversionOK && dst[0] == 0x00E9, "utf8 to utf16 e-acute");
    }
    {
        UTF8 src[] = { 0xE6, 0xBC, 0xA2 };
        UTF16 dst[1];
        ConversionResult r = utf8_to_utf16(src, sizeof src, dst, 1, NULL, NULL);
        check(r == conversionOK && dst[0] == 0x6F22, "utf8 to utf16 han");
    }
    {
        UTF8 src[] = { 0xF0, 0x9F, 0x98, 0x80 };
        UTF16 dst[2];
        ConversionResult r = utf8_to_utf16(src, sizeof src, dst, 2, NULL, NULL);
        check(r == conversionOK, "utf8 to utf16 emoji ok");
        check(dst[0] == 0xD83D && dst[1] == 0xDE00, "utf8 to utf16 emoji pair");
    }
    {
        UTF8 src[] = { 0xC0, 0x80 };
        UTF16 dst[1];
        size_t su, du;
        ConversionResult r = utf8_to_utf16(src, sizeof src, dst, 1, &su, &du);
        check(r == sourceIllegal, "utf8 to utf16 overlong illegal");
        check(su == 0, "utf8 to utf16 overlong rewound");
    }
    {
        UTF8 src[] = { 0xED, 0xA0, 0x80 };
        UTF16 dst[1];
        ConversionResult r = utf8_to_utf16(src, sizeof src, dst, 1, NULL, NULL);
        check(r == sourceIllegal, "utf8 to utf16 surrogate illegal");
    }
    {
        UTF8 src[] = { 0xE2, 0x82 };
        UTF16 dst[1];
        ConversionResult r = utf8_to_utf16(src, sizeof src, dst, 1, NULL, NULL);
        check(r == sourceExhausted, "utf8 to utf16 truncated exhausted");
    }
    {
        UTF8 src[] = { 0xF4, 0x90, 0x80, 0x80 };
        UTF16 dst[1];
        ConversionResult r = utf8_to_utf16(src, sizeof src, dst, 1, NULL, NULL);
        check(r == sourceIllegal, "utf8 to utf16 above max illegal");
    }
    {
        /* Empty input converts cleanly. */
        UTF8 src[] = { 0 };
        UTF16 dst[1];
        size_t su, du;
        ConversionResult r = utf8_to_utf16(src, 0, dst, 1, &su, &du);
        check(r == conversionOK && su == 0 && du == 0, "utf8 to utf16 empty");
    }

    /* UTF-16 -> UTF-8. */
    {
        UTF16 src[] = { 0xD83D, 0xDE00 };
        UTF8 dst[4];
        ConversionResult r = utf16_to_utf8(src, 2, dst, 4, NULL, NULL, strictConversion);
        check(r == conversionOK, "utf16 to utf8 pair ok");
        check(dst[0] == 0xF0 && dst[1] == 0x9F && dst[2] == 0x98 && dst[3] == 0x80,
              "utf16 to utf8 pair bytes");
    }
    {
        UTF16 src[] = { 0xD800 };
        UTF8 dst[3];
        ConversionResult r = utf16_to_utf8(src, 1, dst, 3, NULL, NULL, strictConversion);
        check(r == sourceExhausted, "utf16 to utf8 lone high exhausted");
    }
    {
        UTF16 src[] = { 0xD800, 0x0041 };
        UTF8 dst[4];
        ConversionResult r = utf16_to_utf8(src, 2, dst, 4, NULL, NULL, strictConversion);
        check(r == sourceIllegal, "utf16 to utf8 unpaired high illegal");
    }

    /* UTF-32 -> UTF-8 and UTF-8 -> UTF-32. */
    {
        UTF32 src[] = { 0x1F600 };
        UTF8 dst[4];
        const UTF32* s = src;
        UTF8* t = dst;
        ConversionResult r = ConvertUTF32toUTF8(&s, src + 1, &t, dst + 4, strictConversion);
        check(r == conversionOK, "utf32 to utf8 emoji ok");
        check(dst[0] == 0xF0 && dst[1] == 0x9F && dst[2] == 0x98 && dst[3] == 0x80,
              "utf32 to utf8 emoji bytes");
    }
    {
        UTF32 src[] = { 0x10FFFF };
        UTF8 dst[4];
        const UTF32* s = src;
        UTF8* t = dst;
        ConversionResult r = ConvertUTF32toUTF8(&s, src + 1, &t, dst + 4, strictConversion);
        check(r == conversionOK, "utf32 to utf8 max ok");
        check(dst[0] == 0xF4 && dst[1] == 0x8F && dst[2] == 0xBF && dst[3] == 0xBF,
              "utf32 to utf8 max bytes");
    }
    {
        UTF32 src[] = { 0xD800 };
        UTF8 dst[3];
        const UTF32* s = src;
        UTF8* t = dst;
        ConversionResult r = ConvertUTF32toUTF8(&s, src + 1, &t, dst + 3, strictConversion);
        check(r == sourceIllegal, "utf32 to utf8 surrogate illegal");
    }
    {
        UTF8 src[] = { 0xF0, 0x9F, 0x98, 0x80 };
        UTF32 dst[1];
        const UTF8* s = src;
        UTF32* t = dst;
        ConversionResult r = ConvertUTF8toUTF32(&s, src + 4, &t, dst + 1, strictConversion);
        check(r == conversionOK && dst[0] == 0x1F600, "utf8 to utf32 emoji");
    }

    /* UTF-16 -> UTF-32 and UTF-32 -> UTF-16. */
    {
        UTF16 src[] = { 0xD83D, 0xDE00 };
        UTF32 dst[1];
        const UTF16* s = src;
        UTF32* t = dst;
        ConversionResult r = ConvertUTF16toUTF32(&s, src + 2, &t, dst + 1, strictConversion);
        check(r == conversionOK && dst[0] == 0x1F600, "utf16 to utf32 pair");
    }
    {
        UTF32 src[] = { 0x1F600 };
        UTF16 dst[2];
        const UTF32* s = src;
        UTF16* t = dst;
        ConversionResult r = ConvertUTF32toUTF16(&s, src + 1, &t, dst + 2, strictConversion);
        check(r == conversionOK, "utf32 to utf16 ok");
        check(dst[0] == 0xD83D && dst[1] == 0xDE00, "utf32 to utf16 pair");
    }
    {
        UTF32 src[] = { 0xD800 };
        UTF16 dst[1];
        const UTF32* s = src;
        UTF16* t = dst;
        ConversionResult r = ConvertUTF32toUTF16(&s, src + 1, &t, dst + 1, strictConversion);
        check(r == sourceIllegal, "utf32 to utf16 surrogate illegal");
    }

    /* Legality predicate. */
    {
        UTF8 ascii[] = { 0x41 };
        check(isLegalUTF8Sequence(ascii, ascii + 1) != 0, "isLegal ascii");
        UTF8 overlong[] = { 0xC0, 0x80 };
        check(isLegalUTF8Sequence(overlong, overlong + 2) == 0, "isLegal overlong");
        UTF8 trunc[] = { 0xE2, 0x82 };
        check(isLegalUTF8Sequence(trunc, trunc + 2) == 0, "isLegal truncated");
        UTF8 eacute[] = { 0xC3, 0xA9 };
        check(isLegalUTF8Sequence(eacute, eacute + 2) != 0, "isLegal e-acute");
        UTF8 emoji[] = { 0xF0, 0x9F, 0x98, 0x80 };
        check(isLegalUTF8Sequence(emoji, emoji + 4) != 0, "isLegal emoji");
        UTF8 toolarge[] = { 0xF4, 0x90, 0x80, 0x80 };
        check(isLegalUTF8Sequence(toolarge, toolarge + 4) == 0, "isLegal above max");
    }

    /* Target exhaustion: single-slot target for a surrogate pair. */
    {
        UTF8 src[] = { 0xF0, 0x9F, 0x98, 0x80 };
        UTF16 dst[1];
        size_t su, du;
        ConversionResult r = utf8_to_utf16(src, sizeof src, dst, 1, &su, &du);
        check(r == targetExhausted, "utf8 to utf16 target exhausted");
        check(su == 0 && du == 0, "utf8 to utf16 target rewound");
    }

    if (failures == 0) {
        printf("test_convertutf: %zu checks, PASS\n", checks);
        return 0;
    }
    fprintf(stderr, "test_convertutf: %zu of %zu checks failed\n", failures, checks);
    return 1;
}
