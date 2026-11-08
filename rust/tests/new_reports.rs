//! Integration tests for the newly implemented UAX/UTR/UTS modules
//! (mirrors `clike/tests/test_new_reports.c`).

use unicodoid::uax::{uax9, uax11, uax14, uax24, uax31, uax50};
use unicodoid::uts::{cldr, uts18, uts61};
use unicodoid::utr::{utr17, utr25};

#[test]
fn bidi_levels() {
    let latin = [0x61u32, 0x62, 0x63];
    assert!(uax9::resolve_levels(&latin, 0).iter().all(|&l| l == 0));
    let hebrew = [0x05D0u32, 0x05D0];
    assert!(uax9::resolve_levels(&hebrew, uax9::dir::AUTO)
        .iter()
        .all(|l| l % 2 == 1));
    assert!(uax9::is_mirrored(0x28));
    assert_eq!(uax9::mirrored_glyph(0x28), 0x29);
}

#[test]
fn widths_and_breaks() {
    assert!(uax11::is_wide(0xAC00));
    assert_eq!(uax11::column_count(0xAC00, false), 2);
    assert_eq!(uax11::column_count(0x41, false), 1);
    let line = [0x61u32, 0x62, 0x63, 0x20, 0x64];
    assert_eq!(uax14::next_break(&line, 0), 4);
    assert_eq!(uax24::find("Latin"), uax24::script(0x41));
    assert!(uax31::is_valid(&[0x61, 0x62, 0x31]));
    assert!(!uax31::is_valid(&[0x31, 0x61]));
    assert!(!uax50::is_upright(0x41));
    assert!(uax50::is_upright(0x3042));
}

#[test]
fn misc_reports() {
    assert_eq!(utr17::utf8_length(0x1F600), 4);
    assert_eq!(utr17::utf16_length(0x1F600), 2);
    assert!(utr25::is_math(0x2B));
    assert!(utr25::is_math_alphanumeric(0x1D400));
    assert_ne!(uts18::property_match(r"\p{Lu}", 0x41), uts18::kind::NONE);
    assert_eq!(uts18::property_match(r"\p{Lu}", 0x61), uts18::kind::NONE);
    assert_eq!(uts61::set_membership(r"\p{Lu}", 0x41), 1);
    assert_eq!(uts61::set_membership("41..5A", 0x41), 1);
}

#[test]
fn cldr_core() {
    let loc = cldr::parse_locale("en-US").expect("valid locale");
    assert_eq!(loc.language, "en");
    assert!(cldr::is_valid_locale("de-DE"));
    assert_eq!(cldr::number_symbols(Some("DE")).decimal, ",");
    assert!(cldr::is_decimal("3.14"));
    assert!(cldr::is_date_pattern("yyyy-MM-dd"));
    assert_eq!(cldr::collation("en").strength, 3);
    assert!(cldr::is_weekday("Monday"));
    assert_eq!(cldr::day_count(2, true), 29);
    assert!(cldr::is_keyboard_id("en_US"));
    assert_eq!(cldr::plural("en", 1.0), cldr::plural::ONE);
    assert!(cldr::is_message_pattern_valid("{x}"));
    assert_eq!(cldr::name_order("zh"), cldr::name_order::FAMILY_FIRST);
    assert!(cldr::is_person_name("John"));
    assert!(cldr::is_zone_id("Europe/London"));
}
