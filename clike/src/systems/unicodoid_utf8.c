#include "unicodoid/systems/unicodoid_utf8.h"

static unicodoid_status_t utf8_to_unicode(
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

static unicodoid_status_t utf8_from_unicode(
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

static const unicodoid_unicode_profile_t s_utf8_unicode_profile = {
    .unicode_major = 18,
    .unicode_minor = 0,
    .unicode_patch = 0,
    .supported_uax = UNICODOID_UAX_9_BIDI | UNICODOID_UAX_11_EAST_ASIAN_W |
                     UNICODOID_UAX_14_LINEBREAK | UNICODOID_UAX_15_NORMALIZATION |
                     UNICODOID_UAX_24_SCRIPT_PROP | UNICODOID_UAX_29_TEXT_SEGMENT |
                     UNICODOID_UAX_31_ID_SYNTAX | UNICODOID_UAX_44_UCD,
    .supported_uts = UNICODOID_UTS_10_UCA | UNICODOID_UTS_18_REGEX |
                     UNICODOID_UTS_35_LDML | UNICODOID_UTS_39_SECURITY |
                     UNICODOID_UTS_46_IDNA | UNICODOID_UTS_51_EMOJI,
    .supported_utr = UNICODOID_UTR_17_ENCODING_MODEL | UNICODOID_UTR_23_PROP_MODEL |
                     UNICODOID_UTR_25_MATH | UNICODOID_UTR_33_CONFORMANCE,
    .context = NULL,
    .to_unicode = utf8_to_unicode,
    .from_unicode = utf8_from_unicode
};

static unicodoid_status_t utf8_encode_char(
    const unicodoid_system_t* sys,
    void* state,
    unicodoid_char64_t ch,
    void* out_buf,
    size_t out_cap,
    size_t* out_written
) {
    (void)sys; (void)state;
    if (!out_buf || !out_written) return UNICODOID_ERR_INVALID_PARAM;
    if (!unicodoid_is_valid_unicode_scalar((uint32_t)ch)) return UNICODOID_ERR_UNMAPPABLE_CHAR;

    uint8_t* p = (uint8_t*)out_buf;
    if (ch <= 0x7F) {
        if (out_cap < 1) return UNICODOID_ERR_BUFFER_TOO_SMALL;
        p[0] = (uint8_t)ch;
        *out_written = 1;
    } else if (ch <= 0x7FF) {
        if (out_cap < 2) return UNICODOID_ERR_BUFFER_TOO_SMALL;
        p[0] = (uint8_t)(0xC0 | (ch >> 6));
        p[1] = (uint8_t)(0x80 | (ch & 0x3F));
        *out_written = 2;
    } else if (ch <= 0xFFFF) {
        if (out_cap < 3) return UNICODOID_ERR_BUFFER_TOO_SMALL;
        p[0] = (uint8_t)(0xE0 | (ch >> 12));
        p[1] = (uint8_t)(0x80 | ((ch >> 6) & 0x3F));
        p[2] = (uint8_t)(0x80 | (ch & 0x3F));
        *out_written = 3;
    } else {
        if (out_cap < 4) return UNICODOID_ERR_BUFFER_TOO_SMALL;
        p[0] = (uint8_t)(0xF0 | (ch >> 18));
        p[1] = (uint8_t)(0x80 | ((ch >> 12) & 0x3F));
        p[2] = (uint8_t)(0x80 | ((ch >> 6) & 0x3F));
        p[3] = (uint8_t)(0x80 | (ch & 0x3F));
        *out_written = 4;
    }
    return UNICODOID_OK;
}

static unicodoid_status_t utf8_decode_char(
    const unicodoid_system_t* sys,
    void* state,
    const void* in_buf,
    size_t in_len,
    unicodoid_char64_t* out_ch,
    size_t* out_read
) {
    (void)sys; (void)state;
    if (!in_buf || !out_ch || !out_read) return UNICODOID_ERR_INVALID_PARAM;
    if (in_len == 0) return UNICODOID_ERR_INCOMPLETE_SEQUENCE;

    const uint8_t* p = (const uint8_t*)in_buf;
    uint8_t b0 = p[0];

    if ((b0 & 0x80) == 0x00) {
        *out_ch = b0;
        *out_read = 1;
        return UNICODOID_OK;
    } else if ((b0 & 0xE0) == 0xC0) {
        if (in_len < 2) return UNICODOID_ERR_INCOMPLETE_SEQUENCE;
        uint8_t b1 = p[1];
        if ((b1 & 0xC0) != 0x80) return UNICODOID_ERR_INVALID_CODE_UNIT;
        uint32_t val = ((b0 & 0x1F) << 6) | (b1 & 0x3F);
        if (val < 0x80) return UNICODOID_ERR_INVALID_CODE_UNIT;
        *out_ch = val;
        *out_read = 2;
        return UNICODOID_OK;
    } else if ((b0 & 0xF0) == 0xE0) {
        if (in_len < 3) return UNICODOID_ERR_INCOMPLETE_SEQUENCE;
        uint8_t b1 = p[1];
        uint8_t b2 = p[2];
        if ((b1 & 0xC0) != 0x80 || (b2 & 0xC0) != 0x80) return UNICODOID_ERR_INVALID_CODE_UNIT;
        uint32_t val = ((b0 & 0x0F) << 12) | ((b1 & 0x3F) << 6) | (b2 & 0x3F);
        if (val < 0x800) return UNICODOID_ERR_INVALID_CODE_UNIT;
        if (val >= 0xD800 && val <= 0xDFFF) return UNICODOID_ERR_INVALID_CODE_UNIT;
        *out_ch = val;
        *out_read = 3;
        return UNICODOID_OK;
    } else if ((b0 & 0xF8) == 0xF0) {
        if (in_len < 4) return UNICODOID_ERR_INCOMPLETE_SEQUENCE;
        uint8_t b1 = p[1];
        uint8_t b2 = p[2];
        uint8_t b3 = p[3];
        if ((b1 & 0xC0) != 0x80 || (b2 & 0xC0) != 0x80 || (b3 & 0xC0) != 0x80) {
            return UNICODOID_ERR_INVALID_CODE_UNIT;
        }
        uint32_t val = ((b0 & 0x07) << 18) | ((b1 & 0x3F) << 12) |
                       ((b2 & 0x3F) << 6) | (b3 & 0x3F);
        if (val < 0x10000 || val > 0x10FFFF) return UNICODOID_ERR_INVALID_CODE_UNIT;
        *out_ch = val;
        *out_read = 4;
        return UNICODOID_OK;
    }

    return UNICODOID_ERR_INVALID_CODE_UNIT;
}

static unicodoid_status_t utf8_validate(
    const unicodoid_system_t* sys,
    void* state,
    const void* in_buf,
    size_t in_len,
    size_t* out_invalid_offset
) {
    size_t offset = 0;
    while (offset < in_len) {
        unicodoid_char64_t ch;
        size_t read_bytes = 0;
        unicodoid_status_t st = utf8_decode_char(
            sys, state, (const uint8_t*)in_buf + offset, in_len - offset, &ch, &read_bytes
        );
        if (st != UNICODOID_OK) {
            if (out_invalid_offset) *out_invalid_offset = offset;
            return st;
        }
        offset += read_bytes;
    }
    return UNICODOID_OK;
}

static unicodoid_status_t utf8_char_count(
    const unicodoid_system_t* sys,
    void* state,
    const void* in_buf,
    size_t in_len,
    size_t* out_count
) {
    if (!in_buf || !out_count) return UNICODOID_ERR_INVALID_PARAM;
    size_t count = 0;
    size_t offset = 0;
    while (offset < in_len) {
        unicodoid_char64_t ch;
        size_t read_bytes = 0;
        unicodoid_status_t st = utf8_decode_char(
            sys, state, (const uint8_t*)in_buf + offset, in_len - offset, &ch, &read_bytes
        );
        if (st != UNICODOID_OK) return st;
        offset += read_bytes;
        ++count;
    }
    *out_count = count;
    return UNICODOID_OK;
}

static unicodoid_status_t utf8_byte_count(
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
        if (!unicodoid_is_valid_unicode_scalar((uint32_t)ch)) return UNICODOID_ERR_UNMAPPABLE_CHAR;
        if (ch <= 0x7F) total += 1;
        else if (ch <= 0x7FF) total += 2;
        else if (ch <= 0xFFFF) total += 3;
        else total += 4;
    }
    *out_bytes = total;
    return UNICODOID_OK;
}

static const unicodoid_system_t s_utf8_system = {
    .meta = {
        .id = "org.unicode.utf8",
        .name = "UTF-8",
        .version = "1.0.0",
        .author = "Unicode Consortium / Ken Thompson / Rob Pike",
        .description = "Variable-width 8-bit Unicode Transformation Format",
        .code_unit_bits = 8,
        .min_bytes_per_char = 1,
        .max_bytes_per_char = 4,
        .model = UNICODOID_MODEL_VARIABLE_WIDTH,
        .endianness = UNICODOID_ENDIAN_NONE,
        .compat_level = UNICODOID_COMPAT_FULL,
        .max_char_val = UNICODOID_UNICODE_MAX_SCALAR,
        .replacement_char = UNICODOID_UNICODE_REPLACEMENT
    },
    .ops = {
        .encode_char = utf8_encode_char,
        .decode_char = utf8_decode_char,
        .validate = utf8_validate,
        .char_count = utf8_char_count,
        .byte_count = utf8_byte_count
    },
    .state_size = 0,
    .state_init = NULL,
    .state_reset = NULL,
    .unicode_profile = &s_utf8_unicode_profile,
    .user_data = NULL
};

const unicodoid_system_t* unicodoid_system_get_utf8(void) {
    return &s_utf8_system;
}
