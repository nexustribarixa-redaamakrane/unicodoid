//! ConvertUTF harness: well-formedness, lenient/strict behavior, and
//! cross-encoding equivalence for the Unicode 18.0.0 ConvertUTF port.
//!
//! Vectors cover empty inputs, BMP and supplementary code points, overlong
//! forms, encoded surrogates, truncated sequences, and values above U+10FFFF.

use unicodoid::convertutf::{
    ConversionError, ConversionFlags, ConversionResult, UNICODE_MAX_LEGAL_UTF32, is_legal_utf8,
    utf16_to_utf32, utf16_to_utf8, utf32_to_utf16, utf32_to_utf8, utf8_to_utf16, utf8_to_utf32,
};
use unicodoid::unicode::UnicodeVersion;

const STRICT: ConversionFlags = ConversionFlags::StrictConversion;
const LENIENT: ConversionFlags = ConversionFlags::LenientConversion;

fn err(result: ConversionResult) -> ConversionError {
    ConversionError::new(result)
}

#[test]
fn utf8_to_utf16_vectors() {
    // Empty input converts to empty output.
    assert_eq!(
        utf8_to_utf16(&[], STRICT),
        Ok(Vec::new()),
        "empty utf8 to utf16"
    );
    // ASCII.
    assert_eq!(
        utf8_to_utf16(&[0x41], STRICT),
        Ok(vec![0x0041]),
        "A to utf16"
    );
    // Two-byte sequence: é (U+00E9).
    assert_eq!(
        utf8_to_utf16(&[0xC3, 0xA9], STRICT),
        Ok(vec![0x00E9]),
        "e-acute to utf16"
    );
    // Three-byte sequence: 漢 (U+6F22).
    assert_eq!(
        utf8_to_utf16(&[0xE6, 0xBC, 0xA2], STRICT),
        Ok(vec![0x6F22]),
        "han to utf16"
    );
    // Four-byte sequence: 😀 (U+1F600) becomes a surrogate pair.
    assert_eq!(
        utf8_to_utf16(&[0xF0, 0x9F, 0x98, 0x80], STRICT),
        Ok(vec![0xD83D, 0xDE00]),
        "emoji to utf16"
    );
    // Well-formedness: overlong NUL is illegal in both modes.
    assert_eq!(
        utf8_to_utf16(&[0xC0, 0x80], STRICT),
        Err(err(ConversionResult::SourceIllegal)),
        "overlong NUL strict"
    );
    assert_eq!(
        utf8_to_utf16(&[0xC0, 0x80], LENIENT),
        Err(err(ConversionResult::SourceIllegal)),
        "overlong NUL lenient"
    );
    // Encoded surrogate U+D800 (ED A0 80) is illegal.
    assert_eq!(
        utf8_to_utf16(&[0xED, 0xA0, 0x80], STRICT),
        Err(err(ConversionResult::SourceIllegal)),
        "encoded surrogate"
    );
    // Truncated sequence is exhausted.
    assert_eq!(
        utf8_to_utf16(&[0xE2, 0x82], STRICT),
        Err(err(ConversionResult::SourceExhausted)),
        "truncated sequence"
    );
    // F4 90 80 80 would be U+110000, above the Unicode maximum.
    assert_eq!(
        utf8_to_utf16(&[0xF4, 0x90, 0x80, 0x80], STRICT),
        Err(err(ConversionResult::SourceIllegal)),
        "above U+10FFFF"
    );
    assert_eq!(
        utf8_to_utf16(&[0xF4, 0x90, 0x80, 0x80], LENIENT),
        Err(err(ConversionResult::SourceIllegal)),
        "above U+10FFFF lenient"
    );
    // Non-zero offset: decode after a leading ASCII character.
    assert_eq!(
        utf8_to_utf16(b"A\xE6\xBC\xA2", STRICT),
        Ok(vec![0x0041, 0x6F22]),
        "offset decode"
    );
}

#[test]
fn utf16_to_utf8_vectors() {
    assert_eq!(utf16_to_utf8(&[], STRICT), Ok(Vec::new()), "empty");
    assert_eq!(utf16_to_utf8(&[0x0041], STRICT), Ok(vec![0x41]), "A");
    // Surrogate pair for U+1F600.
    assert_eq!(
        utf16_to_utf8(&[0xD83D, 0xDE00], STRICT),
        Ok(vec![0xF0, 0x9F, 0x98, 0x80]),
        "pair to utf8"
    );
    // Lone high surrogate at end of buffer is exhausted.
    assert_eq!(
        utf16_to_utf8(&[0xD800], STRICT),
        Err(err(ConversionResult::SourceExhausted)),
        "lone high surrogate"
    );
    // Unpaired high surrogate followed by ASCII is illegal in strict mode.
    assert_eq!(
        utf16_to_utf8(&[0xD800, 0x0041], STRICT),
        Err(err(ConversionResult::SourceIllegal)),
        "unpaired high strict"
    );
    // Lone low surrogate is illegal in strict mode.
    assert_eq!(
        utf16_to_utf8(&[0xDC00], STRICT),
        Err(err(ConversionResult::SourceIllegal)),
        "lone low strict"
    );
    // Lenient mode encodes the surrogate value itself as 3-byte UTF-8.
    assert!(utf16_to_utf8(&[0xD800, 0x0041], LENIENT).is_ok(), "unpaired high lenient");
    assert!(utf16_to_utf8(&[0xDC00], LENIENT).is_ok(), "lone low lenient");
}

