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

static unicodoid_string_t make_string(const unicodoid_system_t* sys,
                                      const uint8_t* bytes, size_t len) {
    unicodoid_string_t s;
    unicodoid_status_t st = unicodoid_string_from_slice(&s, sys, bytes, len);
    if (st != UNICODOID_OK) {
        fprintf(stderr, "failed to create test string: %s\n", unicodoid_status_string(st));
        ++g_failures;
        memset(&s, 0, sizeof(s));
    }
    return s;
}

static void test_catalog(void) {
    unicodoid_unicode_spec_ref_t ref;

    CHECK(unicodoid_unicode_spec_count() > 10);
    CHECK_STATUS(unicodoid_unicode_spec_at(0, &ref), UNICODOID_OK);
    CHECK(ref.title != NULL && ref.url != NULL);

    CHECK_STATUS(unicodoid_get_unicode_spec_ref(UNICODOID_DOC_UAX, 15, &ref), UNICODOID_OK);
    CHECK(ref.category == UNICODOID_DOC_UAX);
    CHECK(strstr(ref.title, "Normalization") != NULL);

    CHECK_STATUS(unicodoid_get_unicode_spec_ref(UNICODOID_DOC_UAX, 31, &ref), UNICODOID_OK);
    CHECK(strstr(ref.title, "Identifiers") != NULL);

    CHECK_STATUS(unicodoid_get_unicode_spec_ref(UNICODOID_DOC_UTN, 1, &ref), UNICODOID_OK);
    CHECK(strstr(ref.title, "Indic Language Collation") != NULL);

    CHECK_STATUS(unicodoid_get_unicode_spec_ref(UNICODOID_DOC_UAX, 999, &ref),
                 UNICODOID_ERR_NOT_FOUND);
    CHECK_STATUS(unicodoid_unicode_spec_at(unicodoid_unicode_spec_count(), &ref),
                 UNICODOID_ERR_NOT_FOUND);
}

static void test_scalar_properties(void) {
    CHECK(unicodoid_unicode_general_category('A') == UNICODOID_CAT_UPPERCASE_LETTER);
    CHECK(unicodoid_unicode_general_category('a') == UNICODOID_CAT_LOWERCASE_LETTER);
    CHECK(unicodoid_unicode_general_category('1') == UNICODOID_CAT_DECIMAL_NUMBER);
    CHECK(unicodoid_unicode_general_category(' ') == UNICODOID_CAT_SPACE_SEPARATOR);
    CHECK(unicodoid_unicode_general_category(0x0301) == UNICODOID_CAT_MARK);
    CHECK(unicodoid_unicode_general_category(0x000A) == UNICODOID_CAT_CONTROL);

    CHECK(unicodoid_unicode_is_alphabetic('A'));
    CHECK(!unicodoid_unicode_is_alphabetic('1'));
    CHECK(unicodoid_unicode_is_numeric('9'));
    CHECK(unicodoid_unicode_is_whitespace('\t'));
    CHECK(unicodoid_unicode_is_whitespace(' '));
    CHECK(unicodoid_unicode_is_control('\n'));
    CHECK(unicodoid_unicode_is_punctuation('!'));
    CHECK(unicodoid_unicode_is_mark(0x0301));

    CHECK(unicodoid_unicode_combining_class(0x0301) == 230);
    CHECK(unicodoid_unicode_combining_class('a') == 0);

    CHECK(unicodoid_unicode_to_uppercase('a') == 'A');
    CHECK(unicodoid_unicode_to_lowercase('A') == 'a');
    CHECK(unicodoid_unicode_ea_width('A') == UNICODOID_EA_NARROW);
    CHECK(unicodoid_unicode_ea_width(0xAC00) == UNICODOID_EA_WIDE);

    const unicodoid_system_t* utf8 = unicodoid_system_get_utf8();
    unicodoid_category_t cat;
    CHECK_STATUS(unicodoid_get_general_category(utf8, 'A', &cat), UNICODOID_OK);
    CHECK(cat == UNICODOID_CAT_UPPERCASE_LETTER);

    unicodoid_char64_t upper = 0;
    CHECK_STATUS(unicodoid_to_uppercase(utf8, 'a', &upper), UNICODOID_OK);
    CHECK(upper == 'A');

    const unicodoid_system_t* conlang = unicodoid_system_get_custom_conlang();
    CHECK_STATUS(unicodoid_get_general_category(conlang, 'A', &cat),
                 UNICODOID_ERR_UNSUPPORTED);
}

