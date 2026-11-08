// Exercises the C++ UAX / UTS / UTR modules added for Unicode 18.0.0.
//
// The checks mirror the C suite in clike/tests/test_new_reports.c so the two
// language bindings stay in parity.

#include "unicodoid/unicodoid.hpp"

#include <cstdint>
#include <cstdio>
#include <string>
#include <string_view>
#include <vector>

namespace {

int g_checks = 0;
int g_failures = 0;

void check(bool condition, const char* what) {
    ++g_checks;
    if (condition) return;
    ++g_failures;
    std::fprintf(stderr, "FAIL: %s\n", what);
}

std::u32string u32(std::initializer_list<uint32_t> cps) {
    std::u32string out;
    for (const uint32_t cp : cps) out.push_back(static_cast<char32_t>(cp));
    return out;
}

void test_bidi() {
    const std::u32string abc = u32({'a', 'b', 'c'});
    const std::vector<uint8_t> levels =
        unicodoid::uax9::levels(abc, unicodoid::uax9::Ltr);
    check(levels.size() == 3 && levels[0] == 0 && levels[1] == 0 &&
              levels[2] == 0,
          "UAX9 LTR levels");

    const std::u32string hebrew = u32({0x05D0, 0x05D0});
    const std::vector<uint8_t> rtl_levels =
        unicodoid::uax9::levels(hebrew, unicodoid::uax9::Auto);
    check(rtl_levels.size() == 2 && (rtl_levels[0] % 2) == 1,
          "UAX9 auto RTL level");

    check(unicodoid::uax9::is_mirrored('('), "UAX9 '(' is mirrored");
    check(unicodoid::uax9::mirrored_glyph('(') == ')',
          "UAX9 '(' mirrors to ')'");
    check(unicodoid::uax9::is_paired_bracket('('),
          "UAX9 '(' is a paired bracket");
}

void test_uax_wrappers() {
    check(unicodoid::uax11::is_wide(0xAC00), "UAX11 Hangul is wide");
    check(unicodoid::uax11::column_count(0xAC00, false) == 2,
          "UAX11 Hangul column count");
    check(unicodoid::uax11::column_count('A', false) == 1,
          "UAX11 Latin column count");

    const std::u32string line = u32({'a', 'b', 'c', ' ', 'd'});
    check(unicodoid::uax14::next_break(line, 0) == 4,
          "UAX14 next break after the space");

    const std::u32string decomposed = u32({0x65, 0x0301});
    check(unicodoid::uax15::normalize(decomposed, unicodoid::uax15::Form::Nfc) ==
              u32({0x00E9}),
          "UAX15 NFC composition");

    const uint16_t latin =
        unicodoid::uax24::find("Latin");
    check(latin != 0 && unicodoid::uax24::is_script('A', latin),
          "UAX24 Latin script");
    check(unicodoid::uax24::script_name(latin) == "Latin",
          "UAX24 Latin script name");

    const std::u32string identifier = u32({'a', 'b', '1'});
    check(unicodoid::uax31::is_valid(identifier), "UAX31 valid identifier");
    check(!unicodoid::uax31::is_valid(u32({'1', 'a'})),
          "UAX31 invalid identifier");

    check(unicodoid::uax38::is_unified_ideograph(0x4E2D),
          "UAX38 unified ideograph");
    check(unicodoid::uax38::is_radical(0x2F00), "UAX38 radical");
    check(unicodoid::uax41::revision() == "38", "UAX41 revision");
    check(unicodoid::uax42::char_record('A').find("<char") != std::string::npos,
          "UAX42 char record");
    check(unicodoid::uax44::block_name(unicodoid::uax44::block('A')) ==
              "Basic Latin",
          "UAX44 block name");
    check(!unicodoid::uax44::to_casefold('A').empty(),
          "UAX44 casefold coverage");
    check(unicodoid::uax45::is_usource(0x4E2D), "UAX45 U-source");
    check(!unicodoid::uax50::is_upright('A'), "UAX50 Latin is rotated");
    check(unicodoid::uax50::is_upright(0x3042), "UAX50 Hiragana is upright");
    check(unicodoid::uax53::is_consistent(u32({0x0627, 0x0627})),
          "UAX53 joining consistency");
    check(unicodoid::uax57::is_hieroglyph(0x13000), "UAX57 hieroglyph");
    check(unicodoid::uax57::is_legacy(0x13000), "UAX57 legacy sign");
    check(unicodoid::uax60::is_han(0x4E2D), "UAX60 Han script");
}

void test_utr_uts() {
    check(unicodoid::utr17::utf8_length(0x1F600) == 4,
          "UTR17 UTF-8 length");
    check(unicodoid::utr17::utf16_length(0x1F600) == 2,
          "UTR17 UTF-16 length");
    check(unicodoid::utr23::binary_property_count() == 41,
          "UTR23 binary property count");
    check(unicodoid::utr25::is_math('+'), "UTR25 math operator");
    check(unicodoid::utr25::is_math_alphanumeric(0x1D400),
          "UTR25 math alphanumeric");
    check(unicodoid::utr33::level_name(1) == "Well_Formed",
          "UTR33 conformance names");
    check(unicodoid::utr54::is_mongolian(0x1800), "UTR54 Mongolian block");
    check(unicodoid::utr54::is_fvs(0x180B), "UTR54 FVS");
    check(unicodoid::utr56::is_cuneiform(0x12000), "UTR56 cuneiform");

    check(unicodoid::uts18::property_match("\\p{Lu}", 'A') !=
              unicodoid::uts18::Match::None,
          "UTS18 uppercase property match");
    check(unicodoid::uts18::property_match("\\p{Lu}", 'a') ==
              unicodoid::uts18::Match::None,
          "UTS18 uppercase property non-match");
    check(unicodoid::uts37::is_ivs(u32({0x4E2D, 0xE0100})),
          "UTS37 ideographic variation sequence");
    check(unicodoid::uts39::are_confusable(u32({'O'}), u32({'O'})),
          "UTS39 identical skeletons");
    check(unicodoid::uts46::check_label(u32({'a', 'b'})),
          "UTS46 label check");
    check(unicodoid::uts55::is_bidi_control(0x202A),
          "UTS55 bidi control");
    check(unicodoid::uts61::set_membership("\\p{Lu}", 'A') == 1,
          "UTS61 property set membership");
    check(unicodoid::uts61::set_membership("41..5A", 'A') == 1,
          "UTS61 range set membership");

    unicodoid::uts35::Locale locale;
    check(unicodoid::uts35::parse_locale("en-US", locale) &&
              locale.language == "en" && locale.region == "US",
          "CLDR locale parsing");
    check(unicodoid::uts35::is_valid_locale("de-DE"), "CLDR valid locale");
    const unicodoid::uts35::NumberSymbols symbols =
        unicodoid::uts35::number_symbols("DE");
    check(symbols.decimal == ",", "CLDR German decimal separator");
    check(unicodoid::uts35::is_decimal("3.14"), "CLDR decimal syntax");
    check(unicodoid::uts35::is_date_pattern("yyyy-MM-dd"),
          "CLDR date pattern");
    check(unicodoid::uts35::collation("en").strength == 3,
          "CLDR collation strength");
    check(unicodoid::uts35::is_weekday("en", "Monday"),
          "CLDR weekday name");
    check(unicodoid::uts35::day_count("en", 2, true) == 29,
          "CLDR leap February");
    check(unicodoid::uts35::is_keyboard_id("en_US"),
          "CLDR keyboard id");
    check(unicodoid::uts35::plural("en", 1.0) ==
              unicodoid::uts35::Plural::One,
          "CLDR plural category");
    check(unicodoid::uts35::is_message_pattern_valid("{x}"),
          "CLDR message pattern");
    check(unicodoid::uts35::name_order("zh") ==
              unicodoid::uts35::NameOrder::FamilyFirst,
          "CLDR family-first name order");
    check(unicodoid::uts35::is_person_name("en", "John"),
          "CLDR person name");
    check(unicodoid::uts35::is_zone_id("Europe/London"),
          "CLDR zone id");
}

} // namespace

int main() {
    try {
        test_bidi();
        test_uax_wrappers();
        test_utr_uts();
    } catch (const std::exception& ex) {
        std::fprintf(stderr, "unexpected exception: %s\n", ex.what());
        return 1;
    }

    if (g_failures == 0) {
        std::printf("test_cpp_new_reports: %d checks passed\n", g_checks);
        return 0;
    }
    std::fprintf(stderr, "test_cpp_new_reports: %d of %d checks failed\n",
                 g_failures, g_checks);
    return 1;
}
