#include "unicodoid/systems/unicodoid_utf32.h"

static unicodoid_status_t utf32_to_unicode(
    const unicodoid_unicode_profile_t* profile,
    unicodoid_char64_t native_val,
    uint32_t* out_unicode
) {
    (void)profile;
    if (!out_unicode) return UNICODOID_ERR_INVALID_PARAM;
    if (!unicodoid_is_valid_unicode_scalar((uint32_t)native_val)) {
        return UNICODOID_ERR_UNMAPPABLE_CHAR;
    }
    *out_unicode = (uint32_t)native_val;
    return UNICODOID_OK;
}

static unicodoid_status_t utf32_from_unicode(
    const unicodoid_unicode_profile_t* profile,
    uint32_t unicode_val,
    unicodoid_char64_t* out_native
) {
    (void)profile;
    if (!out_native) return UNICODOID_ERR_INVALID_PARAM;
    if (!unicodoid_is_valid_unicode_scalar(unicode_val)) return UNICODOID_ERR_INVALID_PARAM;
    *out_native = (unicodoid_char64_t)unicode_val;
    return UNICODOID_OK;
}

static const unicodoid_unicode_profile_t s_utf32_unicode_profile = {
    .unicode_major = 18,
    .unicode_minor = 0,
    .unicode_patch = 0,
    .supported_uax = UNICODOID_UAX_9_BIDI | UNICODOID_UAX_11_EAST_ASIAN_W |
                     UNICODOID_UAX_14_LINEBREAK | UNICODOID_UAX_15_NORMALIZATION |
                     UNICODOID_UAX_29_TEXT_SEGMENT | UNICODOID_UAX_44_UCD,
    .supported_uts = UNICODOID_UTS_10_UCA | UNICODOID_UTS_18_REGEX |
                     UNICODOID_UTS_35_LDML | UNICODOID_UTS_39_SECURITY,
    .supported_utr = UNICODOID_UTR_17_ENCODING_MODEL,
    .context = NULL,
    .to_unicode = utf32_to_unicode,
    .from_unicode = utf32_from_unicode
};

static inline void write_u32(uint8_t* p, uint32_t val, bool is_le) {
    if (is_le) {
        p[0] = (uint8_t)(val & 0xFF);
        p[1] = (uint8_t)((val >> 8) & 0xFF);
        p[2] = (uint8_t)((val >> 16) & 0xFF);
        p[3] = (uint8_t)((val >> 24) & 0xFF);
    } else {
        p[0] = (uint8_t)((val >> 24) & 0xFF);
        p[1] = (uint8_t)((val >> 16) & 0xFF);
        p[2] = (uint8_t)((val >> 8) & 0xFF);
        p[3] = (uint8_t)(val & 0xFF);
    }
}

