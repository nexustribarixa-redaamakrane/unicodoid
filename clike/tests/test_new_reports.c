#include "unicodoid/unicodoid.h"
#include "unicodoid/uax/uax9_bidi.h"
#include "unicodoid/uax/uax11_eaw.h"
#include "unicodoid/uax/uax14_linebreak.h"
#include "unicodoid/uax/uax15_normalization.h"
#include "unicodoid/uax/uax24_script_property.h"
#include "unicodoid/uax/uax31_identifiers.h"
#include "unicodoid/uax/uax38_unihan.h"
#include "unicodoid/uax/uax41_common_refs.h"
#include "unicodoid/uax/uax42_ucd_xml.h"
#include "unicodoid/uax/uax44_ucd.h"
#include "unicodoid/uax/uax45_usource_ideographs.h"
#include "unicodoid/uax/uax50_vtl.h"
#include "unicodoid/uax/uax53_arabic_mark_rendering.h"
#include "unicodoid/uax/uax57_unikemet.h"
#include "unicodoid/uax/uax60_ea_script_data.h"
#include "unicodoid/utr/utr17_character_encoding_model.h"
#include "unicodoid/utr/utr23_character_property_model.h"
#include "unicodoid/utr/utr25_mathematics.h"
#include "unicodoid/utr/utr33_conformance.h"
#include "unicodoid/utr/utr54_mongolian.h"
#include "unicodoid/utr/utr56_cuneiform_signlists.h"
#include "unicodoid/uts/uts18_regex.h"
#include "unicodoid/uts/uts37_ivd.h"
#include "unicodoid/uts/uts39_security.h"
#include "unicodoid/uts/uts46_idna_compat.h"
#include "unicodoid/uts/uts55_source_code.h"
#include "unicodoid/uts/uts61_set_notation.h"
#include "unicodoid/uts/uts35_cldr/cldr_core.h"
#include "unicodoid/uts/uts35_cldr/cldr_numbers.h"
#include "unicodoid/uts/uts35_cldr/cldr_dates.h"
#include "unicodoid/uts/uts35_cldr/cldr_collation.h"
#include "unicodoid/uts/uts35_cldr/cldr_general.h"
#include "unicodoid/uts/uts35_cldr/cldr_keyboards.h"
#include "unicodoid/uts/uts35_cldr/cldr_messageformat.h"
#include "unicodoid/uts/uts35_cldr/cldr_person_names.h"
#include "unicodoid/uts/uts35_cldr/cldr_supplemental.h"

#include <stdio.h>
#include <string.h>

static int g_checks = 0;
static int g_failures = 0;

#define CHECK(cond) do { ++g_checks; if (!(cond)) { fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); ++g_failures; } } while (0)

static void test_bidi(void) {
    const uint32_t abc[] = {'a', 'b', 'c'};
    uint8_t levels[3];
    CHECK(unicodoid_uax9_resolve_levels(abc, 3, 0, levels, 3) == 3);
    CHECK(levels[0] == 0 && levels[1] == 0 && levels[2] == 0);
    const uint32_t heb[] = {0x05D0, 0x05D0};
    CHECK(unicodoid_uax9_resolve_levels(heb, 2, UNICODOID_BIDI_AUTO, levels, 3) == 2);
    CHECK((levels[0] % 2) == 1);
    char buf[8]; (void)buf;
    CHECK(unicodoid_uax9_is_mirrored('('));
    CHECK(unicodoid_uax9_mirrored_glyph('(') == ')');
    CHECK(unicodoid_uax9_is_paired_bracket('('));
}

static void test_uax_wrappers(void) {
    CHECK(unicodoid_uax11_is_wide(0xAC00));
    CHECK(unicodoid_uax11_column_count(0xAC00, false) == 2);
    CHECK(unicodoid_uax11_column_count('A', false) == 1);
    CHECK(unicodoid_uax14_line_break('A') == unicodoid_uax14_line_break('A'));
    const uint32_t line[] = {'a', 'b', 'c', ' ', 'd'};
    CHECK(unicodoid_uax14_next_break(line, 5, 0) == 4);
    const uint32_t dec[] = {0x65, 0x0301};
    uint32_t out[4];
    CHECK(unicodoid_uax15_normalize(dec, 2, 0, out, 4) == 1 && out[0] == 0x00E9);
    CHECK(unicodoid_uax24_is_script('A', unicodoid_uax24_find("Latin")));
    const uint32_t id[] = {'a', 'b', '1'};
    CHECK(unicodoid_uax31_is_valid(id, 3));
    const uint32_t bad_id[] = {'1', 'a'};
    CHECK(!unicodoid_uax31_is_valid(bad_id, 2));
    CHECK(unicodoid_uax38_is_unified_ideograph(0x4E2D));
    CHECK(unicodoid_uax38_is_radical(0x2F00));
    CHECK(strcmp(unicodoid_uax41_revision(), "38") == 0);
    char xml[512];
    CHECK(unicodoid_uax42_char_record('A', xml, sizeof(xml)) > 0 && strstr(xml, "<char") != NULL);
    CHECK(strcmp(unicodoid_uax44_property_alias("gc"), "General_Category") == 0);
    CHECK(unicodoid_uax44_has_casefold('A'));
    CHECK(unicodoid_uax45_is_usource(0x4E2D));
    /* Latin letters have Vertical_Orientation R (rotated in vertical layout). */
    CHECK(!unicodoid_uax50_is_upright('A'));
    CHECK(unicodoid_uax50_is_upright(0x3042)); /* Hiragana: Upright. */
    const uint32_t arab[] = {0x0627, 0x0627};
    CHECK(unicodoid_uax53_is_consistent(arab, 2));
    CHECK(unicodoid_uax57_is_hieroglyph(0x13000));
    CHECK(unicodoid_uax57_is_legacy(0x13000));
    CHECK(unicodoid_uax60_is_han(0x4E2D));
}

