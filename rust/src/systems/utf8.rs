//! UTF-8, a variable-width 8-bit Unicode transformation format.

use crate::system::EncodingSystem;
use crate::types::{
    CharModel, CharValue, CompatibilityLevel, DecodeOutcome, Endianness, Status,
    is_valid_unicode_char, is_valid_unicode_scalar,
};
use crate::unicode::{UaxFlags, UnicodeProfile, UnicodeVersion, UtrFlags, UtsFlags};

fn to_unicode(native: CharValue) -> Option<u32> {
    if is_valid_unicode_char(native) {
        Some(native as u32)
    } else {
        None
    }
}

fn from_unicode(scalar: u32) -> Option<CharValue> {
    if is_valid_unicode_scalar(scalar) {
        Some(scalar as CharValue)
    } else {
        None
    }
}

/// Unicode profile attached to the UTF-8 system.
pub const PROFILE: UnicodeProfile = UnicodeProfile::new(
    UnicodeVersion::new(18, 0, 0),
    UaxFlags::Bidi9
        .with(UaxFlags::EastAsianWidth11)
        .with(UaxFlags::LineBreak14)
        .with(UaxFlags::Normalization15)
        .with(UaxFlags::Script24)
        .with(UaxFlags::Segmentation29)
        .with(UaxFlags::IdSyntax31)
        .with(UaxFlags::Ucd44),
    UtsFlags::Uca10
        .with(UtsFlags::Regex18)
        .with(UtsFlags::Ldml35)
        .with(UtsFlags::Security39)
        .with(UtsFlags::Idna46)
        .with(UtsFlags::Emoji51),
    UtrFlags::EncodingModel17
        .with(UtrFlags::PropertyModel23)
        .with(UtrFlags::Math25)
        .with(UtrFlags::Conformance33),
    to_unicode,
    from_unicode,
);

/// UTF-8 encoding system.
#[derive(Debug, Default, Clone, Copy)]
pub struct Utf8;

/// Process-wide UTF-8 instance.
pub static UTF8: Utf8 = Utf8;

/// Borrow the process-wide UTF-8 system.
pub fn system() -> &'static dyn EncodingSystem {
    &UTF8
}

