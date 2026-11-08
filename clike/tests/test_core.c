#include "unicodoid/unicodoid.h"
#include "unicodoid/unicodoid_ucd.h"

#include <stdio.h>
#include <string.h>

static int g_checks = 0;
static int g_failures = 0;

#define CHECK(cond)                                                              \
    do {                                                                         \
        ++g_checks;                                                              \
        if (!(cond)) {                                                           \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);      \
            ++g_failures;                                                        \
        }                                                                        \
    } while (0)

#define CHECK_STATUS(expr, expected)                                             \
    do {                                                                         \
        unicodoid_status_t status_ = (expr);                                     \
        ++g_checks;                                                              \
        if (status_ != (expected)) {                                             \
            fprintf(stderr, "FAIL %s:%d: %s returned %d (%s), expected %d\n",    \
                    __FILE__, __LINE__, #expr, (int)status_,                     \
                    unicodoid_status_string(status_), (int)(expected));          \
            ++g_failures;                                                        \
        }                                                                        \
    } while (0)

static bool bytes_equal(const void* a, size_t a_len, const void* b, size_t b_len) {
    return a_len == b_len && (a_len == 0 || memcmp(a, b, a_len) == 0);
}

static void test_status_and_registry(void) {
    CHECK(unicodoid_status_string(UNICODOID_OK) != NULL);
    CHECK(strcmp(unicodoid_status_string(UNICODOID_ERR_STATE), "invalid codec state") == 0);
    CHECK(strcmp(unicodoid_status_string(UNICODOID_ERR_OVERFLOW), "numeric overflow") == 0);

    unicodoid_registry_cleanup();
    CHECK_STATUS(unicodoid_registry_init(), UNICODOID_OK);
    CHECK_STATUS(unicodoid_registry_init(), UNICODOID_OK); /* idempotent */
    CHECK(unicodoid_registry_count() == 8);

    const unicodoid_system_t* utf8 = unicodoid_registry_find_by_id("org.unicode.utf8");
    CHECK(utf8 == unicodoid_system_get_utf8());
    CHECK(unicodoid_registry_find_by_name("UTF-8") == utf8);
    CHECK(unicodoid_registry_find_by_id("org.example.missing") == NULL);
    CHECK(unicodoid_registry_get_at(0) != NULL);
    CHECK(unicodoid_registry_get_at(unicodoid_registry_count()) == NULL);

    CHECK_STATUS(unicodoid_registry_register(utf8), UNICODOID_ERR_ALREADY_EXISTS);
    CHECK_STATUS(unicodoid_registry_unregister("org.unicode.utf8"), UNICODOID_OK);
    CHECK(unicodoid_registry_find_by_id("org.unicode.utf8") == NULL);
    CHECK_STATUS(unicodoid_registry_unregister("org.unicode.utf8"), UNICODOID_ERR_NOT_FOUND);
    unicodoid_registry_cleanup();
}

static void test_utf8(void) {
    const unicodoid_system_t* sys = unicodoid_system_get_utf8();
    unicodoid_codec_t codec;
    CHECK_STATUS(unicodoid_codec_init(&codec, sys), UNICODOID_OK);

    uint8_t buf[8];
    size_t written = 0;

    CHECK_STATUS(unicodoid_codec_encode_char(&codec, 0x00E9, buf, sizeof(buf), &written),
                 UNICODOID_OK);
    CHECK(written == 2 && buf[0] == 0xC3 && buf[1] == 0xA9);

    unicodoid_char64_t ch = 0;
    size_t read = 0;
    CHECK_STATUS(unicodoid_codec_decode_char(&codec, buf, written, &ch, &read), UNICODOID_OK);
    CHECK(ch == 0x00E9 && read == 2);

    CHECK_STATUS(unicodoid_codec_encode_char(&codec, 0x1F600, buf, sizeof(buf), &written),
                 UNICODOID_OK);
    CHECK(written == 4 && buf[0] == 0xF0 && buf[1] == 0x9F &&
          buf[2] == 0x98 && buf[3] == 0x80);
    CHECK_STATUS(unicodoid_codec_decode_char(&codec, buf, written, &ch, &read), UNICODOID_OK);
    CHECK(ch == 0x1F600 && read == 4);

    /* Invalid and incomplete sequences. */
    const uint8_t overlong[] = { 0xC0, 0x80 };
    CHECK_STATUS(unicodoid_codec_decode_char(&codec, overlong, sizeof(overlong), &ch, &read),
                 UNICODOID_ERR_INVALID_CODE_UNIT);

    const uint8_t truncated[] = { 0xE2, 0x82 };
    CHECK_STATUS(unicodoid_codec_decode_char(&codec, truncated, sizeof(truncated), &ch, &read),
                 UNICODOID_ERR_INCOMPLETE_SEQUENCE);

    const uint8_t surrogate[] = { 0xED, 0xA0, 0x80 };
    CHECK_STATUS(unicodoid_codec_decode_char(&codec, surrogate, sizeof(surrogate), &ch, &read),
                 UNICODOID_ERR_INVALID_CODE_UNIT);

    const uint8_t bad_mix[] = { 0x41, 0xFF, 0x42 };
    size_t invalid_offset = 0;
    CHECK_STATUS(unicodoid_codec_validate(&codec, bad_mix, sizeof(bad_mix), &invalid_offset),
                 UNICODOID_ERR_INVALID_CODE_UNIT);
    CHECK(invalid_offset == 1);

    const unicodoid_char64_t chars[] = { 0x41, 0x00E9, 0x1F600 };
    CHECK_STATUS(unicodoid_codec_encode(&codec, chars, 3, buf, sizeof(buf), &written),
                 UNICODOID_OK);
    CHECK(written == 7);

    unicodoid_char64_t decoded[3];
    size_t count = 0;
    CHECK_STATUS(unicodoid_codec_decode(&codec, buf, written, decoded, 3, &count),
                 UNICODOID_OK);
    CHECK(count == 3 && decoded[0] == 0x41 && decoded[1] == 0x00E9 && decoded[2] == 0x1F600);

    unicodoid_codec_free(&codec);
}

