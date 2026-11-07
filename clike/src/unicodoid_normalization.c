#include "unicodoid/unicodoid_normalization.h"
#include "unicodoid/unicodoid_codec.h"
#include "unicodoid/unicodoid_properties.h"
#include "unicodoid/unicodoid_ucd.h"
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------------- */
/* Scalar vector                                                             */
/* ------------------------------------------------------------------------- */

typedef struct scalar_vec {
    uint32_t* data;
    size_t    length;
    size_t    capacity;
} scalar_vec_t;

static unicodoid_status_t vec_push(scalar_vec_t* v, uint32_t cp) {
    if (v->length == v->capacity) {
        size_t cap = v->capacity ? v->capacity * 2 : 32;
        uint32_t* p = (uint32_t*)realloc(v->data, cap * sizeof(uint32_t));
        if (!p) return UNICODOID_ERR_ALLOCATION_FAILED;
        v->data = p;
        v->capacity = cap;
    }
    v->data[v->length++] = cp;
    return UNICODOID_OK;
}

static void vec_free(scalar_vec_t* v) {
    free(v->data);
    memset(v, 0, sizeof(*v));
}

typedef struct byte_vec {
    uint8_t* data;
    size_t   length;
    size_t   capacity;
} byte_vec_t;

static unicodoid_status_t bytes_reserve(byte_vec_t* v, size_t extra) {
    if (v->length + extra <= v->capacity) return UNICODOID_OK;
    size_t cap = v->capacity ? v->capacity : 64;
    while (cap < v->length + extra) {
        if (cap > (SIZE_MAX / 2)) return UNICODOID_ERR_OVERFLOW;
        cap *= 2;
    }
    uint8_t* p = (uint8_t*)realloc(v->data, cap);
    if (!p) return UNICODOID_ERR_ALLOCATION_FAILED;
    v->data = p;
    v->capacity = cap;
    return UNICODOID_OK;
}

static void bytes_free(byte_vec_t* v) {
    free(v->data);
    memset(v, 0, sizeof(*v));
}

/* ------------------------------------------------------------------------- */
/* Decomposition                                                              */
/* ------------------------------------------------------------------------- */

#define HANGUL_SBASE  0xAC00u
#define HANGUL_LBASE  0x1100u
#define HANGUL_VBASE  0x1161u
#define HANGUL_TBASE  0x11A7u
#define HANGUL_LCOUNT 19u
#define HANGUL_VCOUNT 21u
#define HANGUL_TCOUNT 28u
#define HANGUL_NCOUNT (HANGUL_VCOUNT * HANGUL_TCOUNT)
#define HANGUL_SCOUNT (HANGUL_LCOUNT * HANGUL_NCOUNT)

static void decompose_scalar(uint32_t cp, bool compatibility, scalar_vec_t* out) {
    if (cp >= HANGUL_SBASE && cp < HANGUL_SBASE + HANGUL_SCOUNT) {
        uint32_t sindex = cp - HANGUL_SBASE;
        uint32_t l = HANGUL_LBASE + (sindex / HANGUL_NCOUNT);
        uint32_t v = HANGUL_VBASE + ((sindex % HANGUL_NCOUNT) / HANGUL_TCOUNT);
        uint32_t t = HANGUL_TBASE + (sindex % HANGUL_TCOUNT);
        vec_push(out, l);
        vec_push(out, v);
        if (t != HANGUL_TBASE) vec_push(out, t);
        return;
    }

    uint32_t parts[32];
    size_t part_len = 0;
    if (unicodoid_ucd_has_decomposition(cp, compatibility)) {
        part_len = unicodoid_ucd_decomposition(cp, compatibility, parts, 32);
        if (part_len > 0 && part_len <= 32) {
            for (size_t i = 0; i < part_len; ++i) {
                decompose_scalar(parts[i], compatibility, out);
            }
            return;
        }
    }

    vec_push(out, cp);
}

