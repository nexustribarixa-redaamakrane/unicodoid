//! Core contract tests: status codes, registry, codecs, strings, transcoding.

use unicodoid::properties::system_general_category;
use unicodoid::{
    Codec, DecodeOutcome, Status, TextString, TranscodeConfig, TranscodePolicy, ascii_system,
    conlang_system, transcode, transcode_string, utf8_system, utf16le,
};

#[test]
fn status_codes_and_messages() {
    assert_eq!(Status::Ok.code(), 0);
    assert_eq!(Status::InvalidParam.code(), -1);
    assert_eq!(Status::BufferTooSmall.code(), -2);
    assert_eq!(Status::InvalidCodeUnit.code(), -3);
    assert_eq!(Status::UnmappableChar.code(), -4);
    assert_eq!(Status::IncompleteSequence.code(), -5);
    assert_eq!(Status::NotFound.code(), -6);
    assert_eq!(Status::AlreadyExists.code(), -7);
    assert_eq!(Status::Unsupported.code(), -8);
    assert_eq!(Status::CapacityExceeded.code(), -9);
    assert_eq!(Status::AllocationFailed.code(), -10);
    assert_eq!(Status::Internal.code(), -11);
    assert_eq!(Status::State.code(), -12);
    assert_eq!(Status::Overflow.code(), -13);
    assert_eq!(Status::NoCharacter.code(), -14);

    assert_eq!(Status::State.message(), "invalid codec state");
    assert_eq!(Status::Overflow.message(), "numeric overflow");
    assert!(!Status::Ok.is_error());
    assert!(Status::UnmappableChar.is_error());
}

#[test]
fn registry_lifecycle() {
    unicodoid::registry::cleanup();
    unicodoid::registry::init().unwrap();
    unicodoid::registry::init().unwrap(); // idempotent
    assert_eq!(unicodoid::registry::count(), 8);

    let utf8 = unicodoid::registry::find_by_id("org.unicode.utf8").unwrap();
    assert_eq!(utf8.id(), utf8_system().id());
    assert_eq!(
        unicodoid::registry::find_by_name("UTF-8").unwrap().id(),
        utf8.id()
    );
    assert!(unicodoid::registry::find_by_id("org.example.missing").is_none());
    assert!(unicodoid::registry::get_at(0).is_some());
    assert!(unicodoid::registry::get_at(unicodoid::registry::count()).is_none());

    assert_eq!(
        unicodoid::registry::register(utf8),
        Err(Status::AlreadyExists)
    );
    unicodoid::registry::unregister("org.unicode.utf8").unwrap();
    assert!(unicodoid::registry::find_by_id("org.unicode.utf8").is_none());
    assert_eq!(
        unicodoid::registry::unregister("org.unicode.utf8"),
        Err(Status::NotFound)
    );

    unicodoid::registry::cleanup();
}

#[test]
fn utf8_roundtrip_and_validation() {
    let sys = utf8_system();
    let mut codec = Codec::new(sys);
    let mut buf = [0u8; 8];

    let written = codec.encode_char(0x00E9, &mut buf).unwrap();
    assert_eq!(written, 2);
    assert_eq!(&buf[..2], &[0xC3, 0xA9]);

    match codec.decode_char(&buf[..written]).unwrap() {
        DecodeOutcome::Char { ch, read } => {
            assert_eq!(ch, 0x00E9);
            assert_eq!(read, 2);
        }
        other => panic!("unexpected outcome: {other:?}"),
    }

    let written = codec.encode_char(0x1F600, &mut buf).unwrap();
    assert_eq!(written, 4);
    assert_eq!(&buf[..4], &[0xF0, 0x9F, 0x98, 0x80]);
    match codec.decode_char(&buf[..written]).unwrap() {
        DecodeOutcome::Char { ch, read } => {
            assert_eq!(ch, 0x1F600);
            assert_eq!(read, 4);
        }
        other => panic!("unexpected outcome: {other:?}"),
    }

    let overlong = [0xC0, 0x80];
    assert_eq!(
        codec.decode_char(&overlong).unwrap_err(),
        Status::InvalidCodeUnit
    );

    let truncated = [0xE2, 0x82];
    assert_eq!(
        codec.decode_char(&truncated).unwrap_err(),
        Status::IncompleteSequence
    );

    let surrogate = [0xED, 0xA0, 0x80];
    assert_eq!(
        codec.decode_char(&surrogate).unwrap_err(),
        Status::InvalidCodeUnit
    );

    let bad_mix = [0x41, 0xFF, 0x42];
    let error = codec.validate(&bad_mix).unwrap_err();
    assert_eq!(error.status, Status::InvalidCodeUnit);
    assert_eq!(error.offset, 1);

    let chars = [0x41u64, 0x00E9, 0x1F600];
    let written = codec.encode(&chars, &mut buf).unwrap();
    assert_eq!(written, 7);

    let mut decoded = [0u64; 3];
    let count = codec.decode(&buf[..written], &mut decoded).unwrap();
    assert_eq!(count, 3);
    assert_eq!(decoded, [0x41, 0x00E9, 0x1F600]);
}

