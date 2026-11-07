#include "unicodoid/unicodoid.hpp"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

static int g_checks = 0;
static int g_failures = 0;

#define CHECK(cond)                                                              \
    do {                                                                         \
        ++g_checks;                                                              \
        if (!(cond)) {                                                           \
            std::fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
            ++g_failures;                                                        \
        }                                                                        \
    } while (0)

static bool bytes_equal(const std::vector<uint8_t>& a,
                        const uint8_t* b, std::size_t b_len) {
    return a.size() == b_len && (b_len == 0 || std::memcmp(a.data(), b, b_len) == 0);
}

static void test_metadata(void) {
    using namespace unicodoid;

    Registry::init();
    CHECK(Registry::count() >= 8);

    EncodingSystem utf8 = Registry::utf8();
    CHECK(utf8.id() == "org.unicode.utf8");
    CHECK(utf8.name() == "UTF-8");
    CHECK(utf8.has_unicode_profile());
    CHECK(utf8.unicode_profile().major_version() == 18);
    CHECK(utf8.unicode_profile().supports_uax(UNICODOID_UAX_15_NORMALIZATION));

    auto scalar = utf8.unicode_profile().to_unicode(0x00E9);
    CHECK(scalar.has_value() && *scalar == 0x00E9);
    auto native = utf8.unicode_profile().from_unicode(0x00E9);
    CHECK(native.has_value() && *native == 0x00E9);

    EncodingSystem conlang = Registry::custom_conlang();
    CHECK(!conlang.has_unicode_profile());
    CHECK(!conlang.unicode_profile().to_unicode('A').has_value());

    EncodingSystem shift = Registry::shiftmodal16();
    CHECK(shift.model() == UNICODOID_MODEL_MODAL_STATEFUL);
}

static void test_encoding_system(void) {
    using namespace unicodoid;

    EncodingSystem utf8 = Registry::utf8();
    std::vector<uint8_t> e_acute = utf8.encode(0x00E9);
    const uint8_t expected[] = { 0xC3, 0xA9 };
    CHECK(bytes_equal(e_acute, expected, sizeof(expected)));

    auto decoded = utf8.decode(e_acute.data(), e_acute.size());
    CHECK(decoded.first == 0x00E9 && decoded.second == 2);

    CHECK(utf8.validate(e_acute.data(), e_acute.size()));
    const uint8_t bad[] = { 0xC0, 0x80 };
    CHECK(!utf8.validate(bad, sizeof(bad)));
    CHECK(utf8.char_count(e_acute.data(), e_acute.size()) == 1);
}

static void test_text_string(void) {
    using namespace unicodoid;

    EncodingSystem utf8 = Registry::utf8();
    TextString s(utf8);
    s.append('H');
    s.append('i');
    s.append(0x1F600);

    CHECK(s.byte_length() == 6);
    CHECK(s.char_length() == 3);
    CHECK(s.char_at(0) == 'H');
    CHECK(s.char_at(2) == 0x1F600);

    TextString copy(s);
    CHECK(copy == s);

    TextString moved(std::move(copy));
    CHECK(moved == s);
    CHECK(moved.char_length() == 3);

    TextString assigned(utf8);
    assigned = std::move(moved);
    CHECK(assigned == s);
    CHECK(assigned.char_length() == 3);

    const uint8_t raw[] = { 'A', 0x00, 'B' };
    TextString slice(utf8, raw, sizeof(raw));
    CHECK(slice.byte_length() == 3);
    CHECK(slice.char_length() == 3);
    CHECK(slice.char_at(1) == 0);
}

static void test_codec(void) {
    using namespace unicodoid;

    EncodingSystem shift = Registry::shiftmodal16();
    Codec codec(shift);

    auto a = codec.encode_char(0x41);
    CHECK(a.size() == 1 && a[0] == 0x41);

    auto shifted = codec.encode_char(0x4000);
    const uint8_t expected[] = { 0x0E, 0x00 };
    CHECK(bytes_equal(shifted, expected, sizeof(expected)));

    auto next = codec.encode_char(0x4001);
    CHECK(next.size() == 1 && next[0] == 0x01);

    std::vector<uint8_t> stream = { 0x41, 0x0E, 0x00, 0x01 };
    codec.reset();
    auto chars = codec.decode(stream.data(), stream.size());
    CHECK(chars.size() == 3);
    CHECK(chars[0] == 0x41 && chars[1] == 0x4000 && chars[2] == 0x4001);

    codec.reset();
    std::vector<unicodoid_char64_t> values = { 0x41, 0x4000, 0x4001 };
    auto encoded = codec.encode(values);
    CHECK(bytes_equal(encoded, stream.data(), stream.size()));
}

static void test_normalizer(void) {
    using namespace unicodoid;

    EncodingSystem utf8 = Registry::utf8();
    const uint8_t decomposed[] = { 0x65, 0xCC, 0x81 };
    TextString src(utf8, decomposed, sizeof(decomposed));

    TextString nfc = Normalizer::normalize(src, UNICODOID_NORM_NFC);
    const uint8_t composed[] = { 0xC3, 0xA9 };
    CHECK(nfc.byte_length() == sizeof(composed));
    CHECK(std::memcmp(nfc.data(), composed, sizeof(composed)) == 0);

    TextString nfd = Normalizer::normalize(nfc, UNICODOID_NORM_NFD);
    CHECK(nfd.byte_length() == sizeof(decomposed));
    CHECK(std::memcmp(nfd.data(), decomposed, sizeof(decomposed)) == 0);
}

static void test_transcoder(void) {
    using namespace unicodoid;

    EncodingSystem utf8 = Registry::utf8();
    EncodingSystem utf16 = Registry::utf16le();

    const uint8_t text[] = { 'h', 0xC3, 0xA9, 'l', 'l', 'o' };
    TextString src(utf8, text, sizeof(text));

    TextString mid = Transcoder::transcode(src, utf16);
    TextString back = Transcoder::transcode(mid, utf8);
    CHECK(back == src);
    CHECK(back.byte_length() == sizeof(text));

    EncodingSystem ascii = Registry::ascii();
    TextString replaced = Transcoder::transcode(src, ascii, UNICODOID_POLICY_REPLACE);
    CHECK(replaced.byte_length() == 5);
    CHECK(replaced.data()[1] == '?');
}

static void test_collator(void) {
    using namespace unicodoid;

    EncodingSystem utf8 = Registry::utf8();
    const uint8_t a[] = { 'a' };
    const uint8_t b[] = { 'b' };
    const uint8_t upper_a[] = { 'A' };

    TextString sa(utf8, a, sizeof(a));
    TextString sb(utf8, b, sizeof(b));
    TextString s_upper(utf8, upper_a, sizeof(upper_a));

    CHECK(Collator::collate(sa, sb) < 0);
    CHECK(Collator::collate(sa, sa) == 0);
    CHECK(Collator::collate(sa, s_upper, UNICODOID_COLLATE_PRIMARY) == 0);
    CHECK(Collator::collate(sa, s_upper) < 0);
}

int main() {
    try {
        test_metadata();
        test_encoding_system();
        test_text_string();
        test_codec();
        test_normalizer();
        test_transcoder();
        test_collator();
    } catch (const unicodoid::Exception& ex) {
        std::fprintf(stderr, "unexpected exception: %s (status %d)\n",
                     ex.what(), static_cast<int>(ex.status));
        return 1;
    }

    if (g_failures == 0) {
        std::printf("test_cpp: %d checks passed\n", g_checks);
        return 0;
    }
    std::fprintf(stderr, "test_cpp: %d of %d checks failed\n", g_failures, g_checks);
    return 1;
}