static bool compose_pair(uint32_t starter, uint32_t ch, uint32_t* out) {
    /* Hangul LV composition. */
    if (starter >= HANGUL_LBASE && starter < HANGUL_LBASE + HANGUL_LCOUNT &&
        ch >= HANGUL_VBASE && ch < HANGUL_VBASE + HANGUL_VCOUNT) {
        uint32_t lindex = starter - HANGUL_LBASE;
        uint32_t vindex = ch - HANGUL_VBASE;
        *out = HANGUL_SBASE + (lindex * HANGUL_VCOUNT + vindex) * HANGUL_TCOUNT;
        return true;
    }

    /* Hangul LVT composition. */
    if (starter >= HANGUL_SBASE && starter < HANGUL_SBASE + HANGUL_SCOUNT &&
        ((starter - HANGUL_SBASE) % HANGUL_TCOUNT) == 0 &&
        ch > HANGUL_TBASE && ch < HANGUL_TBASE + HANGUL_TCOUNT) {
        *out = starter + (ch - HANGUL_TBASE);
        return true;
    }

    uint32_t composed = 0;
    if (unicodoid_ucd_compose_pair(starter, ch, &composed)) {
        *out = composed;
        return true;
    }
    return false;
}

static void canonical_order(scalar_vec_t* seq) {
    for (size_t i = 1; i < seq->length; ++i) {
        uint32_t cp = seq->data[i];
        int cc = unicodoid_unicode_combining_class(cp);
        if (cc == 0) continue;

        size_t j = i;
        while (j > 0) {
            int prev = unicodoid_unicode_combining_class(seq->data[j - 1]);
            if (prev == 0 || prev <= cc) break;
            seq->data[j] = seq->data[j - 1];
            --j;
        }
        seq->data[j] = cp;
    }
}

static unicodoid_status_t compose_scalars(scalar_vec_t* seq, scalar_vec_t* out) {
    if (seq->length == 0) return UNICODOID_OK;

    unicodoid_status_t st = vec_push(out, seq->data[0]);
    if (st != UNICODOID_OK) return st;

    size_t starter_pos = 0;
    uint32_t starter = seq->data[0];
    int last_cc = 0;

    for (size_t i = 1; i < seq->length; ++i) {
        uint32_t cp = seq->data[i];
        int cc = unicodoid_unicode_combining_class(cp);
        uint32_t composed = 0;

        if ((last_cc < cc || last_cc == 0) && compose_pair(starter, cp, &composed)) {
            out->data[starter_pos] = composed;
            starter = composed;
            continue;
        }

        if (cc == 0) {
            starter_pos = out->length;
            starter = cp;
        }
        st = vec_push(out, cp);
        if (st != UNICODOID_OK) return st;
        last_cc = cc;
    }
    return UNICODOID_OK;
}

/* ------------------------------------------------------------------------- */
/* Public API                                                                */
/* ------------------------------------------------------------------------- */

static unicodoid_status_t decode_scalars(
    const unicodoid_system_t* sys,
    const void* in_buf,
    size_t in_len,
    scalar_vec_t* out
) {
    unicodoid_codec_t codec;
    unicodoid_status_t st = unicodoid_codec_init(&codec, sys);
    if (st != UNICODOID_OK) return st;

    size_t off = 0;
    while (off < in_len) {
        unicodoid_char64_t native = 0;
        size_t read = 0;
        st = unicodoid_codec_decode_char(
            &codec, (const uint8_t*)in_buf + off, in_len - off, &native, &read
        );
        if (st != UNICODOID_OK) { unicodoid_codec_free(&codec); return st; }

        uint32_t cp = 0;
        st = sys->unicode_profile->to_unicode(sys->unicode_profile, native, &cp);
        if (st != UNICODOID_OK) { unicodoid_codec_free(&codec); return st; }
        st = vec_push(out, cp);
        if (st != UNICODOID_OK) { unicodoid_codec_free(&codec); return st; }
        off += read;
    }

    unicodoid_codec_free(&codec);
    return UNICODOID_OK;
}

static unicodoid_status_t encode_scalars(
    const unicodoid_system_t* sys,
    const scalar_vec_t* scalars,
    byte_vec_t* out
) {
    unicodoid_codec_t codec;
    unicodoid_status_t st = unicodoid_codec_init(&codec, sys);
    if (st != UNICODOID_OK) return st;

    for (size_t i = 0; i < scalars->length; ++i) {
        unicodoid_char64_t native = 0;
        st = sys->unicode_profile->from_unicode(sys->unicode_profile, scalars->data[i], &native);
        if (st != UNICODOID_OK) { unicodoid_codec_free(&codec); return st; }

        size_t reserve = sys->meta.max_bytes_per_char ? sys->meta.max_bytes_per_char : 8;
        st = bytes_reserve(out, reserve);
        if (st != UNICODOID_OK) { unicodoid_codec_free(&codec); return st; }

        size_t written = 0;
        st = unicodoid_codec_encode_char(
            &codec, native, out->data + out->length, out->capacity - out->length, &written
        );
        if (st != UNICODOID_OK) { unicodoid_codec_free(&codec); return st; }
        out->length += written;
    }

    unicodoid_codec_free(&codec);
    return UNICODOID_OK;
}