#[test]
fn ascii_and_conlang() {
    let ascii = ascii_system();
    assert!(ascii.has_unicode_profile());
    assert!(!ascii.is_stateful());

    let mut codec = Codec::new(ascii);
    let mut buf = [0u8; 8];
    let written = codec.encode_char(u64::from(b'A'), &mut buf).unwrap();
    assert_eq!(written, 1);
    assert_eq!(buf[0], b'A');
    assert_eq!(
        codec.encode_char(0x80, &mut buf).unwrap_err(),
        Status::UnmappableChar
    );
    assert_eq!(
        codec.decode_char(&[0x80]).unwrap_err(),
        Status::InvalidCodeUnit
    );

    let conlang = conlang_system();
    assert!(!conlang.has_unicode_profile());

    let mut codec = Codec::new(conlang);
    let written = codec.encode_char(0x3FFF, &mut buf).unwrap();
    assert_eq!(written, 2);
    assert_eq!(&buf[..2], &[0xBF, 0xFF]);
    match codec.decode_char(&buf[..written]).unwrap() {
        DecodeOutcome::Char { ch, read } => {
            assert_eq!(ch, 0x3FFF);
            assert_eq!(read, 2);
        }
        other => panic!("unexpected outcome: {other:?}"),
    }
    assert_eq!(
        codec.encode_char(0x4000, &mut buf).unwrap_err(),
        Status::UnmappableChar
    );

    assert_eq!(
        system_general_category(conlang, u64::from(b'A')).unwrap_err(),
        Status::Unsupported
    );
}

#[test]
fn strings_are_length_bounded_and_stateful() {
    let sys = utf8_system();
    let mut string = TextString::from_chars(sys, &[u64::from(b'H'), u64::from(b'i')]).unwrap();
    assert_eq!(string.bytes(), b"Hi");
    assert_eq!(string.char_len().unwrap(), 2);
    assert_eq!(string.char_at(1).unwrap(), u64::from(b'i'));
    assert_eq!(string.char_at(2).unwrap_err(), Status::NotFound);

    string.append_char(0x1F600).unwrap();
    assert_eq!(string.char_len().unwrap(), 3);

    let clone = string.clone();
    assert_eq!(string, clone);
    assert_eq!(string.bytes(), clone.bytes());

    let with_zero = TextString::from_chars(sys, &[u64::from(b'A'), 0, u64::from(b'B')]).unwrap();
    assert_eq!(with_zero.len(), 3);
    assert_eq!(with_zero.char_len().unwrap(), 3);
    assert_eq!(with_zero.char_at(1).unwrap(), 0);

    let bad = [0x41, 0xC0, 0x80];
    assert_eq!(
        TextString::from_bytes(sys, &bad).unwrap_err(),
        Status::InvalidCodeUnit
    );
}

#[test]
fn transcoding_is_explicit() {
    let utf8 = utf8_system();
    let utf16 = utf16le();
    let ascii = ascii_system();
    let conlang = conlang_system();

    let text = b"h\xC3\xA9llo";
    let src = TextString::from_bytes(utf8, text).unwrap();

    let cfg = TranscodeConfig::default();
    let mid = transcode_string(&src, utf16, &cfg).unwrap();
    let roundtrip = transcode_string(&mid, utf8, &cfg).unwrap();
    assert_eq!(src.bytes(), roundtrip.bytes());

    let same = transcode_string(&src, utf8, &cfg).unwrap();
    assert_eq!(src.bytes(), same.bytes());

    let ascii_str = TextString::from_bytes(ascii, b"Hi").unwrap();
    let wide = transcode_string(&ascii_str, utf8, &cfg).unwrap();
    assert_eq!(wide.bytes(), b"Hi");

    let conlang_str = TextString::from_chars(conlang, &[u64::from(b'A')]).unwrap();
    assert_eq!(
        transcode_string(&conlang_str, utf8, &cfg).unwrap_err(),
        Status::UnmappableChar
    );

    let bridged_cfg = TranscodeConfig {
        bridge: Some(Box::new(|_src, ch, _dst| {
            if ch > 0x10FFFF {
                Err(Status::UnmappableChar)
            } else {
                Ok(ch)
            }
        })),
        ..TranscodeConfig::default()
    };
    let bridged = transcode_string(&conlang_str, utf8, &bridged_cfg).unwrap();
    assert_eq!(bridged.bytes(), b"A");

    let replace_cfg = TranscodeConfig {
        policy: TranscodePolicy::Replace,
        ..TranscodeConfig::default()
    };
    let accented = TextString::from_bytes(utf8, &[0xC3, 0xA9]).unwrap();
    let replaced = transcode_string(&accented, ascii, &replace_cfg).unwrap();
    assert_eq!(replaced.bytes(), b"?");

    let required = transcode(utf8, text, utf16, &cfg).unwrap();
    assert_eq!(required, mid.bytes());

    // Strict mode must preserve the mapping failure, not mask it.
    assert_eq!(
        transcode(conlang, b"A", utf8, &TranscodeConfig::default()).unwrap_err(),
        Status::UnmappableChar
    );

    // Same-system conversion validates before copying.
    assert_eq!(
        transcode(utf8, &[0xC0, 0x80], utf8, &TranscodeConfig::default()).unwrap_err(),
        Status::InvalidCodeUnit
    );
}