impl EncodingSystem for Utf8 {
    fn id(&self) -> &'static str {
        "org.unicode.utf8"
    }

    fn name(&self) -> &'static str {
        "UTF-8"
    }

    fn version(&self) -> &'static str {
        "1.0.0"
    }

    fn author(&self) -> &'static str {
        "Unicode Consortium / Ken Thompson / Rob Pike"
    }

    fn description(&self) -> &'static str {
        "Variable-width 8-bit Unicode Transformation Format"
    }

    fn code_unit_bits(&self) -> u32 {
        8
    }

    fn min_bytes_per_char(&self) -> usize {
        1
    }

    fn max_bytes_per_char(&self) -> usize {
        4
    }

    fn model(&self) -> CharModel {
        CharModel::VariableWidth
    }

    fn endianness(&self) -> Endianness {
        Endianness::None
    }

    fn compatibility_level(&self) -> CompatibilityLevel {
        CompatibilityLevel::Full
    }

    fn max_char_value(&self) -> CharValue {
        crate::types::UNICODE_MAX_SCALAR as CharValue
    }

    fn replacement_char(&self) -> CharValue {
        crate::types::UNICODE_REPLACEMENT as CharValue
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
        if !is_valid_unicode_char(ch) {
            return Err(Status::UnmappableChar);
        }

        let cp = ch as u32;
        if cp <= 0x7F {
            if destination.is_empty() {
                return Err(Status::BufferTooSmall);
            }
            destination[0] = cp as u8;
            Ok(1)
        } else if cp <= 0x7FF {
            if destination.len() < 2 {
                return Err(Status::BufferTooSmall);
            }
            destination[0] = 0xC0 | (cp >> 6) as u8;
            destination[1] = 0x80 | (cp & 0x3F) as u8;
            Ok(2)
        } else if cp <= 0xFFFF {
            if destination.len() < 3 {
                return Err(Status::BufferTooSmall);
            }
            destination[0] = 0xE0 | (cp >> 12) as u8;
            destination[1] = 0x80 | ((cp >> 6) & 0x3F) as u8;
            destination[2] = 0x80 | (cp & 0x3F) as u8;
            Ok(3)
        } else {
            if destination.len() < 4 {
                return Err(Status::BufferTooSmall);
            }
            destination[0] = 0xF0 | (cp >> 18) as u8;
            destination[1] = 0x80 | ((cp >> 12) & 0x3F) as u8;
            destination[2] = 0x80 | ((cp >> 6) & 0x3F) as u8;
            destination[3] = 0x80 | (cp & 0x3F) as u8;
            Ok(4)
        }
    }

    fn decode_char(&self, _state: &mut [u8], source: &[u8]) -> Result<DecodeOutcome, Status> {
        let Some(&b0) = source.first() else {
            return Err(Status::IncompleteSequence);
        };

        if b0 & 0x80 == 0x00 {
            return Ok(DecodeOutcome::Char {
                ch: b0 as CharValue,
                read: 1,
            });
        }

        if b0 & 0xE0 == 0xC0 {
            if source.len() < 2 {
                return Err(Status::IncompleteSequence);
            }
            let b1 = source[1];
            if b1 & 0xC0 != 0x80 {
                return Err(Status::InvalidCodeUnit);
            }
            let value = ((b0 as u32 & 0x1F) << 6) | (b1 as u32 & 0x3F);
            if value < 0x80 {
                return Err(Status::InvalidCodeUnit);
            }
            return Ok(DecodeOutcome::Char {
                ch: value as CharValue,
                read: 2,
            });
        }

        if b0 & 0xF0 == 0xE0 {
            if source.len() < 3 {
                return Err(Status::IncompleteSequence);
            }
            let b1 = source[1];
            let b2 = source[2];
            if b1 & 0xC0 != 0x80 || b2 & 0xC0 != 0x80 {
                return Err(Status::InvalidCodeUnit);
            }
            let value = ((b0 as u32 & 0x0F) << 12) | ((b1 as u32 & 0x3F) << 6) | (b2 as u32 & 0x3F);
            if value < 0x800 || (0xD800..=0xDFFF).contains(&value) {
                return Err(Status::InvalidCodeUnit);
            }
            return Ok(DecodeOutcome::Char {
                ch: value as CharValue,
                read: 3,
            });
        }

        if b0 & 0xF8 == 0xF0 {
            if source.len() < 4 {
                return Err(Status::IncompleteSequence);
            }
            let b1 = source[1];
            let b2 = source[2];
            let b3 = source[3];
            if b1 & 0xC0 != 0x80 || b2 & 0xC0 != 0x80 || b3 & 0xC0 != 0x80 {
                return Err(Status::InvalidCodeUnit);
            }
            let value = ((b0 as u32 & 0x07) << 18)
                | ((b1 as u32 & 0x3F) << 12)
                | ((b2 as u32 & 0x3F) << 6)
                | (b3 as u32 & 0x3F);
            if !(0x10000..=0x10FFFF).contains(&value) {
                return Err(Status::InvalidCodeUnit);
            }
            return Ok(DecodeOutcome::Char {
                ch: value as CharValue,
                read: 4,
            });
        }

        Err(Status::InvalidCodeUnit)
    }

    fn byte_count(&self, chars: &[CharValue]) -> Result<usize, Status> {
        let mut total = 0usize;
        for &ch in chars {
            if !is_valid_unicode_char(ch) {
                return Err(Status::UnmappableChar);
            }
            total += if ch <= 0x7F {
                1
            } else if ch <= 0x7FF {
                2
            } else if ch <= 0xFFFF {
                3
            } else {
                4
            };
        }
        Ok(total)
    }
}
