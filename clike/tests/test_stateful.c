#include "unicodoid/unicodoid.h"

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

static void test_stateful_codec(void) {
    const unicodoid_system_t* sys = unicodoid_system_get_shiftmodal16();
    CHECK(sys != NULL);
    CHECK(unicodoid_system_is_stateful(sys));
    CHECK(!unicodoid_system_has_unicode_profile(sys));
    CHECK(sys->state_size > 0);
    CHECK(sys->state_init != NULL && sys->state_reset != NULL);
    CHECK(sys->meta.model == UNICODOID_MODEL_MODAL_STATEFUL);

    const unicodoid_char64_t chars[] = { 0x41, 0x4000, 0x4001, 0x42, 0x0100 };
    const uint8_t expected[] = {
        0x41,
        0x0E, 0x00,
        0x01,
        0x0F, 0x42,
        0x1B, 0x00
    };

    size_t needed = 0;
    CHECK_STATUS(sys->ops.byte_count(sys, chars, 5, &needed), UNICODOID_OK);
    CHECK(needed == sizeof(expected));

    unicodoid_codec_t codec;
    CHECK_STATUS(unicodoid_codec_init(&codec, sys), UNICODOID_OK);
    CHECK(codec.state != NULL);

    uint8_t encoded[16];
    size_t written = 0;
    CHECK_STATUS(unicodoid_codec_encode(&codec, chars, 5, encoded, sizeof(encoded), &written),
                 UNICODOID_OK);
    CHECK(bytes_equal(encoded, written, expected, sizeof(expected)));

    unicodoid_char64_t decoded[8];
    size_t count = 0;
    CHECK_STATUS(unicodoid_codec_decode(&codec, encoded, written, decoded, 8, &count),
                 UNICODOID_OK);
    CHECK(count == 5);
    CHECK(decoded[0] == 0x41);
    CHECK(decoded[1] == 0x4000);
    CHECK(decoded[2] == 0x4001);
    CHECK(decoded[3] == 0x42);
    CHECK(decoded[4] == 0x0100);

    size_t char_count = 0;
    CHECK_STATUS(unicodoid_codec_char_count(&codec, encoded, written, &char_count),
                 UNICODOID_OK);
    CHECK(char_count == 5);

    size_t invalid_offset = 0;
    CHECK_STATUS(unicodoid_codec_validate(&codec, encoded, written, &invalid_offset),
                 UNICODOID_OK);

    /* Reset restores base mode. */
    CHECK_STATUS(unicodoid_codec_reset(&codec), UNICODOID_OK);
    CHECK_STATUS(unicodoid_codec_encode_char(&codec, 0x41, encoded, sizeof(encoded), &written),
                 UNICODOID_OK);
    CHECK(written == 1 && encoded[0] == 0x41);

    /* A trailing escape or shift control is incomplete. */
    const uint8_t trailing_escape[] = { 0x41, 0x1B };
    CHECK_STATUS(unicodoid_codec_validate(&codec, trailing_escape, sizeof(trailing_escape),
                                          &invalid_offset),
                 UNICODOID_ERR_INCOMPLETE_SEQUENCE);
    CHECK(invalid_offset == 1);

    /* A control-only fragment is valid: it changes state but yields no char. */
    const uint8_t lone_shift[] = { 0x0E };
    CHECK_STATUS(unicodoid_codec_validate(&codec, lone_shift, sizeof(lone_shift),
                                          &invalid_offset),
                 UNICODOID_OK);
    CHECK_STATUS(unicodoid_codec_reset(&codec), UNICODOID_OK);
    size_t control_chars = 99;
    CHECK_STATUS(unicodoid_codec_char_count(&codec, lone_shift, sizeof(lone_shift),
                                            &control_chars),
                 UNICODOID_OK);
    CHECK(control_chars == 0);
    CHECK_STATUS(unicodoid_codec_reset(&codec), UNICODOID_OK);

    /* Reserved native values cannot be represented. */
    const unicodoid_char64_t reserved[] = { 0x000E, 0x000F, 0x001B, 0x400F, 0x401B };
    for (size_t i = 0; i < sizeof(reserved) / sizeof(reserved[0]); ++i) {
        CHECK_STATUS(unicodoid_codec_reset(&codec), UNICODOID_OK);
        CHECK_STATUS(unicodoid_codec_encode_char(&codec, reserved[i], encoded,
                                                 sizeof(encoded), &written),
                     UNICODOID_ERR_UNMAPPABLE_CHAR);
    }

    unicodoid_codec_free(&codec);
}