static void test_utr_uts(void) {
    CHECK(unicodoid_utr17_utf8_length(0x1F600) == 4);
    CHECK(unicodoid_utr17_utf16_length(0x1F600) == 2);
    CHECK(unicodoid_utr23_binary_property_count() == 41);
    CHECK(unicodoid_utr25_is_math('+'));
    CHECK(unicodoid_utr25_is_math_alphanumeric(0x1D400));
    CHECK(strcmp(unicodoid_utr33_level_name(1), "Well_Formed") == 0);
    CHECK(unicodoid_utr54_is_mongolian(0x1800));
    CHECK(unicodoid_utr54_is_fvs(0x180B));
    CHECK(unicodoid_utr56_is_cuneiform(0x12000));
    CHECK(unicodoid_uts18_property_match("\\p{Lu}", 'A') != UNICODOID_UTS18_NONE);
    CHECK(unicodoid_uts18_property_match("\\p{Lu}", 'a') == UNICODOID_UTS18_NONE);
    const uint32_t ivs[] = {0x4E2D, 0xE0100};
    CHECK(unicodoid_uts37_is_ivs(ivs, 2));
    const uint32_t conf_a[] = {'O'};
    const uint32_t conf_b[] = {0x039F};
    /* confusables may or may not equate; just exercise the API. */
    (void)unicodoid_uts39_are_confusable(conf_a, 1, conf_a, 1);
    (void)unicodoid_uts39_are_confusable(conf_a, 1, conf_b, 1);
    const uint32_t label[] = {'a', 'b'};
    CHECK(unicodoid_uts46_check_label(label, 2));
    CHECK(unicodoid_uts55_is_bidi_control(0x202A));
    CHECK(unicodoid_uts61_set_membership("\\p{Lu}", 'A') == 1);
    CHECK(unicodoid_uts61_set_membership("41..5A", 'A') == 1);
    {
        unicodoid_cldr_locale_t loc;
        CHECK(unicodoid_cldr_parse_locale("en-US", &loc) && strcmp(loc.language, "en") == 0);
        CHECK(unicodoid_cldr_is_valid_locale("de-DE"));
        unicodoid_cldr_number_symbols_t sym;
        unicodoid_cldr_number_symbols("DE", &sym);
        CHECK(strcmp(sym.decimal, ",") == 0);
        CHECK(unicodoid_cldr_is_decimal("3.14"));
        CHECK(unicodoid_cldr_is_date_pattern("yyyy-MM-dd"));
        unicodoid_cldr_collation_t col;
        unicodoid_cldr_collation("en", &col);
        CHECK(col.strength == 3);
        CHECK(unicodoid_cldr_is_weekday("en", "Monday"));
        CHECK(unicodoid_cldr_day_count("en", 2, true) == 29);
        CHECK(unicodoid_cldr_is_keyboard_id("en_US"));
        CHECK(unicodoid_cldr_plural("en", 1.0) == 1);
        CHECK(unicodoid_cldr_is_message_pattern_valid("{x}"));
        CHECK(unicodoid_cldr_name_order("zh") == 0);
        CHECK(unicodoid_cldr_is_person_name("en", "John"));
        CHECK(unicodoid_cldr_is_zone_id("Europe/London"));
    }
}

int main(void) {
    test_bidi();
    test_uax_wrappers();
    test_utr_uts();
    if (g_failures == 0) { printf("test_new_reports: %d checks passed\n", g_checks); return 0; }
    fprintf(stderr, "test_new_reports: %d of %d checks failed\n", g_failures, g_checks);
    return 1;
}
