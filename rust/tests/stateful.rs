//! Stateful / modal system tests for ShiftModal-16.

use unicodoid::{
    CharModel, Codec, DecodeOutcome, Status, TextString, TranscodeConfig, conlang_system,
    shiftmodal16_system, transcode_string,
};

fn shiftmodal() -> &'static dyn unicodoid::EncodingSystem {
    shiftmodal16_system()
}

#[test]
fn stateful_codec_contract() {
    let sys = shiftmodal();
    assert!(sys.is_stateful());
    assert!(!sys.has_unicode_profile());
    assert!(sys.state_size() > 0);
    assert_eq!(sys.model(), CharModel::ModalStateful);

    let chars = [0x41u64, 0x4000, 0x4001, 0x42, 0x0100];
    let expected = [0x41, 0x0E, 0x00, 0x01, 0x0F, 0x42, 0x1B, 0x00];

    assert_eq!(sys.byte_count(&chars).unwrap(), expected.len());

    let mut codec = Codec::new(sys);
    let mut encoded = [0u8; 16];
    let written = codec.encode(&chars, &mut encoded).unwrap();
    assert_eq!(&encoded[..written], &expected);

    let mut decoded = [0u64; 8];
    let count = codec.decode(&encoded[..written], &mut decoded).unwrap();
    assert_eq!(count, 5);
    assert_eq!(decoded[..5], chars);

    assert_eq!(codec.count_chars(&encoded[..written]).unwrap(), 5);
    codec.validate(&encoded[..written]).unwrap();

    // Reset restores base mode.
    codec.reset();
    let written = codec.encode_char(0x41, &mut encoded).unwrap();
    assert_eq!(written, 1);
    assert_eq!(encoded[0], 0x41);

    // A trailing escape is incomplete.
    let trailing_escape = [0x41, 0x1B];
    let error = codec.validate(&trailing_escape).unwrap_err();
    assert_eq!(error.status, Status::IncompleteSequence);
    assert_eq!(error.offset, 1);

    // A control-only fragment is valid: it changes state but yields no char.
    let lone_shift = [0x0E];
    codec.validate(&lone_shift).unwrap();
    codec.reset();
    assert_eq!(codec.count_chars(&lone_shift).unwrap(), 0);
    codec.reset();

    // Reserved native values cannot be represented.
    for reserved in [0x000Eu64, 0x000F, 0x001B, 0x400F, 0x401B] {
        codec.reset();
        assert_eq!(
            codec.encode_char(reserved, &mut encoded).unwrap_err(),
            Status::UnmappableChar,
            "value {reserved:#06X} should be unrepresentable"
        );
    }

    // A lone shift control decoded directly reports NoCharacter.
    codec.reset();
    match codec.decode_char(&lone_shift).unwrap() {
        DecodeOutcome::NoCharacter { read } => assert_eq!(read, 1),
        other => panic!("unexpected outcome: {other:?}"),
    }
}

#[test]
fn stateful_string_persists_mode() {
    let sys = shiftmodal();
    let chars = [0x41u64, 0x4000, 0x4001, 0x42, 0x0100];

    let mut string = TextString::from_chars(sys, &chars).unwrap();
    assert_eq!(string.len(), 8);
    assert_eq!(string.char_len().unwrap(), 5);
    assert_eq!(string.char_at(1).unwrap(), 0x4000);
    assert_eq!(string.char_at(4).unwrap(), 0x0100);

    // Clone copies both bytes and codec state.
    let mut clone = string.clone();
    assert_eq!(string, clone);

    // Both strings are in base mode after the sequence; append shifted data.
    clone.append_char(0x4002).unwrap();
    assert_eq!(clone.len(), 10);
    assert_eq!(&clone.bytes()[8..], &[0x0E, 0x02]);

    // The original is unaffected.
    assert_eq!(string.len(), 8);

    // State can be reset and the string extended from base mode.
    string.reset_state();
    string.append_char(0x41).unwrap();
    assert_eq!(string.len(), 9);
    assert_eq!(string.bytes()[8], 0x41);
}

#[test]
fn append_bytes_advances_state() {
    let sys = shiftmodal();
    let mut string = TextString::new(sys, 8);

    // Enter shifted mode, then append a raw continuation byte.
    string.append_char(0x4000).unwrap();
    assert_eq!(string.bytes(), &[0x0E, 0x00]);

    string.append_bytes(&[0x01]).unwrap();
    assert_eq!(string.bytes(), &[0x0E, 0x00, 0x01]);

    // SI returns to base mode; a following literal is one byte.
    string.append_bytes(&[0x0F]).unwrap();
    string.append_char(0x41).unwrap();
    assert_eq!(string.len(), 5);
    assert_eq!(string.bytes()[4], 0x41);
    assert_eq!(string.char_len().unwrap(), 3);

    // Invalid continuation is rejected without committing the state.
    let mut trial = string.clone();
    assert_eq!(
        trial.append_bytes(&[0x1B]).unwrap_err(),
        Status::IncompleteSequence
    );
    assert_eq!(trial.bytes(), string.bytes());
}

#[test]
fn stateful_transcoding_uses_explicit_bridge() {
    let shift = shiftmodal();
    let conlang = conlang_system();

    let chars = [0x4001u64, 0x41];
    let src = TextString::from_chars(shift, &chars).unwrap();
    assert_eq!(src.bytes(), &[0x0E, 0x01, 0x0F, 0x41]);

    // Unrelated systems need an explicit bridge.
    assert_eq!(
        transcode_string(&src, conlang, &TranscodeConfig::default()).unwrap_err(),
        Status::UnmappableChar
    );

    let cfg = TranscodeConfig {
        bridge: Some(Box::new(|_src, ch, _dst| {
            if (0x4000..=0x40FF).contains(&ch) {
                Ok(ch & 0xFF)
            } else if ch <= 0x3FFF {
                Ok(ch)
            } else {
                Err(Status::UnmappableChar)
            }
        })),
        ..TranscodeConfig::default()
    };

    let out = transcode_string(&src, conlang, &cfg).unwrap();
    assert_eq!(out.bytes(), &[0x01, 0x41]);

    // Same-system round trip preserves the canonical byte stream.
    let same = transcode_string(&src, shift, &TranscodeConfig::default()).unwrap();
    assert_eq!(same.bytes(), src.bytes());
}
