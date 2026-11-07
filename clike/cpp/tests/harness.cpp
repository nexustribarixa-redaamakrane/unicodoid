// Harness for the ConvertUTF native C++ port.
//
// Covers well-formedness, strict/lenient behavior, exhaustion, and
// cross-encoding equivalence for the Unicode 18.0.0 ConvertUTF C++ port.
// This exercises the native std::vector API, not the C port.

#include "unicodoid/ConvertUTF.hpp"

#include <cstdint>
#include <cstdio>
#include <vector>

namespace {

size_t gFailures = 0;
size_t gChecks = 0;

void check(bool condition, const char* what) {
    ++gChecks;
    if (!condition) {
        ++gFailures;
        std::fprintf(stderr, "FAIL: %s\n", what);
    }
}

using unicodoid::convertutf::ConversionFlags;
using unicodoid::convertutf::ConversionResult;

}  // namespace

int main() {
    constexpr auto kStrict = ConversionFlags::StrictConversion;
    constexpr auto kLenient = ConversionFlags::LenientConversion;

    {
        std::vector<std::uint8_t> src = { 0x41 };
        ConversionResult result = ConversionResult::ConversionOk;
        auto dst = unicodoid::convertutf::utf8ToUtf16(src, kStrict, &result);
        check(result == ConversionResult::ConversionOk, "utf8 to utf16 A ok");
        check(dst.size() == 1 && dst[0] == 0x0041, "utf8 to utf16 A value");
    }
    {
        std::vector<std::uint8_t> src = { 0xC3, 0xA9 };
        auto dst = unicodoid::convertutf::utf8ToUtf16(src, kStrict);
        check(dst.size() == 1 && dst[0] == 0x00E9, "utf8 to utf16 e-acute");
    }
    {
        std::vector<std::uint8_t> src = { 0xE6, 0xBC, 0xA2 };
        auto dst = unicodoid::convertutf::utf8ToUtf16(src, kStrict);
        check(dst.size() == 1 && dst[0] == 0x6F22, "utf8 to utf16 han");
    }
    {
        std::vector<std::uint8_t> src = { 0xF0, 0x9F, 0x98, 0x80 };
        ConversionResult result = ConversionResult::ConversionOk;
        auto dst = unicodoid::convertutf::utf8ToUtf16(src, kStrict, &result);
        check(result == ConversionResult::ConversionOk, "utf8 to utf16 emoji ok");
        check(dst.size() == 2 && dst[0] == 0xD83D && dst[1] == 0xDE00,
              "utf8 to utf16 emoji pair");
    }
    {
        std::vector<std::uint8_t> src = { 0x41, 0xE6, 0xBC, 0xA2 };
        auto dst = unicodoid::convertutf::utf8ToUtf16(src, kStrict);
        check(dst.size() == 2 && dst[0] == 0x0041 && dst[1] == 0x6F22,
              "utf8 to utf16 offset decode");
    }
    {
        std::vector<std::uint8_t> src = { 0xC0, 0x80 };
        ConversionResult result = ConversionResult::ConversionOk;
        auto dst = unicodoid::convertutf::utf8ToUtf16(src, kStrict, &result);
        check(result == ConversionResult::SourceIllegal, "overlong illegal");
        check(dst.empty(), "overlong empty on error");
    }
    {
        std::vector<std::uint8_t> src = { 0xED, 0xA0, 0x80 };
        ConversionResult result = ConversionResult::ConversionOk;
        unicodoid::convertutf::utf8ToUtf16(src, kStrict, &result);
        check(result == ConversionResult::SourceIllegal, "encoded surrogate illegal");
    }
    {
        std::vector<std::uint8_t> src = { 0xE2, 0x82 };
        ConversionResult result = ConversionResult::ConversionOk;
        unicodoid::convertutf::utf8ToUtf16(src, kStrict, &result);
        check(result == ConversionResult::SourceExhausted, "truncated exhausted");
    }
    {
        std::vector<std::uint8_t> src = { 0xF4, 0x90, 0x80, 0x80 };
        ConversionResult result = ConversionResult::ConversionOk;
        unicodoid::convertutf::utf8ToUtf16(src, kStrict, &result);
        check(result == ConversionResult::SourceIllegal, "above max illegal");
    }
    {
        std::vector<std::uint16_t> src = { 0xD83D, 0xDE00 };
        ConversionResult result = ConversionResult::ConversionOk;
        auto dst = unicodoid::convertutf::utf16ToUtf8(src, kStrict, &result);
        check(result == ConversionResult::ConversionOk, "utf16 to utf8 pair ok");
        check(dst.size() == 4 && dst[0] == 0xF0 && dst[1] == 0x9F && dst[2] == 0x98 &&
                  dst[3] == 0x80,
              "utf16 to utf8 pair bytes");
    }
    {
        std::vector<std::uint16_t> src = { 0xD800 };
        ConversionResult result = ConversionResult::ConversionOk;
        unicodoid::convertutf::utf16ToUtf8(src, kStrict, &result);
        check(result == ConversionResult::SourceExhausted, "lone high exhausted");
    }
    {
        std::vector<std::uint16_t> src = { 0xD800, 0x0041 };
        ConversionResult result = ConversionResult::ConversionOk;
        unicodoid::convertutf::utf16ToUtf8(src, kStrict, &result);
        check(result == ConversionResult::SourceIllegal, "unpaired high illegal");
        auto dst = unicodoid::convertutf::utf16ToUtf8(src, kLenient, &result);
        check(result == ConversionResult::ConversionOk, "unpaired high lenient ok");
        check(!dst.empty(), "unpaired high lenient produced bytes");
    }
    {
        std::vector<std::uint32_t> src = { 0x1F600 };
        auto dst = unicodoid::convertutf::utf32ToUtf8(src, kStrict);
        check(dst.size() == 4 && dst[0] == 0xF0 && dst[1] == 0x9F && dst[2] == 0x98 &&
                  dst[3] == 0x80,
              "utf32 to utf8 emoji");
    }
    {
        std::vector<std::uint32_t> src = { 0x10FFFF };
        auto dst = unicodoid::convertutf::utf32ToUtf8(src, kStrict);
        check(dst.size() == 4 && dst[0] == 0xF4 && dst[1] == 0x8F && dst[2] == 0xBF &&
                  dst[3] == 0xBF,
              "utf32 to utf8 max");
    }
    {
        std::vector<std::uint32_t> src = { 0xD800 };
        ConversionResult result = ConversionResult::ConversionOk;
        unicodoid::convertutf::utf32ToUtf8(src, kStrict, &result);
        check(result == ConversionResult::SourceIllegal, "utf32 surrogate strict illegal");
        auto dst = unicodoid::convertutf::utf32ToUtf8(src, kLenient, &result);
        check(result == ConversionResult::ConversionOk, "utf32 surrogate lenient ok");
        check(dst.size() == 3 && dst[0] == 0xED && dst[1] == 0xA0 && dst[2] == 0x80,
              "utf32 surrogate lenient bytes");
    }
    {
        std::vector<std::uint32_t> src = { 0x110000 };
        ConversionResult result = ConversionResult::ConversionOk;
        unicodoid::convertutf::utf32ToUtf8(src, kStrict, &result);
        check(result == ConversionResult::SourceIllegal, "utf32 above max illegal");
        auto dst = unicodoid::convertutf::utf32ToUtf8(src, kLenient, &result);
        check(result == ConversionResult::SourceIllegal,
              "utf32 above max lenient still illegal (replacement emitted)");
        check(dst.size() == 3 && dst[0] == 0xEF && dst[1] == 0xBF && dst[2] == 0xBD,
              "utf32 above max replacement bytes");
    }
    {
        std::vector<std::uint8_t> src = { 0xF0, 0x9F, 0x98, 0x80 };
        auto dst = unicodoid::convertutf::utf8ToUtf32(src, kStrict);
        check(dst.size() == 1 && dst[0] == 0x1F600, "utf8 to utf32 emoji");
    }
    {
        std::vector<std::uint16_t> src = { 0xD83D, 0xDE00 };
        auto dst = unicodoid::convertutf::utf16ToUtf32(src, kStrict);
        check(dst.size() == 1 && dst[0] == 0x1F600, "utf16 to utf32 pair");
    }
    {
        std::vector<std::uint32_t> src = { 0x1F600 };
        auto dst = unicodoid::convertutf::utf32ToUtf16(src, kStrict);
        check(dst.size() == 2 && dst[0] == 0xD83D && dst[1] == 0xDE00,
              "utf32 to utf16 pair");
    }
    {
        std::vector<std::uint32_t> src = { 0xD800 };
        ConversionResult result = ConversionResult::ConversionOk;
        unicodoid::convertutf::utf32ToUtf16(src, kStrict, &result);
        check(result == ConversionResult::SourceIllegal, "utf32 to utf16 surrogate illegal");
        auto dst = unicodoid::convertutf::utf32ToUtf16(src, kLenient, &result);
        check(result == ConversionResult::ConversionOk, "utf32 to utf16 lenient ok");
        check(dst.size() == 1 && dst[0] == 0xFFFD, "utf32 to utf16 replacement");
    }
    {
        check(unicodoid::convertutf::isLegalUtf8({ 0x41 }, 0), "isLegal ascii");
        check(!unicodoid::convertutf::isLegalUtf8({ 0xC0, 0x80 }, 0), "isLegal overlong");
        check(!unicodoid::convertutf::isLegalUtf8({ 0xE2, 0x82 }, 0), "isLegal truncated");
        check(unicodoid::convertutf::isLegalUtf8({ 0xC3, 0xA9 }, 0), "isLegal e-acute");
        check(unicodoid::convertutf::isLegalUtf8({ 0xF0, 0x9F, 0x98, 0x80 }, 0),
              "isLegal emoji");
        check(!unicodoid::convertutf::isLegalUtf8({ 0xF4, 0x90, 0x80, 0x80 }, 0),
              "isLegal above max");
        check(!unicodoid::convertutf::isLegalUtf8({ 0xED, 0xA0, 0x80 }, 0),
              "isLegal surrogate");
        std::vector<std::uint8_t> offset = { 0x41, 0xC3, 0xA9 };
        check(unicodoid::convertutf::isLegalUtf8(offset, 1), "isLegal offset e-acute");
        check(unicodoid::convertutf::kMaxLegalUtf32 == 0x10FFFF, "legal max");
    }
    {
        // Round trips preserve bytes / code units.
        std::vector<std::uint8_t> original = { 0x41, 0xC3, 0xA9, 0xE6, 0xBC, 0xA2, 0xF0,
                                               0x9F, 0x98, 0x80 };
        auto utf16 = unicodoid::convertutf::utf8ToUtf16(original, kStrict);
        auto back = unicodoid::convertutf::utf16ToUtf8(utf16, kStrict);
        check(back == original, "utf8 round trip via utf16");
        auto utf32 = unicodoid::convertutf::utf8ToUtf32(original, kStrict);
        auto back32 = unicodoid::convertutf::utf32ToUtf8(utf32, kStrict);
        check(back32 == original, "utf8 round trip via utf32");
        auto utf32z = unicodoid::convertutf::utf16ToUtf32(utf16, kStrict);
        auto back16 = unicodoid::convertutf::utf32ToUtf16(utf32z, kStrict);
        check(back16 == utf16, "utf16 round trip via utf32");
    }

    if (gFailures == 0) {
        std::printf("test_convertutf_cpp: %zu checks, PASS\n", gChecks);
        return 0;
    }
    std::fprintf(stderr, "test_convertutf_cpp: %zu of %zu checks failed\n", gFailures, gChecks);
    return 1;
}
