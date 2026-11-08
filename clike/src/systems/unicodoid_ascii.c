#include "unicodoid/systems/unicodoid_ascii.h"
#include <string.h>

static unicodoid_status_t ascii_to_unicode(
    const unicodoid_unicode_profile_t* profile,
    unicodoid_char64_t native_val,
    uint32_t* out_unicode
) {
    (void)profile;
    if (!out_unicode) return UNICODOID_ERR_INVALID_PARAM;
    if (native_val > 0x7F) return UNICODOID_ERR_UNMAPPABLE_CHAR;
    *out_unicode = (uint32_t)native_val;
    return UNICODOID_OK;
}

static unicodoid_status_t ascii_from_unicode(
    const unicodoid_unicode_profile_t* profile,
    uint32_t unicode_val,
    unicodoid_char64_t* out_native
) {
    (void)profile;
    if (!out_native) return UNICODOID_ERR_INVALID_PARAM;
    if (unicode_val > 0x7F) return UNICODOID_ERR_UNMAPPABLE_CHAR;
    *out_native = (unicodoid_char64_t)unicode_val;
    return UNICODOID_OK;
}

static const unicodoid_unicode_profile_t s_ascii_unicode_profile = {
    .unicode_major = 18,
    .unicode_minor = 0,
    .unicode_patch = 0,
    .supported_uax = UNICODOID_UAX_14_LINEBREAK | UNICODOID_UAX_29_TEXT_SEGMENT | UNICODOID_UAX_44_UCD,
    .supported_uts = UNICODOID_UTS_10_UCA,
    .supported_utr = UNICODOID_UTR_17_ENCODING_MODEL,
    .context = NULL,
    .to_unicode = ascii_to_unicode,
    .from_unicode = ascii_from_unicode
};

static unicodoid_status_t ascii_encode_char(
    const unicodoid_system_t* sys,
    void* state,
    unicodoid_char64_t ch,
    void* out_buf,
    size_t out_cap,
    size_t* out_written
) {
    (void)sys; (void)state;
    if (!out_buf || !out_written) return UNICODOID_ERR_INVALID_PARAM;
    if (ch > 0x7F) return UNICODOID_ERR_UNMAPPABLE_CHAR;
    if (out_cap < 1) return UNICODOID_ERR_BUFFER_TOO_SMALL;

    *((uint8_t*)out_buf) = (uint8_t)ch;
    *out_written = 1;
    return UNICODOID_OK;
}

static unicodoid_status_t ascii_decode_char(
    const unicodoid_system_t* sys,
    void* state,
    const void* in_buf,
    size_t in_len,
    unicodoid_char64_t* out_ch,
    size_t* out_read
) {
    (void)sys; (void)state;
    if (!in_buf || !out_ch || !out_read) return UNICODOID_ERR_INVALID_PARAM;
    if (in_len < 1) return UNICODOID_ERR_INCOMPLETE_SEQUENCE;

    uint8_t b = *((const uint8_t*)in_buf);
    if (b > 0x7F) return UNICODOID_ERR_INVALID_CODE_UNIT;

    *out_ch = b;
    *out_read = 1;
    return UNICODOID_OK;
}

static unicodoid_status_t ascii_validate(
    const unicodoid_system_t* sys,
    void* state,
    const void* in_buf,
    size_t in_len,
    size_t* out_invalid_offset
) {
    (void)sys; (void)state;
    if (in_len > 0 && !in_buf) return UNICODOID_ERR_INVALID_PARAM;
    const uint8_t* p = (const uint8_t*)in_buf;
    for (size_t i = 0; i < in_len; ++i) {
        if (p[i] > 0x7F) {
            if (out_invalid_offset) *out_invalid_offset = i;
            return UNICODOID_ERR_INVALID_CODE_UNIT;
        }
    }
    return UNICODOID_OK;
}

static unicodoid_status_t ascii_char_count(
    const unicodoid_system_t* sys,
    void* state,
    const void* in_buf,
    size_t in_len,
    size_t* out_count
) {
    unicodoid_status_t st = ascii_validate(sys, state, in_buf, in_len, NULL);
    if (st != UNICODOID_OK) return st;
    if (out_count) *out_count = in_len;
    return UNICODOID_OK;
}

static unicodoid_status_t ascii_byte_count(
    const unicodoid_system_t* sys,
    const unicodoid_char64_t* chars,
    size_t num_chars,
    size_t* out_bytes
) {
    (void)sys;
    if (!out_bytes) return UNICODOID_ERR_INVALID_PARAM;
    if (!chars && num_chars > 0) return UNICODOID_ERR_INVALID_PARAM;
    for (size_t i = 0; i < num_chars; ++i) {
        if (chars[i] > 0x7F) return UNICODOID_ERR_UNMAPPABLE_CHAR;
    }
    *out_bytes = num_chars;
    return UNICODOID_OK;
}

static const unicodoid_system_t s_ascii_system = {
    .meta = {
        .id = "org.unicodoid.ascii",
        .name = "ASCII-7",
        .version = "1.0.0",
        .author = "ANSI / ASA",
        .description = "Standard 7-bit American Standard Code for Information Interchange",
        .code_unit_bits = 8,
        .min_bytes_per_char = 1,
        .max_bytes_per_char = 1,
        .model = UNICODOID_MODEL_FIXED_WIDTH,
        .endianness = UNICODOID_ENDIAN_NONE,
        .compat_level = UNICODOID_COMPAT_PARTIAL,
        .max_char_val = 0x7F,
        .replacement_char = '?'
    },
    .ops = {
        .encode_char = ascii_encode_char,
        .decode_char = ascii_decode_char,
        .validate = ascii_validate,
        .char_count = ascii_char_count,
        .byte_count = ascii_byte_count
    },
    .state_size = 0,
    .state_init = NULL,
    .state_reset = NULL,
    .unicode_profile = &s_ascii_unicode_profile,
    .user_data = NULL
};

const unicodoid_system_t* unicodoid_system_get_ascii(void) {
    return &s_ascii_system;
}