static void test_normalization(void) {
    const unicodoid_system_t* utf8 = unicodoid_system_get_utf8();

    /* e + combining acute -> é under NFC. */
    const uint8_t decomposed[] = { 0x65, 0xCC, 0x81 };
    const uint8_t composed[] = { 0xC3, 0xA9 };

    CHECK(!unicodoid_is_normalized(utf8, decomposed, sizeof(decomposed), UNICODOID_NORM_NFC));
    CHECK(unicodoid_is_normalized(utf8, composed, sizeof(composed), UNICODOID_NORM_NFC));

    uint8_t out[32];
    size_t written = 0;
    CHECK_STATUS(unicodoid_normalize(utf8, decomposed, sizeof(decomposed), UNICODOID_NORM_NFC,
                                     out, sizeof(out), &written), UNICODOID_OK);
    CHECK(bytes_equal(out, written, composed, sizeof(composed)));

    CHECK_STATUS(unicodoid_normalize(utf8, composed, sizeof(composed), UNICODOID_NORM_NFD,
                                     out, sizeof(out), &written), UNICODOID_OK);
    CHECK(bytes_equal(out, written, decomposed, sizeof(decomposed)));

    /* Compatibility ligature fi. */
    const uint8_t ligature[] = { 0xEF, 0xAC, 0x81 }; /* U+FB01 */
    const uint8_t fi[] = { 0x66, 0x69 };
    CHECK_STATUS(unicodoid_normalize(utf8, ligature, sizeof(ligature), UNICODOID_NORM_NFKD,
                                     out, sizeof(out), &written), UNICODOID_OK);
    CHECK(bytes_equal(out, written, fi, sizeof(fi)));
    CHECK_STATUS(unicodoid_normalize(utf8, ligature, sizeof(ligature), UNICODOID_NORM_NFC,
                                     out, sizeof(out), &written), UNICODOID_OK);
    CHECK(bytes_equal(out, written, ligature, sizeof(ligature)));

    /* Hangul syllable decomposition and recomposition. */
    const uint8_t hangul[] = { 0xEA, 0xB0, 0x80 };          /* U+AC00 */
    const uint8_t jamo[] = { 0xE1, 0x84, 0x80, 0xE1, 0x85, 0xA1 };
    CHECK_STATUS(unicodoid_normalize(utf8, hangul, sizeof(hangul), UNICODOID_NORM_NFD,
                                     out, sizeof(out), &written), UNICODOID_OK);
    CHECK(bytes_equal(out, written, jamo, sizeof(jamo)));
    CHECK_STATUS(unicodoid_normalize(utf8, jamo, sizeof(jamo), UNICODOID_NORM_NFC,
                                     out, sizeof(out), &written), UNICODOID_OK);
    CHECK(bytes_equal(out, written, hangul, sizeof(hangul)));

    /* Canonical ordering: a + acute (ccc 230) + cedilla (ccc 202). */
    const uint8_t unordered[] = { 0x61, 0xCC, 0x81, 0xCC, 0xA7 };
    const uint8_t ordered[] = { 0x61, 0xCC, 0xA7, 0xCC, 0x81 };
    CHECK_STATUS(unicodoid_normalize(utf8, unordered, sizeof(unordered), UNICODOID_NORM_NFD,
                                     out, sizeof(out), &written), UNICODOID_OK);
    CHECK(bytes_equal(out, written, ordered, sizeof(ordered)));

    /* Required size is reported. */
    size_t required = 0;
    CHECK_STATUS(unicodoid_normalize(utf8, decomposed, sizeof(decomposed), UNICODOID_NORM_NFC,
                                     NULL, 0, &required), UNICODOID_ERR_BUFFER_TOO_SMALL);
    CHECK(required == sizeof(composed));

    /* Normalization is opt-in. */
    const unicodoid_system_t* conlang = unicodoid_system_get_custom_conlang();
    const uint8_t conlang_bytes[] = { 'A' };
    CHECK_STATUS(unicodoid_normalize(conlang, conlang_bytes, 1, UNICODOID_NORM_NFC,
                                     out, sizeof(out), &written),
                 UNICODOID_ERR_UNSUPPORTED);
}

