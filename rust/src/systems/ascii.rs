//! ASCII-7, a fixed-width 7-bit system with a partial Unicode relationship.

use crate::system::EncodingSystem;
use crate::types::{CharModel, CharValue, CompatibilityLevel, DecodeOutcome, Endianness, Status};
use crate::unicode::{UaxFlags, UnicodeProfile, UnicodeVersion, UtrFlags, UtsFlags};

fn to_unicode(native: CharValue) -> Option<u32> {
    if native <= 0x7F {
        Some(native as u32)
    } else {
        None
    }
}

fn from_unicode(scalar: u32) -> Option<CharValue> {
    if scalar <= 0x7F {
        Some(scalar as CharValue)
    } else {
        None
    }
}

/// Unicode profile attached to the ASCII system.
pub const PROFILE: UnicodeProfile = UnicodeProfile::new(
    UnicodeVersion::new(18, 0, 0),
    UaxFlags::LineBreak14
        .with(UaxFlags::Segmentation29)
        .with(UaxFlags::Ucd44),
    UtsFlags::Uca10,
    UtrFlags::EncodingModel17,
    to_unicode,
    from_unicode,
);

/// ASCII-7 encoding system.
#[derive(Debug, Default, Clone, Copy)]
pub struct Ascii;

/// Process-wide ASCII instance.
pub static ASCII: Ascii = Ascii;

/// Borrow the process-wide ASCII system.
pub fn system() -> &'static dyn EncodingSystem {
    &ASCII
}

impl EncodingSystem for Ascii {
    fn id(&self) -> &'static str {
        "org.unicodoid.ascii"
    }

    fn name(&self) -> &'static str {
        "ASCII-7"
    }

    fn version(&self) -> &'static str {
        "1.0.0"
    }

    fn author(&self) -> &'static str {
        "ANSI / ASA"
    }

    fn description(&self) -> &'static str {
        "Standard 7-bit American Standard Code for Information Interchange"
    }

    fn code_unit_bits(&self) -> u32 {
        8
    }

    fn min_bytes_per_char(&self) -> usize {
        1
    }

    fn max_bytes_per_char(&self) -> usize {
        1
    }

    fn model(&self) -> CharModel {
        CharModel::FixedWidth
    }

    fn endianness(&self) -> Endianness {
        Endianness::None
    }

    fn compatibility_level(&self) -> CompatibilityLevel {
        CompatibilityLevel::Partial
    }

    fn max_char_value(&self) -> CharValue {
        0x7F
    }

    fn replacement_char(&self) -> CharValue {
        b'?' as CharValue
    }

    fn unicode_profile(&self) -> Option<&'static UnicodeProfile> {
        Some(&PROFILE)
    }

    fn encode_char(
        &self,
        _state: &mut [u8],
        ch: CharValue,
        destination: &mut [u8],
    ) -> Result<usize, Status> {
        if ch > 0x7F {
            return Err(Status::UnmappableChar);
        }
        if destination.is_empty() {
            return Err(Status::BufferTooSmall);
        }
        destination[0] = ch as u8;
        Ok(1)
    }

    fn decode_char(&self, _state: &mut [u8], source: &[u8]) -> Result<DecodeOutcome, Status> {
        let Some(&byte) = source.first() else {
            return Err(Status::IncompleteSequence);
        };
        if byte > 0x7F {
            return Err(Status::InvalidCodeUnit);
        }
        Ok(DecodeOutcome::Char {
            ch: byte as CharValue,
            read: 1,
        })
    }

    fn byte_count(&self, chars: &[CharValue]) -> Result<usize, Status> {
        for &ch in chars {
            if ch > 0x7F {
                return Err(Status::UnmappableChar);
            }
        }
        Ok(chars.len())
    }
}
