//! Unicode catalog, property, normalization, segmentation, and collation tests.

use unicodoid::properties::{
    Category, EastAsianWidth, combining_class, east_asian_width, general_category, is_alphabetic,
    is_control, is_mark, is_numeric, is_punctuation, is_whitespace, system_general_category,
    system_to_uppercase, to_lowercase, to_uppercase,
};
use unicodoid::{
    BoundaryType, CollationOptions, CollationStrength, NormalizationForm, Segment, Segmenter,
    TextString, UnicodeDocCategory, collate, conlang_system, get_spec_ref, is_normalized,
    normalize, normalize_into, sort_key, sort_key_into, spec_at, spec_count, utf8_system,
};

fn collect_segments(
    system: &dyn unicodoid::EncodingSystem,
    bytes: &[u8],
    boundary: BoundaryType,
) -> Vec<Segment> {
    Segmenter::new(system, bytes, boundary).unwrap().collect()
}

#[test]
fn catalog_is_accurate_and_bounded() {
    assert!(spec_count() > 10);

    let first = spec_at(0).unwrap();
    assert!(!first.title.is_empty());
    assert!(!first.url.is_empty());

    let normalization = get_spec_ref(UnicodeDocCategory::Uax, 15).unwrap();
    assert_eq!(normalization.category, UnicodeDocCategory::Uax);
    assert!(normalization.title.contains("Normalization"));

    let identifiers = get_spec_ref(UnicodeDocCategory::Uax, 31).unwrap();
    assert!(identifiers.title.contains("Identifiers"));

    let indic = get_spec_ref(UnicodeDocCategory::Utn, 1).unwrap();
    assert!(indic.title.contains("Indic Language Collation"));

    assert!(get_spec_ref(UnicodeDocCategory::Uax, 999).is_none());
    assert!(spec_at(spec_count()).is_none());
}

#[test]
fn generated_data_version_is_unicode_18() {
    // The generated Unicode tables and the encoding profiles must both report
    // the Unicode 18.0.0 baseline.
    assert_eq!(unicodoid::ucd::VERSION, "18.0.0");
    assert_eq!(unicodoid::generated::ucd_data::UCD_VERSION, "18.0.0");
    assert_eq!(unicodoid::generated::ucd_data::UCD_GENERATOR, "1.2.0");

    let utf8 = unicodoid::utf8_system();
    assert!(utf8.has_unicode_profile());
    let profile = utf8.unicode_profile().expect("utf8 has a unicode profile");
    assert_eq!(profile.version, unicodoid::UnicodeVersion::new(18, 0, 0));
}

#[test]
fn scalar_properties() {
    assert_eq!(general_category(u32::from(b'A')), Category::UppercaseLetter);
    assert_eq!(general_category(u32::from(b'a')), Category::LowercaseLetter);
    assert_eq!(general_category(u32::from(b'1')), Category::DecimalNumber);
    assert_eq!(general_category(u32::from(b' ')), Category::SpaceSeparator);
    assert_eq!(general_category(0x0301), Category::Mark);
    assert_eq!(general_category(0x000A), Category::Control);

    assert!(is_alphabetic(u32::from(b'A')));
    assert!(!is_alphabetic(u32::from(b'1')));
    assert!(is_numeric(u32::from(b'9')));
    assert!(is_whitespace(u32::from(b'\t')));
    assert!(is_whitespace(u32::from(b' ')));
    assert!(is_control(0x000A));
    assert!(is_punctuation(u32::from(b'!')));
    assert!(is_mark(0x0301));

    assert_eq!(combining_class(0x0301), 230);
    assert_eq!(combining_class(u32::from(b'a')), 0);

    assert_eq!(to_uppercase(u32::from(b'a')), u32::from(b'A'));
    assert_eq!(to_lowercase(u32::from(b'A')), u32::from(b'a'));
    assert_eq!(east_asian_width(u32::from(b'A')), EastAsianWidth::Narrow);
    assert_eq!(east_asian_width(0xAC00), EastAsianWidth::Wide);

    let utf8 = utf8_system();
    assert_eq!(
        system_general_category(utf8, u64::from(b'A')).unwrap(),
        Category::UppercaseLetter
    );
    assert_eq!(
        system_to_uppercase(utf8, u64::from(b'a')).unwrap(),
        u64::from(b'A')
    );

    let conlang = conlang_system();
    assert_eq!(
        system_general_category(conlang, u64::from(b'A')).unwrap_err(),
        unicodoid::Status::Unsupported
    );
}