static void test_ascii_and_conlang(void) {
    const unicodoid_system_t* ascii = unicodoid_system_get_ascii();
    CHECK(unicodoid_system_has_unicode_profile(ascii));
    CHECK(!unicodoid_system_is_stateful(ascii));

    unicodoid_codec_t codec;
    CHECK_STATUS(unicodoid_codec_init(&codec, ascii), UNICODOID_OK);

    uint8_t buf[8];
    size_t written = 0;
    CHECK_STATUS(unicodoid_codec_encode_char(&codec, 'A', buf, sizeof(buf), &written),
                 UNICODOID_OK);
    CHECK(written == 1 && buf[0] == 'A');
    CHECK_STATUS(unicodoid_codec_encode_char(&codec, 0x80, buf, sizeof(buf), &written),
                 UNICODOID_ERR_UNMAPPABLE_CHAR);

    const uint8_t high[] = { 0x80 };
    unicodoid_char64_t ch = 0;
    size_t read = 0;
    CHECK_STATUS(unicodoid_codec_decode_char(&codec, high, 1, &ch, &read),
                 UNICODOID_ERR_INVALID_CODE_UNIT);
    unicodoid_codec_free(&codec);

    const unicodoid_system_t* conlang = unicodoid_system_get_custom_conlang();
    CHECK(!unicodoid_system_has_unicode_profile(conlang));
    CHECK(conlang->unicode_profile == NULL);

    CHECK_STATUS(unicodoid_codec_init(&codec, conlang), UNICODOID_OK);
    CHECK_STATUS(unicodoid_codec_encode_char(&codec, 0x3FFF, buf, sizeof(buf), &written),
                 UNICODOID_OK);
    CHECK(written == 2 && buf[0] == 0xBF && buf[1] == 0xFF);
    CHECK_STATUS(unicodoid_codec_decode_char(&codec, buf, written, &ch, &read), UNICODOID_OK);
    CHECK(ch == 0x3FFF && read == 2);
    CHECK_STATUS(unicodoid_codec_encode_char(&codec, 0x4000, buf, sizeof(buf), &written),
                 UNICODOID_ERR_UNMAPPABLE_CHAR);
    unicodoid_codec_free(&codec);

    unicodoid_category_t cat;
    CHECK_STATUS(unicodoid_get_general_category(conlang, 'A', &cat),
                 UNICODOID_ERR_UNSUPPORTED);
}

