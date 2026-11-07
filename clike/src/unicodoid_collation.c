#include "unicodoid/unicodoid_collation.h"
#include "unicodoid/unicodoid_codec.h"
#include "unicodoid/unicodoid_properties.h"
#include <stdlib.h>
#include <string.h>

typedef struct weight {
    uint16_t primary;
    uint16_t secondary;
    uint16_t tertiary;
} weight_t;

static uint16_t accent_secondary(uint32_t cp) {
    switch (cp) {
        case 0x0300: return 20; /* grave */
        case 0x0301: return 21; /* acute */
        case 0x0302: return 22; /* circumflex */
        case 0x0303: return 23; /* tilde */
        case 0x0308: return 24; /* diaeresis */
        case 0x030A: return 25; /* ring */
        case 0x0327: return 26; /* cedilla */
        case 0x030C: return 27; /* caron */
        case 0x0306: return 28; /* breve */
        case 0x0304: return 29; /* macron */
        case 0x0307: return 30; /* dot above */
        case 0x0328: return 31; /* ogonek */
        case 0x030B: return 32; /* double acute */
        default: return 33;
    }
}

static weight_t weight_for(uint32_t cp) {
    weight_t w = { 0, 10, 1 };

    if (cp >= 'a' && cp <= 'z') {
        w.primary = (uint16_t)(100 + (cp - 'a'));
        w.tertiary = 1;
        return w;
    }
    if (cp >= 'A' && cp <= 'Z') {
        w.primary = (uint16_t)(100 + (cp - 'A'));
        w.tertiary = 2;
        return w;
    }
    if (cp >= '0' && cp <= '9') {
        w.primary = (uint16_t)(50 + (cp - '0'));
        w.tertiary = 1;
        return w;
    }

    /* Latin letters with a single combining mark. */
    {
        static const uint32_t a_marks[6] = { 0x0300, 0x0301, 0x0302, 0x0303, 0x0308, 0x030A };
        static const uint32_t e_marks[4] = { 0x0300, 0x0301, 0x0302, 0x0308 };
        static const uint32_t o_marks[5] = { 0x0300, 0x0301, 0x0302, 0x0303, 0x0308 };
        static const uint32_t u_marks[4] = { 0x0300, 0x0301, 0x0302, 0x0308 };

        if (cp >= 0x00C0 && cp <= 0x00C5) {
            w.primary = 100; w.tertiary = 2;
            w.secondary = accent_secondary(a_marks[cp - 0x00C0]);
            return w;
        }
        if (cp >= 0x00E0 && cp <= 0x00E5) {
            w.primary = 100; w.tertiary = 1;
            w.secondary = accent_secondary(a_marks[cp - 0x00E0]);
            return w;
        }
        if (cp == 0x00C7) { w.primary = 102; w.secondary = 26; w.tertiary = 2; return w; }
        if (cp == 0x00E7) { w.primary = 102; w.secondary = 26; w.tertiary = 1; return w; }
        if (cp >= 0x00C8 && cp <= 0x00CB) {
            w.primary = 104; w.tertiary = 2;
            w.secondary = accent_secondary(e_marks[cp - 0x00C8]);
            return w;
        }
        if (cp >= 0x00E8 && cp <= 0x00EB) {
            w.primary = 104; w.tertiary = 1;
            w.secondary = accent_secondary(e_marks[cp - 0x00E8]);
            return w;
        }
        if (cp == 0x00D1) { w.primary = 113; w.secondary = 23; w.tertiary = 2; return w; }
        if (cp == 0x00F1) { w.primary = 113; w.secondary = 23; w.tertiary = 1; return w; }
        if (cp >= 0x00D2 && cp <= 0x00D6) {
            w.primary = 114; w.tertiary = 2;
            w.secondary = accent_secondary(o_marks[cp - 0x00D2]);
            return w;
        }
        if (cp >= 0x00F2 && cp <= 0x00F6) {
            w.primary = 114; w.tertiary = 1;
            w.secondary = accent_secondary(o_marks[cp - 0x00F2]);
            return w;
        }
        if (cp >= 0x00D9 && cp <= 0x00DC) {
            w.primary = 120; w.tertiary = 2;
            w.secondary = accent_secondary(u_marks[cp - 0x00D9]);
            return w;
        }
        if (cp >= 0x00F9 && cp <= 0x00FC) {
            w.primary = 120; w.tertiary = 1;
            w.secondary = accent_secondary(u_marks[cp - 0x00F9]);
            return w;
        }
    }

    if (unicodoid_unicode_is_whitespace(cp)) {
        w.primary = 1; w.secondary = 1; w.tertiary = 1;
        return w;
    }
    if (unicodoid_unicode_is_punctuation(cp)) {
        w.primary = (uint16_t)(2 + (cp & 0x3F));
        w.secondary = 2;
        w.tertiary = 1;
        return w;
    }

    w.primary = (uint16_t)(0x1000 + (cp & 0x0FFF));
    w.secondary = 10;
    w.tertiary = 1;
    return w;
}