static inline uint32_t read_u32(const uint8_t* p, bool is_le) {
    if (is_le) {
        return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
               ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
    }
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

static unicodoid_status_t utf32_encode_internal(
    unicodoid_char64_t ch, void* out_buf, size_t out_cap,
    size_t* out_written, bool is_le
) {
    if (!out_buf || !out_written) return UNICODOID_ERR_INVALID_PARAM;
    if (!unicodoid_is_valid_unicode_scalar((uint32_t)ch)) return UNICODOID_ERR_UNMAPPABLE_CHAR;
    if (out_cap < 4) return UNICODOID_ERR_BUFFER_TOO_SMALL;

    write_u32((uint8_t*)out_buf, (uint32_t)ch, is_le);
    *out_written = 4;
    return UNICODOID_OK;
}

static unicodoid_status_t utf32_decode_internal(
    const void* in_buf, size_t in_len, unicodoid_char64_t* out_ch,
    size_t* out_read, bool is_le
) {
    if (!in_buf || !out_ch || !out_read) return UNICODOID_ERR_INVALID_PARAM;
    if (in_len < 4) return UNICODOID_ERR_INCOMPLETE_SEQUENCE;

    uint32_t val = read_u32((const uint8_t*)in_buf, is_le);
    if (!unicodoid_is_valid_unicode_scalar(val)) return UNICODOID_ERR_INVALID_CODE_UNIT;
    *out_ch = val;
    *out_read = 4;
    return UNICODOID_OK;
}

static unicodoid_status_t utf32le_encode_char(
    const unicodoid_system_t* sys, void* state, unicodoid_char64_t ch,
    void* out, size_t cap, size_t* written
) {
    (void)sys; (void)state;
    return utf32_encode_internal(ch, out, cap, written, true);
}

static unicodoid_status_t utf32le_decode_char(
    const unicodoid_system_t* sys, void* state, const void* in, size_t len,
    unicodoid_char64_t* ch, size_t* read
) {
    (void)sys; (void)state;
    return utf32_decode_internal(in, len, ch, read, true);
}

static unicodoid_status_t utf32_validate(
    const unicodoid_system_t* sys, void* state, const void* in, size_t len,
    size_t* invalid
) {
    if (len % 4 != 0) {
        if (invalid) *invalid = len - (len % 4);
        return UNICODOID_ERR_INCOMPLETE_SEQUENCE;
    }
    size_t off = 0;
    while (off < len) {
        unicodoid_char64_t ch;
        size_t read = 0;
        unicodoid_status_t st = sys->ops.decode_char(
            sys, state, (const uint8_t*)in + off, len - off, &ch, &read
        );
        if (st != UNICODOID_OK) {
            if (invalid) *invalid = off;
            return st;
        }
        off += read;
    }
    return UNICODOID_OK;
}

static unicodoid_status_t utf32_char_count(
    const unicodoid_system_t* sys, void* state, const void* in, size_t len,
    size_t* out_count
) {
    (void)sys; (void)state;
    if (!in || !out_count) return UNICODOID_ERR_INVALID_PARAM;
    if (len % 4 != 0) return UNICODOID_ERR_INCOMPLETE_SEQUENCE;
    *out_count = len / 4;
    return UNICODOID_OK;
}

static unicodoid_status_t utf32_byte_count(
    const unicodoid_system_t* sys, const unicodoid_char64_t* chars,
    size_t num_chars, size_t* out_bytes
) {
    (void)sys;
    if (!chars || !out_bytes) return UNICODOID_ERR_INVALID_PARAM;
    for (size_t i = 0; i < num_chars; ++i) {
        if (!unicodoid_is_valid_unicode_scalar((uint32_t)chars[i])) {
            return UNICODOID_ERR_UNMAPPABLE_CHAR;
        }
    }
    *out_bytes = num_chars * 4;
    return UNICODOID_OK;
}

static unicodoid_status_t utf32be_encode_char(
    const unicodoid_system_t* sys, void* state, unicodoid_char64_t ch,
    void* out, size_t cap, size_t* written
) {
    (void)sys; (void)state;
    return utf32_encode_internal(ch, out, cap, written, false);
}

static unicodoid_status_t utf32be_decode_char(
    const unicodoid_system_t* sys, void* state, const void* in, size_t len,
    unicodoid_char64_t* ch, size_t* read
) {
    (void)sys; (void)state;
    return utf32_decode_internal(in, len, ch, read, false);
}

static const unicodoid_system_t s_utf32le_system = {
    .meta = {
        .id = "org.unicode.utf32le",
        .name = "UTF-32LE",
        .version = "1.0.0",
        .author = "Unicode Consortium",
        .description = "Fixed-width 32-bit Unicode Transformation Format (Little Endian)",
        .code_unit_bits = 32,
        .min_bytes_per_char = 4,
        .max_bytes_per_char = 4,
        .model = UNICODOID_MODEL_FIXED_WIDTH,
        .endianness = UNICODOID_ENDIAN_LITTLE,
        .compat_level = UNICODOID_COMPAT_FULL,
        .max_char_val = UNICODOID_UNICODE_MAX_SCALAR,
        .replacement_char = UNICODOID_UNICODE_REPLACEMENT
    },
    .ops = {
        .encode_char = utf32le_encode_char,
        .decode_char = utf32le_decode_char,
        .validate = utf32_validate,
        .char_count = utf32_char_count,
        .byte_count = utf32_byte_count
    },
    .state_size = 0,
    .state_init = NULL,
    .state_reset = NULL,
    .unicode_profile = &s_utf32_unicode_profile,
    .user_data = NULL
};

static const unicodoid_system_t s_utf32be_system = {
    .meta = {
        .id = "org.unicode.utf32be",
        .name = "UTF-32BE",
        .version = "1.0.0",
        .author = "Unicode Consortium",
        .description = "Fixed-width 32-bit Unicode Transformation Format (Big Endian)",
        .code_unit_bits = 32,
        .min_bytes_per_char = 4,
        .max_bytes_per_char = 4,
        .model = UNICODOID_MODEL_FIXED_WIDTH,
        .endianness = UNICODOID_ENDIAN_BIG,
        .compat_level = UNICODOID_COMPAT_FULL,
        .max_char_val = UNICODOID_UNICODE_MAX_SCALAR,
        .replacement_char = UNICODOID_UNICODE_REPLACEMENT
    },
    .ops = {
        .encode_char = utf32be_encode_char,
        .decode_char = utf32be_decode_char,
        .validate = utf32_validate,
        .char_count = utf32_char_count,
        .byte_count = utf32_byte_count
    },
    .state_size = 0,
    .state_init = NULL,
    .state_reset = NULL,
    .unicode_profile = &s_utf32_unicode_profile,
    .user_data = NULL
};

const unicodoid_system_t* unicodoid_system_get_utf32le(void) {
    return &s_utf32le_system;
}

const unicodoid_system_t* unicodoid_system_get_utf32be(void) {
    return &s_utf32be_system;
}