static size_t collect_segments(const unicodoid_system_t* sys,
                               const uint8_t* bytes, size_t len,
                               unicodoid_boundary_type_t type,
                               unicodoid_segment_t* segments, size_t max_segments) {
    unicodoid_segmenter_t seg;
    if (unicodoid_segmenter_init(&seg, sys, bytes, len, type) != UNICODOID_OK) return 0;
    size_t count = 0;
    while (count < max_segments && unicodoid_segmenter_next(&seg, &segments[count])) {
        ++count;
    }
    unicodoid_segmenter_free(&seg);
    return count;
}

static void test_segmentation(void) {
    const unicodoid_system_t* utf8 = unicodoid_system_get_utf8();

    const uint8_t graphemes[] = { 0x61, 0xCC, 0x81, 0x62 }; /* a + acute + b */
    unicodoid_segment_t segments[8];
    size_t count = collect_segments(utf8, graphemes, sizeof(graphemes),
                                    UNICODOID_BOUNDARY_GRAPHEME, segments, 8);
    CHECK(count == 2);
    CHECK(segments[0].byte_offset == 0 && segments[0].byte_length == 3);
    CHECK(segments[0].char_count == 2);
    CHECK(segments[1].byte_offset == 3 && segments[1].byte_length == 1);

    const uint8_t words[] = { 'h', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd' };
    count = collect_segments(utf8, words, sizeof(words), UNICODOID_BOUNDARY_WORD, segments, 8);
    CHECK(count == 3);
    CHECK(segments[0].byte_length == 5);
    CHECK(segments[1].byte_length == 1);
    CHECK(segments[2].byte_length == 5);

    const uint8_t sentence[] = { 'H', 'i', '.', ' ', 'B', 'y', 'e', '.' };
    count = collect_segments(utf8, sentence, sizeof(sentence),
                             UNICODOID_BOUNDARY_SENTENCE, segments, 8);
    CHECK(count == 2);
    CHECK(segments[0].byte_length == 4);
    CHECK(segments[1].byte_length == 4);

    const uint8_t line[] = { 'a', 'b', 'c', ' ', 'd', 'e', 'f' };
    count = collect_segments(utf8, line, sizeof(line), UNICODOID_BOUNDARY_LINE, segments, 8);
    /* UAX #14 LB18 breaks after spaces, so the space stays with "abc". */
    CHECK(count == 2);
    CHECK(segments[0].byte_length == 4);
    CHECK(segments[1].byte_length == 3);

    const unicodoid_system_t* conlang = unicodoid_system_get_custom_conlang();
    const uint8_t conlang_bytes[] = { 'A' };
    unicodoid_segmenter_t seg;
    CHECK_STATUS(unicodoid_segmenter_init(&seg, conlang, conlang_bytes, 1,
                                          UNICODOID_BOUNDARY_GRAPHEME),
                 UNICODOID_ERR_UNSUPPORTED);
}

static void test_collation(void) {
    const unicodoid_system_t* utf8 = unicodoid_system_get_utf8();

    const uint8_t a[] = { 'a' };
    const uint8_t b[] = { 'b' };
    const uint8_t upper_a[] = { 'A' };
    const uint8_t e[] = { 'e' };
    const uint8_t e_acute[] = { 0xC3, 0xA9 };
    const uint8_t file2[] = { 'f', 'i', 'l', 'e', '2' };
    const uint8_t file10[] = { 'f', 'i', 'l', 'e', '1', '0' };

    unicodoid_string_t sa = make_string(utf8, a, sizeof(a));
    unicodoid_string_t sb = make_string(utf8, b, sizeof(b));
    unicodoid_string_t s_upper = make_string(utf8, upper_a, sizeof(upper_a));
    unicodoid_string_t se = make_string(utf8, e, sizeof(e));
    unicodoid_string_t se_acute = make_string(utf8, e_acute, sizeof(e_acute));
    unicodoid_string_t s_file2 = make_string(utf8, file2, sizeof(file2));
    unicodoid_string_t s_file10 = make_string(utf8, file10, sizeof(file10));

    unicodoid_collation_options_t opts = unicodoid_collation_options_default();

    CHECK(unicodoid_collate(&sa, &sb, &opts) < 0);
    CHECK(unicodoid_collate(&sb, &sa, &opts) > 0);
    CHECK(unicodoid_collate(&sa, &sa, &opts) == 0);

    opts.strength = UNICODOID_COLLATE_PRIMARY;
    CHECK(unicodoid_collate(&sa, &s_upper, &opts) == 0);
    CHECK(unicodoid_collate(&se, &se_acute, &opts) == 0);

    opts = unicodoid_collation_options_default();
    CHECK(unicodoid_collate(&sa, &s_upper, &opts) < 0);
    CHECK(unicodoid_collate(&se, &se_acute, &opts) < 0);

    opts.numeric_ordering = true;
    CHECK(unicodoid_collate(&s_file2, &s_file10, &opts) < 0);
    opts.numeric_ordering = false;
    CHECK(unicodoid_collate(&s_file2, &s_file10, &opts) > 0);

    /* Sort keys must agree with the comparison. */
    opts = unicodoid_collation_options_default();
    size_t needed = 0;
    CHECK_STATUS(unicodoid_collation_sort_key(&sa, &opts, NULL, 0, &needed),
                 UNICODOID_ERR_BUFFER_TOO_SMALL);
    CHECK(needed > 0);

    uint8_t key_a[64];
    uint8_t key_b[64];
    size_t written_a = 0;
    size_t written_b = 0;
    CHECK_STATUS(unicodoid_collation_sort_key(&sa, &opts, key_a, sizeof(key_a), &written_a),
                 UNICODOID_OK);
    CHECK_STATUS(unicodoid_collation_sort_key(&sb, &opts, key_b, sizeof(key_b), &written_b),
                 UNICODOID_OK);
    CHECK(written_a > 0 && written_b > 0);
    CHECK(memcmp(key_a, key_b, written_a < written_b ? written_a : written_b) < 0);

    const unicodoid_system_t* conlang = unicodoid_system_get_custom_conlang();
    const uint8_t conlang_bytes[] = { 'A' };
    unicodoid_string_t conlang_str = make_string(conlang, conlang_bytes, 1);
    CHECK_STATUS(unicodoid_collation_sort_key(&conlang_str, &opts, key_a, sizeof(key_a),
                                              &written_a),
                 UNICODOID_ERR_UNSUPPORTED);

    unicodoid_string_free(&conlang_str);
    unicodoid_string_free(&s_file10);
    unicodoid_string_free(&s_file2);
    unicodoid_string_free(&se_acute);
    unicodoid_string_free(&se);
    unicodoid_string_free(&s_upper);
    unicodoid_string_free(&sb);
    unicodoid_string_free(&sa);
}

int main(void) {
    test_catalog();
    test_scalar_properties();
    test_normalization();
    test_segmentation();
    test_collation();

    if (g_failures == 0) {
        printf("test_unicode: %d checks passed\n", g_checks);
        return 0;
    }
    fprintf(stderr, "test_unicode: %d of %d checks failed\n", g_failures, g_checks);
    return 1;
}