static void test_strings(void) {
    const unicodoid_system_t* sys = unicodoid_system_get_utf8();
    unicodoid_string_t str;
    const unicodoid_char64_t chars[] = { 'H', 'i' };
    CHECK_STATUS(unicodoid_string_from_chars(&str, sys, chars, 2), UNICODOID_OK);
    CHECK(str.length == 2 && str.data[0] == 'H' && str.data[1] == 'i');

    size_t char_len = 0;
    CHECK_STATUS(unicodoid_string_char_length(&str, &char_len), UNICODOID_OK);
    CHECK(char_len == 2);

    unicodoid_char64_t ch = 0;
    CHECK_STATUS(unicodoid_string_get_char_at(&str, 1, &ch), UNICODOID_OK);
    CHECK(ch == 'i');
    CHECK_STATUS(unicodoid_string_get_char_at(&str, 2, &ch), UNICODOID_ERR_NOT_FOUND);

    CHECK_STATUS(unicodoid_string_append_char(&str, 0x1F600), UNICODOID_OK);
    CHECK_STATUS(unicodoid_string_char_length(&str, &char_len), UNICODOID_OK);
    CHECK(char_len == 3);

    unicodoid_string_t clone;
    CHECK_STATUS(unicodoid_string_clone(&str, &clone), UNICODOID_OK);
    CHECK(unicodoid_string_equals(&str, &clone));
    CHECK(unicodoid_string_compare(&str, &clone) == 0);
    unicodoid_string_free(&clone);
    unicodoid_string_free(&str);

    const unicodoid_char64_t with_zero[] = { 'A', 0x00, 'B' };
    CHECK_STATUS(unicodoid_string_from_chars(&str, sys, with_zero, 3), UNICODOID_OK);
    CHECK(str.length == 3);
    CHECK_STATUS(unicodoid_string_char_length(&str, &char_len), UNICODOID_OK);
    CHECK(char_len == 3);
    CHECK_STATUS(unicodoid_string_get_char_at(&str, 1, &ch), UNICODOID_OK);
    CHECK(ch == 0);
    unicodoid_string_free(&str);

    const uint8_t bad[] = { 0x41, 0xC0, 0x80 };
    CHECK_STATUS(unicodoid_string_from_slice(&str, sys, bad, sizeof(bad)),
                 UNICODOID_ERR_INVALID_CODE_UNIT);
}

static unicodoid_status_t conlang_bridge(
    void* user_data,
    const unicodoid_system_t* src_sys,
    unicodoid_char64_t src_ch,
    const unicodoid_system_t* dst_sys,
    unicodoid_char64_t* out_dst_ch
) {
    (void)user_data;
    (void)src_sys;
    (void)dst_sys;
    if (src_ch > 0x10FFFF) return UNICODOID_ERR_UNMAPPABLE_CHAR;
    *out_dst_ch = src_ch;
    return UNICODOID_OK;
}

static void test_transcoding(void) {
    const unicodoid_system_t* utf8 = unicodoid_system_get_utf8();
    const unicodoid_system_t* utf16 = unicodoid_system_get_utf16le();
    const unicodoid_system_t* ascii = unicodoid_system_get_ascii();
    const unicodoid_system_t* conlang = unicodoid_system_get_custom_conlang();

    const uint8_t text[] = { 'h', 0xC3, 0xA9, 'l', 'l', 'o' }; /* "héllo" */
    unicodoid_string_t src;
    CHECK_STATUS(unicodoid_string_from_slice(&src, utf8, text, sizeof(text)), UNICODOID_OK);

    unicodoid_string_t mid;
    unicodoid_transcode_config_t cfg = unicodoid_transcode_config_default();
    CHECK_STATUS(unicodoid_transcode_string(&src, utf16, &cfg, &mid), UNICODOID_OK);

    unicodoid_string_t roundtrip;
    CHECK_STATUS(unicodoid_transcode_string(&mid, utf8, &cfg, &roundtrip), UNICODOID_OK);
    CHECK(bytes_equal(src.data, src.length, roundtrip.data, roundtrip.length));

    /* Same-system transcoding is a validated byte copy. */
    unicodoid_string_t same;
    CHECK_STATUS(unicodoid_transcode_string(&src, utf8, &cfg, &same), UNICODOID_OK);
    CHECK(bytes_equal(src.data, src.length, same.data, same.length));

    /* ASCII -> UTF-8. */
    const uint8_t hi[] = { 'H', 'i' };
    unicodoid_string_t ascii_str;
    CHECK_STATUS(unicodoid_string_from_slice(&ascii_str, ascii, hi, sizeof(hi)), UNICODOID_OK);
    unicodoid_string_t wide;
    CHECK_STATUS(unicodoid_transcode_string(&ascii_str, utf8, &cfg, &wide), UNICODOID_OK);
    CHECK(bytes_equal(wide.data, wide.length, hi, sizeof(hi)));

    /* No implicit identity mapping between unrelated systems. */
    unicodoid_string_t conlang_str;
    const unicodoid_char64_t conlang_chars[] = { 'A' };
    CHECK_STATUS(unicodoid_string_from_chars(&conlang_str, conlang, conlang_chars, 1),
                 UNICODOID_OK);
    unicodoid_string_t bridged;
    CHECK_STATUS(unicodoid_transcode_string(&conlang_str, utf8, &cfg, &bridged),
                 UNICODOID_ERR_UNMAPPABLE_CHAR);

    cfg.bridge = conlang_bridge;
    CHECK_STATUS(unicodoid_transcode_string(&conlang_str, utf8, &cfg, &bridged), UNICODOID_OK);
    CHECK(bridged.length == 1 && bridged.data[0] == 'A');

    /* Replacement policy for an unrepresentable character. */
    cfg = unicodoid_transcode_config_default();
    cfg.policy = UNICODOID_POLICY_REPLACE;
    const uint8_t accented[] = { 0xC3, 0xA9 }; /* U+00E9 */
    unicodoid_string_t acc;
    CHECK_STATUS(unicodoid_string_from_slice(&acc, utf8, accented, sizeof(accented)),
                 UNICODOID_OK);
    unicodoid_string_t replaced;
    CHECK_STATUS(unicodoid_transcode_string(&acc, ascii, &cfg, &replaced), UNICODOID_OK);
    CHECK(replaced.length == 1 && replaced.data[0] == '?');

    /* Required-size reporting. */
    size_t required = 0;
    CHECK_STATUS(unicodoid_transcode(utf8, text, sizeof(text), utf16, NULL, 0, &required, NULL),
                 UNICODOID_ERR_BUFFER_TOO_SMALL);
    CHECK(required > 0);

    unicodoid_string_free(&replaced);
    unicodoid_string_free(&acc);
    unicodoid_string_free(&bridged);
    unicodoid_string_free(&conlang_str);
    unicodoid_string_free(&wide);
    unicodoid_string_free(&ascii_str);
    unicodoid_string_free(&same);
    unicodoid_string_free(&roundtrip);
    unicodoid_string_free(&mid);
    unicodoid_string_free(&src);
}