#[test]
fn utf32_vectors() {
    assert_eq!(
        utf32_to_utf8(&[0x1F600], STRICT),
        Ok(vec![0xF0, 0x9F, 0x98, 0x80]),
        "emoji utf32 to utf8"
    );
    assert_eq!(
        utf32_to_utf8(&[0x10FFFF], STRICT),
        Ok(vec![0xF4, 0x8F, 0xBF, 0xBF]),
        "U+10FFFF utf32 to utf8"
    );
    // Surrogate values are illegal in UTF-32 strict mode.
    assert_eq!(
        utf32_to_utf8(&[0xD800], STRICT),
        Err(err(ConversionResult::SourceIllegal)),
        "surrogate utf32 strict"
    );
    // Lenient mode encodes the surrogate value as 3-byte UTF-8.
    assert_eq!(
        utf32_to_utf8(&[0xD800], LENIENT),
        Ok(vec![0xED, 0xA0, 0x80]),
        "surrogate utf32 lenient"
    );

    assert_eq!(
        utf8_to_utf32(&[0xF0, 0x9F, 0x98, 0x80], STRICT),
        Ok(vec![0x1F600]),
        "emoji utf8 to utf32"
    );
    assert_eq!(
        utf8_to_utf32(&[0xC0, 0x80], STRICT),
        Err(err(ConversionResult::SourceIllegal)),
        "overlong utf8 to utf32"
    );

    assert_eq!(
        utf16_to_utf32(&[0xD83D, 0xDE00], STRICT),
        Ok(vec![0x1F600]),
        "pair utf16 to utf32"
    );
    assert_eq!(
        utf16_to_utf32(&[0xD800, 0x0041], STRICT),
        Err(err(ConversionResult::SourceIllegal)),
        "unpaired high utf16 to utf32"
    );

    assert_eq!(
        utf32_to_utf16(&[0x1F600], STRICT),
        Ok(vec![0xD83D, 0xDE00]),
        "emoji utf32 to utf16"
    );
    assert_eq!(
        utf32_to_utf16(&[0xFFFD], STRICT),
        Ok(vec![0xFFFD]),
        "replacement utf32 to utf16"
    );
    assert_eq!(
        utf32_to_utf16(&[0xD800], STRICT),
        Err(err(ConversionResult::SourceIllegal)),
        "surrogate utf32 to utf16 strict"
    );
    // Lenient surrogate in UTF-32 becomes the replacement character.
    assert_eq!(
        utf32_to_utf16(&[0xD800], LENIENT),
        Ok(vec![0xFFFD]),
        "surrogate utf32 to utf16 lenient"
    );
    assert_eq!(
        utf32_to_utf16(&[0x110000], LENIENT),
        Ok(vec![0xFFFD]),
        "above max lenient"
    );
    assert_eq!(
        utf32_to_utf16(&[0x110000], STRICT),
        Err(err(ConversionResult::SourceIllegal)),
        "above max strict"
    );
}

#[test]
fn legality_and_version() {
    assert!(is_legal_utf8(&[0x41], 0, 1), "ASCII legal");
    assert!(!is_legal_utf8(&[0xC0, 0x80], 0, 2), "overlong illegal");
    assert!(!is_legal_utf8(&[0xE2, 0x82], 0, 2), "truncated illegal");
    assert!(is_legal_utf8(&[0xC3, 0xA9], 0, 2), "e-acute legal");
    assert!(is_legal_utf8(&[0xF0, 0x9F, 0x98, 0x80], 0, 4), "emoji legal");
    assert!(!is_legal_utf8(&[0xF4, 0x90, 0x80, 0x80], 0, 4), "above U+10FFFF illegal");
    assert!(!is_legal_utf8(&[0xED, 0xA0, 0x80], 0, 3), "encoded surrogate illegal");
    assert!(!is_legal_utf8(&[0x80], 0, 1), "lone continuation illegal");
    // Offset check: legality past a leading byte.
    assert!(is_legal_utf8(b"A\xC3\xA9", 1, 3), "offset e-acute legal");

    assert_eq!(UNICODE_MAX_LEGAL_UTF32, 0x10FFFF, "legal max");
    assert_eq!(
        unicodoid::convertutf::UNICODE_VERSION,
        UnicodeVersion::new(18, 0, 0),
        "unicode version"
    );
}

#[test]
fn round_trips() {
    // UTF-8 -> UTF-16 -> UTF-8 preserves bytes.
    let original: Vec<u8> = "Aé漢😀".as_bytes().to_vec();
    let utf16 = utf8_to_utf16(&original, STRICT).expect("utf8 to utf16");
    let back = utf16_to_utf8(&utf16, STRICT).expect("utf16 to utf8");
    assert_eq!(back, original, "utf8 round trip");

    // UTF-8 -> UTF-32 -> UTF-8 preserves bytes.
    let utf32 = utf8_to_utf32(&original, STRICT).expect("utf8 to utf32");
    let back32 = utf32_to_utf8(&utf32, STRICT).expect("utf32 to utf8");
    assert_eq!(back32, original, "utf32 round trip");

    // UTF-16 -> UTF-32 -> UTF-16 preserves code units.
    let utf16z: Vec<u16> = vec![0x0041, 0x00E9, 0x6F22, 0xD83D, 0xDE00];
    let utf32z = utf16_to_utf32(&utf16z, STRICT).expect("utf16 to utf32");
    let back16 = utf32_to_utf16(&utf32z, STRICT).expect("utf32 to utf16");
    assert_eq!(back16, utf16z, "utf16 round trip");
}