#[test]
fn normalization_forms() {
    let utf8 = utf8_system();
    let decomposed = [0x65, 0xCC, 0x81]; // e + combining acute
    let composed = [0xC3, 0xA9]; // é

    assert!(!is_normalized(utf8, &decomposed, NormalizationForm::Nfc));
    assert!(is_normalized(utf8, &composed, NormalizationForm::Nfc));

    let nfc = normalize(utf8, &decomposed, NormalizationForm::Nfc).unwrap();
    assert_eq!(nfc, composed);

    let nfd = normalize(utf8, &composed, NormalizationForm::Nfd).unwrap();
    assert_eq!(nfd, decomposed);

    // Compatibility ligature fi.
    let ligature = [0xEF, 0xAC, 0x81]; // U+FB01
    let fi = [0x66, 0x69];
    assert_eq!(
        normalize(utf8, &ligature, NormalizationForm::Nfkd).unwrap(),
        fi
    );
    assert_eq!(
        normalize(utf8, &ligature, NormalizationForm::Nfc).unwrap(),
        ligature
    );

    // Hangul syllable decomposition and recomposition.
    let hangul = [0xEA, 0xB0, 0x80]; // U+AC00
    let jamo = [0xE1, 0x84, 0x80, 0xE1, 0x85, 0xA1];
    assert_eq!(
        normalize(utf8, &hangul, NormalizationForm::Nfd).unwrap(),
        jamo
    );
    assert_eq!(
        normalize(utf8, &jamo, NormalizationForm::Nfc).unwrap(),
        hangul
    );

    // Canonical ordering: a + acute (ccc 230) + cedilla (ccc 202).
    let unordered = [0x61, 0xCC, 0x81, 0xCC, 0xA7];
    let ordered = [0x61, 0xCC, 0xA7, 0xCC, 0x81];
    assert_eq!(
        normalize(utf8, &unordered, NormalizationForm::Nfd).unwrap(),
        ordered
    );

    let mut out = [0u8; 32];
    let written = normalize_into(utf8, &decomposed, NormalizationForm::Nfc, &mut out).unwrap();
    assert_eq!(&out[..written], &composed);
    assert_eq!(
        normalize_into(utf8, &decomposed, NormalizationForm::Nfc, &mut []).unwrap_err(),
        unicodoid::Status::BufferTooSmall
    );

    // Normalization is opt-in.
    let conlang = conlang_system();
    assert_eq!(
        normalize(conlang, b"A", NormalizationForm::Nfc).unwrap_err(),
        unicodoid::Status::Unsupported
    );
    assert_eq!(
        normalize(utf8, b"", NormalizationForm::Custom).unwrap_err(),
        unicodoid::Status::Unsupported
    );
}

#[test]
fn segmentation_boundaries() {
    let utf8 = utf8_system();

    let graphemes = [0x61, 0xCC, 0x81, 0x62]; // a + acute + b
    let segments = collect_segments(utf8, &graphemes, BoundaryType::Grapheme);
    assert_eq!(segments.len(), 2);
    assert_eq!(segments[0].byte_offset, 0);
    assert_eq!(segments[0].byte_length, 3);
    assert_eq!(segments[0].char_count, 2);
    assert_eq!(segments[1].byte_offset, 3);
    assert_eq!(segments[1].byte_length, 1);

    let words = b"hello world";
    let segments = collect_segments(utf8, words, BoundaryType::Word);
    assert_eq!(segments.len(), 3);
    assert_eq!(segments[0].byte_length, 5);
    assert_eq!(segments[1].byte_length, 1);
    assert_eq!(segments[2].byte_length, 5);

    let sentence = b"Hi. Bye.";
    let segments = collect_segments(utf8, sentence, BoundaryType::Sentence);
    assert_eq!(segments.len(), 2);
    assert_eq!(segments[0].byte_length, 4);
    assert_eq!(segments[1].byte_length, 4);

    let line = b"abc def";
    let segments = collect_segments(utf8, line, BoundaryType::Line);
    // UAX #14 LB18 breaks after spaces, so the space stays with "abc".
    assert_eq!(segments.len(), 2);
    assert_eq!(segments[0].byte_length, 4);
    assert_eq!(segments[1].byte_length, 3);

    let conlang = conlang_system();
    match Segmenter::new(conlang, b"A", BoundaryType::Grapheme) {
        Err(status) => assert_eq!(status, unicodoid::Status::Unsupported),
        Ok(_) => panic!("segmentation must require a Unicode profile"),
    }
}

#[test]
fn collation_levels_and_sort_keys() {
    let utf8 = utf8_system();
    let a = TextString::from_bytes(utf8, b"a").unwrap();
    let b = TextString::from_bytes(utf8, b"b").unwrap();
    let upper_a = TextString::from_bytes(utf8, b"A").unwrap();
    let e = TextString::from_bytes(utf8, b"e").unwrap();
    let e_acute = TextString::from_bytes(utf8, &[0xC3, 0xA9]).unwrap();
    let file2 = TextString::from_bytes(utf8, b"file2").unwrap();
    let file10 = TextString::from_bytes(utf8, b"file10").unwrap();

    let mut options = CollationOptions::default();
    assert!(collate(&a, &b, &options) < 0);
    assert!(collate(&b, &a, &options) > 0);
    assert_eq!(collate(&a, &a, &options), 0);

    options.strength = CollationStrength::Primary;
    assert_eq!(collate(&a, &upper_a, &options), 0);
    assert_eq!(collate(&e, &e_acute, &options), 0);

    options = CollationOptions::default();
    assert!(collate(&a, &upper_a, &options) < 0);
    assert!(collate(&e, &e_acute, &options) < 0);

    options.numeric_ordering = true;
    assert!(collate(&file2, &file10, &options) < 0);
    options.numeric_ordering = false;
    assert!(collate(&file2, &file10, &options) > 0);

    // Sort keys must agree with the comparison.
    options = CollationOptions::default();
    let key_a = sort_key(&a, &options).unwrap();
    let key_b = sort_key(&b, &options).unwrap();
    assert!(!key_a.is_empty() && !key_b.is_empty());
    assert!(key_a < key_b);

    let mut buffer = [0u8; 64];
    let written = sort_key_into(&a, &options, &mut buffer).unwrap();
    assert_eq!(&buffer[..written], &key_a[..]);
    assert_eq!(
        sort_key_into(&a, &options, &mut []).unwrap_err(),
        unicodoid::Status::BufferTooSmall
    );

    let conlang = conlang_system();
    let conlang_str = TextString::from_bytes(conlang, b"A").unwrap();
    assert_eq!(
        sort_key(&conlang_str, &options).unwrap_err(),
        unicodoid::Status::Unsupported
    );
}
