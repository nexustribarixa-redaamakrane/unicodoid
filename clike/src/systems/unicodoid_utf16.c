#include "unicodoid/systems/unicodoid_utf16.h"

static unicodoid_status_t utf16_to_unicode(
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

static unicodoid_status_t utf16_from_unicode(
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

static const unicodoid_unicode_profile_t s_utf16_unicode_profile = {
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
    .to_unicode = utf16_to_unicode,
    .from_unicode = utf16_from_unicode
};

static inline void write_u16(uint8_t* p, uint16_t val, bool is_le) {
    if (is_le) {
        p[0] = (uint8_t)(val & 0xFF);
        p[1] = (uint8_t)((val >> 8) & 0xFF);
    } else {
        p[0] = (uint8_t)((val >> 8) & 0xFF);
        p[1] = (uint8_t)(val & 0xFF);
    }
}

static inline uint16_t read_u16(const uint8_t* p, bool is_le) {
    if (is_le) return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
    return ((uint16_t)p[0] << 8) | (uint16_t)p[1];
}

static unicodoid_status_t utf16_encode_internal(
    unicodoid_char64_t ch,
    void* out_buf,
    size_t out_cap,
    size_t* out_written,
    bool is_le
) {
    if (!out_buf || !out_written) return UNICODOID_ERR_INVALID_PARAM;
    if (!unicodoid_is_valid_unicode_scalar((uint32_t)ch)) return UNICODOID_ERR_UNMAPPABLE_CHAR;

    uint8_t* p = (uint8_t*)out_buf;
    if (ch <= 0xFFFF) {
        if (out_cap < 2) return UNICODOID_ERR_BUFFER_TOO_SMALL;
        write_u16(p, (uint16_t)ch, is_le);
        *out_written = 2;
    } else {
        if (out_cap < 4) return UNICODOID_ERR_BUFFER_TOO_SMALL;
        uint32_t val = (uint32_t)(ch - 0x10000);
        uint16_t high = (uint16_t)(0xD800 + (val >> 10));
        uint16_t low = (uint16_t)(0xDC00 + (val & 0x3FF));
        write_u16(p, high, is_le);
        write_u16(p + 2, low, is_le);
        *out_written = 4;
    }
    return UNICODOID_OK;
}

static unicodoid_status_t utf16_decode_internal(
    const void* in_buf,
    size_t in_len,
    unicodoid_char64_t* out_ch,
    size_t* out_read,
    bool is_le
) {
    if (!in_buf || !out_ch || !out_read) return UNICODOID_ERR_INVALID_PARAM;
    if (in_len < 2) return UNICODOID_ERR_INCOMPLETE_SEQUENCE;

    const uint8_t* p = (const uint8_t*)in_buf;
    uint16_t w1 = read_u16(p, is_le);

    if (w1 >= 0xD800 && w1 <= 0xDBFF) {
        if (in_len < 4) return UNICODOID_ERR_INCOMPLETE_SEQUENCE;
        uint16_t w2 = read_u16(p + 2, is_le);
        if (w2 < 0xDC00 || w2 > 0xDFFF) return UNICODOID_ERR_INVALID_CODE_UNIT;
        uint32_t val = (((uint32_t)(w1 - 0xD800) << 10) | (uint32_t)(w2 - 0xDC00)) + 0x10000;
        *out_ch = val;
        *out_read = 4;
        return UNICODOID_OK;
    }
    if (w1 >= 0xDC00 && w1 <= 0xDFFF) return UNICODOID_ERR_INVALID_CODE_UNIT;

    *out_ch = w1;
    *out_read = 2;
    return UNICODOID_OK;
}

static unicodoid_status_t utf16le_encode_char(
    const unicodoid_system_t* sys, void* state, unicodoid_char64_t ch,
    void* out, size_t cap, size_t* written
) {
    (void)sys; (void)state;
    return utf16_encode_internal(ch, out, cap, written, true);
}

static unicodoid_status_t utf16le_decode_char(
    const unicodoid_system_t* sys, void* state, const void* in, size_t len,
    unicodoid_char64_t* ch, size_t* read
) {
    (void)sys; (void)state;
    return utf16_decode_internal(in, len, ch, read, true);
}

static unicodoid_status_t utf16le_validate(
    const unicodoid_system_t* sys, void* state, const void* in, size_t len,
    size_t* invalid
) {
    if (len > 0 && !in) return UNICODOID_ERR_INVALID_PARAM;
    size_t off = 0;
    while (off < len) {
        unicodoid_char64_t ch;
        size_t read = 0;
        unicodoid_status_t st = utf16le_decode_char(
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

static unicodoid_status_t utf16le_char_count(
    const unicodoid_system_t* sys, void* state, const void* in, size_t len,
    size_t* out_count
) {
    if (!out_count) return UNICODOID_ERR_INVALID_PARAM;
    if (!in && len > 0) return UNICODOID_ERR_INVALID_PARAM;
    if (len == 0) { *out_count = 0; return UNICODOID_OK; }
    size_t count = 0, off = 0;
    while (off < len) {
        unicodoid_char64_t ch;
        size_t read = 0;
        unicodoid_status_t st = utf16le_decode_char(
            sys, state, (const uint8_t*)in + off, len - off, &ch, &read
        );
        if (st != UNICODOID_OK) return st;
        off += read;
        ++count;
    }
    *out_count = count;
    return UNICODOID_OK;
}

static unicodoid_status_t utf16_byte_count(
    const unicodoid_system_t* sys, const unicodoid_char64_t* chars,
    size_t num_chars, size_t* out_bytes
) {
    (void)sys;
    if (!out_bytes) return UNICODOID_ERR_INVALID_PARAM;
    if (!chars && num_chars > 0) return UNICODOID_ERR_INVALID_PARAM;
    size_t total = 0;
    for (size_t i = 0; i < num_chars; ++i) {
        if (!unicodoid_is_valid_unicode_scalar((uint32_t)chars[i])) {
            return UNICODOID_ERR_UNMAPPABLE_CHAR;
        }
        size_t need = chars[i] <= 0xFFFF ? 2 : 4;
        if (need > SIZE_MAX - total) return UNICODOID_ERR_OVERFLOW;
        total += need;
    }
    *out_bytes = total;
    return UNICODOID_OK;
}

static unicodoid_status_t utf16be_encode_char(
    const unicodoid_system_t* sys, void* state, unicodoid_char64_t ch,
    void* out, size_t cap, size_t* written
) {
    (void)sys; (void)state;
    return utf16_encode_internal(ch, out, cap, written, false);
}

static unicodoid_status_t utf16be_decode_char(
    const unicodoid_system_t* sys, void* state, const void* in, size_t len,
    unicodoid_char64_t* ch, size_t* read
) {
    (void)sys; (void)state;
    return utf16_decode_internal(in, len, ch, read, false);
}

static unicodoid_status_t utf16be_validate(
    const unicodoid_system_t* sys, void* state, const void* in, size_t len,
    size_t* invalid
) {
    if (len > 0 && !in) return UNICODOID_ERR_INVALID_PARAM;
    size_t off = 0;
    while (off < len) {
        unicodoid_char64_t ch;
        size_t read = 0;
        unicodoid_status_t st = utf16be_decode_char(
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

static unicodoid_status_t utf16be_char_count(
    const unicodoid_system_t* sys, void* state, const void* in, size_t len,
    size_t* out_count
) {
    if (!out_count) return UNICODOID_ERR_INVALID_PARAM;
    if (!in && len > 0) return UNICODOID_ERR_INVALID_PARAM;
    if (len == 0) { *out_count = 0; return UNICODOID_OK; }
    size_t count = 0, off = 0;
    while (off < len) {
        unicodoid_char64_t ch;
        size_t read = 0;
        unicodoid_status_t st = utf16be_decode_char(
            sys, state, (const uint8_t*)in + off, len - off, &ch, &read
        );
        if (st != UNICODOID_OK) return st;
        off += read;
        ++count;
    }
    *out_count = count;
    return UNICODOID_OK;
}

static const unicodoid_system_t s_utf16le_system = {
    .meta = {
        .id = "org.unicode.utf16le",
        .name = "UTF-16LE",
        .version = "1.0.0",
        .author = "Unicode Consortium",
        .description = "UTF-16 Little Endian",
        .code_unit_bits = 16,
        .min_bytes_per_char = 2,
        .max_bytes_per_char = 4,
        .model = UNICODOID_MODEL_VARIABLE_WIDTH,
        .endianness = UNICODOID_ENDIAN_LITTLE,
        .compat_level = UNICODOID_COMPAT_FULL,
        .max_char_val = UNICODOID_UNICODE_MAX_SCALAR,
        .replacement_char = UNICODOID_UNICODE_REPLACEMENT
    },
    .ops = {
        .encode_char = utf16le_encode_char,
        .decode_char = utf16le_decode_char,
        .validate = utf16le_validate,
        .char_count = utf16le_char_count,
        .byte_count = utf16_byte_count
    },
    .state_size = 0,
    .state_init = NULL,
    .state_reset = NULL,
    .unicode_profile = &s_utf16_unicode_profile,
    .user_data = NULL
};

static const unicodoid_system_t s_utf16be_system = {
    .meta = {
        .id = "org.unicode.utf16be",
        .name = "UTF-16BE",
        .version = "1.0.0",
        .author = "Unicode Consortium",
        .description = "UTF-16 Big Endian",
        .code_unit_bits = 16,
        .min_bytes_per_char = 2,
        .max_bytes_per_char = 4,
        .model = UNICODOID_MODEL_VARIABLE_WIDTH,
        .endianness = UNICODOID_ENDIAN_BIG,
        .compat_level = UNICODOID_COMPAT_FULL,
        .max_char_val = UNICODOID_UNICODE_MAX_SCALAR,
        .replacement_char = UNICODOID_UNICODE_REPLACEMENT
    },
    .ops = {
        .encode_char = utf16be_encode_char,
        .decode_char = utf16be_decode_char,
        .validate = utf16be_validate,
        .char_count = utf16be_char_count,
        .byte_count = utf16_byte_count
    },
    .state_size = 0,
    .state_init = NULL,
    .state_reset = NULL,
    .unicode_profile = &s_utf16_unicode_profile,
    .user_data = NULL
};

const unicodoid_system_t* unicodoid_system_get_utf16le(void) {
    return &s_utf16le_system;
}

const unicodoid_system_t* unicodoid_system_get_utf16be(void) {
    return &s_utf16be_system;
}