static void test_stateful_string(void) {
    const unicodoid_system_t* sys = unicodoid_system_get_shiftmodal16();
    const unicodoid_char64_t chars[] = { 0x41, 0x4000, 0x4001, 0x42, 0x0100 };

    unicodoid_string_t str;
    CHECK_STATUS(unicodoid_string_from_chars(&str, sys, chars, 5), UNICODOID_OK);
    CHECK(str.length == 8);
    CHECK(str.state != NULL && str.state_size == sys->state_size);

    size_t char_len = 0;
    CHECK_STATUS(unicodoid_string_char_length(&str, &char_len), UNICODOID_OK);
    CHECK(char_len == 5);

    unicodoid_char64_t ch = 0;
    CHECK_STATUS(unicodoid_string_get_char_at(&str, 1, &ch), UNICODOID_OK);
    CHECK(ch == 0x4000);
    CHECK_STATUS(unicodoid_string_get_char_at(&str, 4, &ch), UNICODOID_OK);
    CHECK(ch == 0x0100);

    /* Clone copies both bytes and codec state. */
    unicodoid_string_t clone;
    CHECK_STATUS(unicodoid_string_clone(&str, &clone), UNICODOID_OK);
    CHECK(unicodoid_string_equals(&str, &clone));

    /* Both strings are in base mode after the sequence; append shifted data. */
    CHECK_STATUS(unicodoid_string_append_char(&clone, 0x4002), UNICODOID_OK);
    CHECK(clone.length == 10);
    CHECK(clone.data[8] == 0x0E && clone.data[9] == 0x02);

    /* The original is unaffected. */
    CHECK(str.length == 8);

    /* State can be reset and the string extended from base mode. */
    CHECK_STATUS(unicodoid_string_reset_state(&str), UNICODOID_OK);
    CHECK_STATUS(unicodoid_string_append_char(&str, 0x41), UNICODOID_OK);
    CHECK(str.length == 9 && str.data[8] == 0x41);

    unicodoid_string_free(&clone);
    unicodoid_string_free(&str);
}

static void test_append_bytes_state(void) {
    const unicodoid_system_t* sys = unicodoid_system_get_shiftmodal16();
    unicodoid_string_t str;
    CHECK_STATUS(unicodoid_string_init(&str, sys, 8), UNICODOID_OK);

    /* Enter shifted mode, then append a raw continuation byte. */
    CHECK_STATUS(unicodoid_string_append_char(&str, 0x4000), UNICODOID_OK);
    CHECK(str.length == 2 && str.data[0] == 0x0E && str.data[1] == 0x00);

    const uint8_t continuation[] = { 0x01 };
    CHECK_STATUS(unicodoid_string_append_bytes(&str, continuation, 1), UNICODOID_OK);
    CHECK(str.length == 3 && str.data[2] == 0x01);

    /* SI returns to base mode; a following literal is one byte. */
    const uint8_t shift_in[] = { 0x0F };
    CHECK_STATUS(unicodoid_string_append_bytes(&str, shift_in, 1), UNICODOID_OK);
    CHECK_STATUS(unicodoid_string_append_char(&str, 0x41), UNICODOID_OK);
    CHECK(str.length == 5 && str.data[4] == 0x41);

    size_t char_len = 0;
    CHECK_STATUS(unicodoid_string_char_length(&str, &char_len), UNICODOID_OK);
    CHECK(char_len == 3);

    unicodoid_string_free(&str);
}

static unicodoid_status_t shiftmodal_bridge(
    void* user_data,
    const unicodoid_system_t* src_sys,
    unicodoid_char64_t src_ch,
    const unicodoid_system_t* dst_sys,
    unicodoid_char64_t* out_dst_ch
) {
    (void)user_data;
    (void)src_sys;
    (void)dst_sys;
    if (src_ch >= 0x4000 && src_ch <= 0x40FF) {
        *out_dst_ch = src_ch & 0xFF;
        return UNICODOID_OK;
    }
    if (src_ch <= 0x3FFF) {
        *out_dst_ch = src_ch;
        return UNICODOID_OK;
    }
    return UNICODOID_ERR_UNMAPPABLE_CHAR;
}

static void test_stateful_transcode(void) {
    const unicodoid_system_t* shift = unicodoid_system_get_shiftmodal16();
    const unicodoid_system_t* conlang = unicodoid_system_get_custom_conlang();

    const unicodoid_char64_t chars[] = { 0x4001, 0x41 };
    unicodoid_string_t src;
    CHECK_STATUS(unicodoid_string_from_chars(&src, shift, chars, 2), UNICODOID_OK);

    unicodoid_transcode_config_t cfg = unicodoid_transcode_config_default();
    unicodoid_string_t out;

    /* Unrelated systems need an explicit bridge. */
    CHECK_STATUS(unicodoid_transcode_string(&src, conlang, &cfg, &out),
                 UNICODOID_ERR_UNMAPPABLE_CHAR);

    cfg.bridge = shiftmodal_bridge;
    CHECK_STATUS(unicodoid_transcode_string(&src, conlang, &cfg, &out), UNICODOID_OK);
    CHECK(out.length == 2 && out.data[0] == 0x01 && out.data[1] == 0x41);

    /* Same-system round trip preserves the canonical byte stream. */
    cfg = unicodoid_transcode_config_default();
    unicodoid_string_t same;
    CHECK_STATUS(unicodoid_transcode_string(&src, shift, &cfg, &same), UNICODOID_OK);
    CHECK(bytes_equal(src.data, src.length, same.data, same.length));

    unicodoid_string_free(&same);
    unicodoid_string_free(&out);
    unicodoid_string_free(&src);
}

int main(void) {
    test_stateful_codec();
    test_stateful_string();
    test_append_bytes_state();
    test_stateful_transcode();

    if (g_failures == 0) {
        printf("test_stateful: %d checks passed\n", g_checks);
        return 0;
    }
    fprintf(stderr, "test_stateful: %d of %d checks failed\n", g_failures, g_checks);
    return 1;
}