static void test_codec_encode_zero_capacity(void) {
    /* Regression for NULL + offset UB: out may be NULL only when out_cap
     * is 0; encoding a non-empty sequence must report BufferTooSmall with
     * written == 0 (size-probe) instead of performing (NULL + 0). */
    const unicodoid_system_t* sys = unicodoid_system_get_utf8();
    unicodoid_codec_t codec;
    CHECK_STATUS(unicodoid_codec_init(&codec, sys), UNICODOID_OK);

    const unicodoid_char64_t chars[] = { 0x41, 0x00E9 };
    size_t written = 99;
    CHECK_STATUS(unicodoid_codec_encode(&codec, chars, 2, NULL, 0, &written),
                 UNICODOID_ERR_BUFFER_TOO_SMALL);
    CHECK(written == 0);

    /* Empty sequence with NULL/0 succeeds. */
    written = 99;
    CHECK_STATUS(unicodoid_codec_encode(&codec, NULL, 0, NULL, 0, &written),
                 UNICODOID_OK);
    CHECK(written == 0);

    /* Invalid scalar with zero capacity reports Unmappable, not BufferTooSmall. */
    const unicodoid_char64_t bad[] = { 0xD800 };
    CHECK_STATUS(unicodoid_codec_encode(&codec, bad, 1, NULL, 0, &written),
                 UNICODOID_ERR_UNMAPPABLE_CHAR);

    /* NULL destination with non-zero capacity is an invalid parameter. */
    CHECK_STATUS(unicodoid_codec_encode(&codec, chars, 2, NULL, 8, &written),
                 UNICODOID_ERR_INVALID_PARAM);

    /* NULL input with non-zero length is an invalid parameter (validate). */
    CHECK_STATUS(unicodoid_codec_validate(&codec, NULL, 3, NULL),
                 UNICODOID_ERR_INVALID_PARAM);
    size_t count = 0;
    CHECK_STATUS(unicodoid_codec_char_count(&codec, NULL, 3, &count),
                 UNICODOID_ERR_INVALID_PARAM);
    /* Empty buffers with NULL are valid. */
    CHECK_STATUS(unicodoid_codec_validate(&codec, NULL, 0, NULL), UNICODOID_OK);
    CHECK_STATUS(unicodoid_codec_char_count(&codec, NULL, 0, &count), UNICODOID_OK);
    CHECK(count == 0);

    unicodoid_codec_free(&codec);
}

static void test_unicode_data_version(void) {
    /* Generated-data baseline: the shared generated Unicode tables must
     * report the Unicode version they were generated from. */
    CHECK(unicodoid_ucd_version() != NULL);
    CHECK(strcmp(unicodoid_ucd_version(), "18.0.0") == 0);
    CHECK(strcmp(unicodoid_ucd_generator_version(), "1.2.0") == 0);

    const unicodoid_system_t* utf8 = unicodoid_system_get_utf8();
    CHECK(unicodoid_system_has_unicode_profile(utf8));
    const unicodoid_unicode_profile_t* profile = utf8->unicode_profile;
    CHECK(profile != NULL);
    CHECK(profile->unicode_major == 18);
    CHECK(profile->unicode_minor == 0);
    CHECK(profile->unicode_patch == 0);
}

int main(void) {
    test_status_and_registry();
    test_utf8();
    test_codec_encode_zero_capacity();
    test_ascii_and_conlang();
    test_strings();
    test_transcoding();
    test_unicode_data_version();

    if (g_failures == 0) {
        printf("test_core: %d checks passed\n", g_checks);
        return 0;
    }
    fprintf(stderr, "test_core: %d of %d checks failed\n", g_failures, g_checks);
    return 1;
}