unicodoid_status_t unicodoid_normalize(
    const unicodoid_system_t* sys,
    const void* in_buf,
    size_t in_len,
    unicodoid_norm_form_t form,
    void* out_buf,
    size_t out_cap,
    size_t* out_written
) {
    if (!sys || !out_written) return UNICODOID_ERR_INVALID_PARAM;
    if (in_len > 0 && !in_buf) return UNICODOID_ERR_INVALID_PARAM;
    if (out_cap > 0 && !out_buf) return UNICODOID_ERR_INVALID_PARAM;

    *out_written = 0;
    if (form == UNICODOID_NORM_CUSTOM) return UNICODOID_ERR_UNSUPPORTED;
    if (!unicodoid_system_has_unicode_profile(sys)) return UNICODOID_ERR_UNSUPPORTED;
    if (in_len == 0) return UNICODOID_OK;

    bool compatibility = (form == UNICODOID_NORM_NFKC || form == UNICODOID_NORM_NFKD);
    bool compose = (form == UNICODOID_NORM_NFC || form == UNICODOID_NORM_NFKC);

    scalar_vec_t decoded = { 0 };
    scalar_vec_t decomposed = { 0 };
    scalar_vec_t normalized = { 0 };
    byte_vec_t encoded = { 0 };

    unicodoid_status_t st = decode_scalars(sys, in_buf, in_len, &decoded);
    if (st != UNICODOID_OK) goto done;

    for (size_t i = 0; i < decoded.length; ++i) {
        decompose_scalar(decoded.data[i], compatibility, &decomposed);
    }
    canonical_order(&decomposed);

    if (compose) {
        st = compose_scalars(&decomposed, &normalized);
        if (st != UNICODOID_OK) goto done;
    }

    st = encode_scalars(sys, compose ? &normalized : &decomposed, &encoded);
    if (st != UNICODOID_OK) goto done;

    *out_written = encoded.length;
    if (encoded.length > out_cap || (encoded.length > 0 && !out_buf)) {
        st = UNICODOID_ERR_BUFFER_TOO_SMALL;
        goto done;
    }
    if (encoded.length > 0) memcpy(out_buf, encoded.data, encoded.length);
    st = UNICODOID_OK;

done:
    vec_free(&decoded);
    vec_free(&decomposed);
    vec_free(&normalized);
    bytes_free(&encoded);
    return st;
}

unicodoid_status_t unicodoid_normalize_string(
    const unicodoid_string_t* src,
    unicodoid_norm_form_t form,
    unicodoid_string_t* out
) {
    if (!src || !out) return UNICODOID_ERR_INVALID_PARAM;

    size_t required = 0;
    unicodoid_status_t st = unicodoid_normalize(
        src->system, src->data, src->length, form, NULL, 0, &required
    );
    if (st != UNICODOID_OK && st != UNICODOID_ERR_BUFFER_TOO_SMALL) return st;

    st = unicodoid_string_init(out, src->system, required > 0 ? required : 16);
    if (st != UNICODOID_OK) return st;

    size_t written = 0;
    st = unicodoid_normalize(
        src->system, src->data, src->length, form,
        out->data, out->capacity, &written
    );
    if (st != UNICODOID_OK) { unicodoid_string_free(out); return st; }
    out->length = written;
    return UNICODOID_OK;
}

bool unicodoid_is_normalized(
    const unicodoid_system_t* sys,
    const void* in_buf,
    size_t in_len,
    unicodoid_norm_form_t form
) {
    if (!sys) return false;
    if (in_len == 0) return true;

    size_t required = 0;
    unicodoid_status_t st = unicodoid_normalize(
        sys, in_buf, in_len, form, NULL, 0, &required
    );
    if (st != UNICODOID_OK && st != UNICODOID_ERR_BUFFER_TOO_SMALL) return false;
    if (required != in_len) return false;

    uint8_t* temp = (uint8_t*)malloc(required);
    if (!temp) return false;
    size_t written = 0;
    st = unicodoid_normalize(sys, in_buf, in_len, form, temp, required, &written);
    bool equal = (st == UNICODOID_OK && written == in_len && memcmp(in_buf, temp, in_len) == 0);
    free(temp);
    return equal;
}