static unicodoid_status_t decode_string(
    const unicodoid_string_t* str,
    uint32_t** out_cps,
    size_t* out_count
) {
    if (!str || !str->system || !out_cps || !out_count) return UNICODOID_ERR_INVALID_PARAM;
    if (!unicodoid_system_has_unicode_profile(str->system)) return UNICODOID_ERR_UNSUPPORTED;

    size_t capacity = str->length > 0 ? str->length : 1;
    uint32_t* cps = (uint32_t*)malloc(capacity * sizeof(uint32_t));
    if (!cps) return UNICODOID_ERR_ALLOCATION_FAILED;

    unicodoid_codec_t codec;
    unicodoid_status_t st = unicodoid_codec_init(&codec, str->system);
    if (st != UNICODOID_OK) {
        free(cps);
        return st;
    }

    size_t count = 0;
    size_t off = 0;
    while (off < str->length) {
        unicodoid_char64_t native = 0;
        size_t read = 0;
        st = unicodoid_codec_decode_char(
            &codec, str->data + off, str->length - off, &native, &read
        );
        if (st != UNICODOID_OK) break;

        uint32_t cp = 0;
        st = str->system->unicode_profile->to_unicode(
            str->system->unicode_profile, native, &cp
        );
        if (st != UNICODOID_OK) break;

        if (count == capacity) {
            capacity *= 2;
            uint32_t* p = (uint32_t*)realloc(cps, capacity * sizeof(uint32_t));
            if (!p) { st = UNICODOID_ERR_ALLOCATION_FAILED; break; }
            cps = p;
        }
        cps[count++] = cp;
        off += read;
    }

    unicodoid_codec_free(&codec);
    if (st != UNICODOID_OK) {
        free(cps);
        return st;
    }

    *out_cps = cps;
    *out_count = count;
    return UNICODOID_OK;
}

static int compare_numeric_run(
    const uint32_t* a, size_t a_len, size_t* a_pos,
    const uint32_t* b, size_t b_len, size_t* b_pos
) {
    size_t ai = *a_pos;
    size_t bi = *b_pos;

    while (ai < a_len && a[ai] >= '0' && a[ai] <= '9') ++ai;
    while (bi < b_len && b[bi] >= '0' && b[bi] <= '9') ++bi;

    size_t a_start = *a_pos;
    size_t b_start = *b_pos;
    while (a_start < ai - 1 && a[a_start] == '0') ++a_start;
    while (b_start < bi - 1 && b[b_start] == '0') ++b_start;

    size_t a_digits = ai - a_start;
    size_t b_digits = bi - b_start;
    int result = 0;
    if (a_digits != b_digits) {
        result = a_digits < b_digits ? -1 : 1;
    } else {
        for (size_t i = 0; i < a_digits; ++i) {
            if (a[a_start + i] != b[b_start + i]) {
                result = a[a_start + i] < b[b_start + i] ? -1 : 1;
                break;
            }
        }
    }

    *a_pos = ai;
    *b_pos = bi;
    return result;
}

