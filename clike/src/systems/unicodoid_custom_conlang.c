#include "unicodoid/systems/unicodoid_custom_conlang.h"
#include <string.h>

/*
 * KupoConlang-14: An independent 14-bit constructed script encoding system.
 * Demonstrates a non-Unicode architecture with NO Unicode profile (unicode_profile = NULL),
 * custom character space (0x0000 - 0x3FFF), and a variable 1-to-2 byte representation.
 */

static unicodoid_status_t conlang_encode_char(
    const unicodoid_system_t* sys,
    void* state,
    unicodoid_char64_t ch,
    void* out_buf,
    size_t out_cap,
    size_t* out_written
) {
    (void)sys; (void)state;
    if (!out_buf || !out_written) return UNICODOID_ERR_INVALID_PARAM;
    if (ch > 0x3FFF) return UNICODOID_ERR_UNMAPPABLE_CHAR;

    uint8_t* p = (uint8_t*)out_buf;
    if (ch <= 0x7F) {
        if (out_cap < 1) return UNICODOID_ERR_BUFFER_TOO_SMALL;
        p[0] = (uint8_t)ch;
        *out_written = 1;
    } else {
        if (out_cap < 2) return UNICODOID_ERR_BUFFER_TOO_SMALL;
        p[0] = (uint8_t)(0x80 | ((ch >> 8) & 0x3F));
        p[1] = (uint8_t)(ch & 0xFF);
        *out_written = 2;
    }
    return UNICODOID_OK;
}

static unicodoid_status_t conlang_decode_char(
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

    const uint8_t* p = (const uint8_t*)in_buf;
    uint8_t b0 = p[0];

    if ((b0 & 0x80) == 0) {
        *out_ch = b0;
        *out_read = 1;
        return UNICODOID_OK;
    } else if ((b0 & 0xC0) == 0x80) {
        if (in_len < 2) return UNICODOID_ERR_INCOMPLETE_SEQUENCE;
        uint8_t b1 = p[1];
        uint32_t val = ((uint32_t)(b0 & 0x3F) << 8) | b1;
        if (val < 0x80) return UNICODOID_ERR_INVALID_CODE_UNIT; /* Non-canonical / overlong */
        *out_ch = val;
        *out_read = 2;
        return UNICODOID_OK;
    } else {
        /* Bytes starting with 11xxxxxx are invalid in KupoConlang-14 */
        return UNICODOID_ERR_INVALID_CODE_UNIT;
    }
}

static unicodoid_status_t conlang_validate(
    const unicodoid_system_t* sys,
    void* state,
    const void* in_buf,
    size_t in_len,
    size_t* out_invalid_offset
) {
    size_t off = 0;
    while (off < in_len) {
        unicodoid_char64_t ch;
        size_t read_bytes = 0;
        unicodoid_status_t st = conlang_decode_char(sys, state, (const uint8_t*)in_buf + off, in_len - off, &ch, &read_bytes);
        if (st != UNICODOID_OK) {
            if (out_invalid_offset) *out_invalid_offset = off;
            return st;
        }
        off += read_bytes;
    }
    return UNICODOID_OK;
}

static unicodoid_status_t conlang_char_count(
    const unicodoid_system_t* sys,
    void* state,
    const void* in_buf,
    size_t in_len,
    size_t* out_count
) {
    if (!in_buf || !out_count) return UNICODOID_ERR_INVALID_PARAM;
    size_t count = 0, off = 0;
    while (off < in_len) {
        unicodoid_char64_t ch;
        size_t read_bytes = 0;
        unicodoid_status_t st = conlang_decode_char(sys, state, (const uint8_t*)in_buf + off, in_len - off, &ch, &read_bytes);
        if (st != UNICODOID_OK) return st;
        off += read_bytes;
        count++;
    }
    *out_count = count;
    return UNICODOID_OK;
}

static unicodoid_status_t conlang_byte_count(
    const unicodoid_system_t* sys,
    const unicodoid_char64_t* chars,
    size_t num_chars,
    size_t* out_bytes
) {
    (void)sys;
    if (!chars || !out_bytes) return UNICODOID_ERR_INVALID_PARAM;
    size_t total = 0;
    for (size_t i = 0; i < num_chars; ++i) {
        unicodoid_char64_t ch = chars[i];
        if (ch <= 0x7F) total += 1;
        else if (ch <= 0x3FFF) total += 2;
        else return UNICODOID_ERR_UNMAPPABLE_CHAR;
    }
    *out_bytes = total;
    return UNICODOID_OK;
}

static const unicodoid_system_t s_conlang_system = {
    .meta = {
        .id = "org.unicodoid.custom.kupoconlang",
        .name = "KupoConlang-14",
        .version = "1.0.0",
        .author = "Unicodoid Conlang Working Group",
        .description = "Independent 14-bit Variable-Width Conlang Script System",
        .code_unit_bits = 8,
        .min_bytes_per_char = 1,
        .max_bytes_per_char = 2,
        .model = UNICODOID_MODEL_VARIABLE_WIDTH,
        .endianness = UNICODOID_ENDIAN_NONE,
        .compat_level = UNICODOID_COMPAT_NONE, /* Independent of Unicode */
        .max_char_val = 0x3FFF,
        .replacement_char = 0x003F
    },
    .ops = {
        .encode_char = conlang_encode_char,
        .decode_char = conlang_decode_char,
        .validate = conlang_validate,
        .char_count = conlang_char_count,
        .byte_count = conlang_byte_count
    },
    .state_size = 0,
    .state_init = NULL,
    .state_reset = NULL,
    .unicode_profile = NULL, /* No Unicode profile */
    .user_data = NULL
};

const unicodoid_system_t* unicodoid_system_get_custom_conlang(void) {
    return &s_conlang_system;
}
