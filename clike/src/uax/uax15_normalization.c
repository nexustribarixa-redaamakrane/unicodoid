#include "unicodoid/uax/uax15_normalization.h"
#include "unicodoid/unicodoid_ucd.h"
#include "unicodoid/unicodoid_normalization.h"
#include "unicodoid/systems/unicodoid_utf8.h"

#include <stdlib.h>
#include <string.h>

/* Scalar normalization reuses the byte normalizer via UTF-8 so behaviour
 * matches unicodoid_normalize exactly (Hangul, ordering, composition). */

static size_t utf8_len_scalar(uint32_t cp) {
    return cp <= 0x7F ? 1 : cp <= 0x7FF ? 2 : cp <= 0xFFFF ? 3 : 4;
}

static bool utf8_encode_scalars(const uint32_t* in, size_t length, uint8_t* out, size_t cap, size_t* need) {
    size_t bytes = 0;
    for (size_t i = 0; i < length; ++i) {
        uint32_t cp = in[i];
        if (!unicodoid_is_valid_unicode_scalar(cp)) return false;
        bytes += utf8_len_scalar(cp);
    }
    if (need) *need = bytes;
    if (!out || cap < bytes) return true;
    size_t b = 0;
    for (size_t i = 0; i < length; ++i) {
        uint32_t cp = in[i];
        if (cp <= 0x7F) out[b++] = (uint8_t)cp;
        else if (cp <= 0x7FF) { out[b++] = (uint8_t)(0xC0 | (cp >> 6)); out[b++] = (uint8_t)(0x80 | (cp & 0x3F)); }
        else if (cp <= 0xFFFF) { out[b++] = (uint8_t)(0xE0 | (cp >> 12)); out[b++] = (uint8_t)(0x80 | ((cp >> 6) & 0x3F)); out[b++] = (uint8_t)(0x80 | (cp & 0x3F)); }
        else { out[b++] = (uint8_t)(0xF0 | (cp >> 18)); out[b++] = (uint8_t)(0x80 | ((cp >> 12) & 0x3F)); out[b++] = (uint8_t)(0x80 | ((cp >> 6) & 0x3F)); out[b++] = (uint8_t)(0x80 | (cp & 0x3F)); }
    }
    return true;
}

static size_t utf8_decode_scalars(const uint8_t* bytes, size_t len, uint32_t* out, size_t cap) {
    size_t s = 0, b = 0;
    while (b < len) {
        uint8_t b0 = bytes[b];
        uint32_t cp = 0; size_t adv = 1;
        if ((b0 & 0x80) == 0) { cp = b0; adv = 1; }
        else if ((b0 & 0xE0) == 0xC0) { cp = ((b0 & 0x1F) << 6) | (bytes[b+1] & 0x3F); adv = 2; }
        else if ((b0 & 0xF0) == 0xE0) { cp = ((b0 & 0x0F) << 12) | ((bytes[b+1] & 0x3F) << 6) | (bytes[b+2] & 0x3F); adv = 3; }
        else { cp = ((b0 & 0x07) << 18) | ((bytes[b+1] & 0x3F) << 12) | ((bytes[b+2] & 0x3F) << 6) | (bytes[b+3] & 0x3F); adv = 4; }
        if (s < cap) out[s] = cp;
        ++s; b += adv;
    }
    return s;
}

bool unicodoid_uax15_has_decomposition(uint32_t cp, bool compatibility) {
    return unicodoid_ucd_has_decomposition(cp, compatibility);
}

size_t unicodoid_uax15_decomposition(uint32_t cp, bool compatibility, uint32_t* out, size_t cap) {
    return unicodoid_ucd_decomposition(cp, compatibility, out, cap);
}

size_t unicodoid_uax15_normalize(const uint32_t* in, size_t length, int form, uint32_t* out, size_t cap) {
    if (!in || length == 0) return 0;
    size_t need = 0;
    if (!utf8_encode_scalars(in, length, NULL, 0, &need)) return 0;
    uint8_t* bytes = (uint8_t*)malloc(need ? need : 1);
    if (!bytes) return 0;
    utf8_encode_scalars(in, length, bytes, need, NULL);

    unicodoid_norm_form_t f = (form == 0) ? UNICODOID_NORM_NFC :
                              (form == 1) ? UNICODOID_NORM_NFD :
                              (form == 2) ? UNICODOID_NORM_NFKC : UNICODOID_NORM_NFKD;
    const unicodoid_system_t* sys = unicodoid_system_get_utf8();
    size_t required = 0;
    unicodoid_status_t st = unicodoid_normalize(sys, bytes, need, f, NULL, 0, &required);
    if (st != UNICODOID_OK && st != UNICODOID_ERR_BUFFER_TOO_SMALL) { free(bytes); return 0; }
    uint8_t* tmp = (uint8_t*)malloc(required ? required : 1);
    if (!tmp) { free(bytes); return 0; }
    size_t written = 0;
    st = unicodoid_normalize(sys, bytes, need, f, tmp, required, &written);
    free(bytes);
    if (st != UNICODOID_OK) { free(tmp); return 0; }
    size_t total = utf8_decode_scalars(tmp, written, NULL, 0);
    if (out && cap >= total) utf8_decode_scalars(tmp, written, out, cap);
    free(tmp);
    return total;
}

bool unicodoid_uax15_is_normalized(const uint32_t* in, size_t length, int form) {
    if (!in || length == 0) return true;
    size_t need = 0;
    if (!utf8_encode_scalars(in, length, NULL, 0, &need)) return false;
    uint8_t* bytes = (uint8_t*)malloc(need ? need : 1);
    if (!bytes) return false;
    utf8_encode_scalars(in, length, bytes, need, NULL);
    unicodoid_norm_form_t f = (form == 0) ? UNICODOID_NORM_NFC :
                              (form == 1) ? UNICODOID_NORM_NFD :
                              (form == 2) ? UNICODOID_NORM_NFKC : UNICODOID_NORM_NFKD;
    bool ok = unicodoid_is_normalized(unicodoid_system_get_utf8(), bytes, need, f);
    free(bytes);
    return ok;
}