int unicodoid_collate(
    const unicodoid_string_t* a,
    const unicodoid_string_t* b,
    const unicodoid_collation_options_t* opts
) {
    if (!a || !b) return 0;
    if (a == b) return 0;

    unicodoid_collation_options_t def = unicodoid_collation_options_default();
    if (!opts) opts = &def;

    uint32_t* ac = NULL;
    uint32_t* bc = NULL;
    size_t an = 0, bn = 0;
    if (decode_string(a, &ac, &an) != UNICODOID_OK) return 0;
    if (decode_string(b, &bc, &bn) != UNICODOID_OK) {
        free(ac);
        return 0;
    }

    int result = 0;
    size_t ai = 0, bi = 0;

    /* Primary level. */
    while (ai < an && bi < bn) {
        if (opts->numeric_ordering &&
            ac[ai] >= '0' && ac[ai] <= '9' &&
            bc[bi] >= '0' && bc[bi] <= '9') {
            result = compare_numeric_run(ac, an, &ai, bc, bn, &bi);
            if (result != 0) goto done;
            continue;
        }

        weight_t wa = weight_for(ac[ai]);
        weight_t wb = weight_for(bc[bi]);
        if (wa.primary != wb.primary) {
            result = wa.primary < wb.primary ? -1 : 1;
            goto done;
        }
        ++ai;
        ++bi;
    }
    if (ai < an || bi < bn) {
        result = ai < an ? 1 : -1;
        goto done;
    }

    /* Secondary level. */
    if (opts->strength >= UNICODOID_COLLATE_SECONDARY) {
        for (size_t i = 0; i < an; ++i) {
            weight_t wa = weight_for(ac[i]);
            weight_t wb = weight_for(bc[i]);
            if (wa.secondary != wb.secondary) {
                result = wa.secondary < wb.secondary ? -1 : 1;
                goto done;
            }
        }
    }

    /* Tertiary level. */
    if (opts->strength >= UNICODOID_COLLATE_TERTIARY) {
        for (size_t i = 0; i < an; ++i) {
            weight_t wa = weight_for(ac[i]);
            weight_t wb = weight_for(bc[i]);
            if (wa.tertiary != wb.tertiary) {
                result = wa.tertiary < wb.tertiary ? -1 : 1;
                goto done;
            }
        }
    }

    /* Quaternary/identical fall back to raw byte order. */
    if (opts->strength >= UNICODOID_COLLATE_QUATERNARY) {
        result = unicodoid_string_compare(a, b);
    }

done:
    free(ac);
    free(bc);
    return result;
}

unicodoid_status_t unicodoid_collation_sort_key(
    const unicodoid_string_t* str,
    const unicodoid_collation_options_t* opts,
    uint8_t* out_key,
    size_t out_cap,
    size_t* out_written
) {
    if (!str || !out_written) return UNICODOID_ERR_INVALID_PARAM;
    if (out_cap > 0 && !out_key) return UNICODOID_ERR_INVALID_PARAM;

    unicodoid_collation_options_t def = unicodoid_collation_options_default();
    if (!opts) opts = &def;

    uint32_t* cps = NULL;
    size_t count = 0;
    unicodoid_status_t st = decode_string(str, &cps, &count);
    if (st != UNICODOID_OK) return st;

    size_t required = count * 2 + 2;
    if (opts->strength >= UNICODOID_COLLATE_SECONDARY) required += count * 2 + 2;
    if (opts->strength >= UNICODOID_COLLATE_TERTIARY) required += count + 2;
    if (opts->strength >= UNICODOID_COLLATE_QUATERNARY) required += str->length + 2;

    if (required > out_cap) {
        *out_written = required;
        free(cps);
        return UNICODOID_ERR_BUFFER_TOO_SMALL;
    }

    size_t off = 0;
    for (size_t i = 0; i < count; ++i) {
        weight_t w = weight_for(cps[i]);
        out_key[off++] = (uint8_t)(w.primary >> 8);
        out_key[off++] = (uint8_t)(w.primary & 0xFF);
    }
    out_key[off++] = 0;
    out_key[off++] = 0;

    if (opts->strength >= UNICODOID_COLLATE_SECONDARY) {
        for (size_t i = 0; i < count; ++i) {
            weight_t w = weight_for(cps[i]);
            out_key[off++] = (uint8_t)(w.secondary >> 8);
            out_key[off++] = (uint8_t)(w.secondary & 0xFF);
        }
        out_key[off++] = 0;
        out_key[off++] = 0;
    }

    if (opts->strength >= UNICODOID_COLLATE_TERTIARY) {
        for (size_t i = 0; i < count; ++i) {
            weight_t w = weight_for(cps[i]);
            out_key[off++] = (uint8_t)w.tertiary;
        }
        out_key[off++] = 0;
        out_key[off++] = 0;
    }

    if (opts->strength >= UNICODOID_COLLATE_QUATERNARY) {
        if (str->length > 0) memcpy(out_key + off, str->data, str->length);
        off += str->length;
        out_key[off++] = 0;
        out_key[off++] = 0;
    }

    *out_written = off;
    free(cps);
    return UNICODOID_OK;
}
