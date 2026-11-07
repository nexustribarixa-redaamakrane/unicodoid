//! UTF-16 in little-endian and big-endian byte order.

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

/// Unicode profile shared by both UTF-16 byte orders.
pub const PROFILE: UnicodeProfile = UnicodeProfile::new(
    UnicodeVersion::new(18, 0, 0),
    UaxFlags::Bidi9
        .with(UaxFlags::EastAsianWidth11)
        .with(UaxFlags::LineBreak14)
        .with(UaxFlags::Normalization15)
        .with(UaxFlags::Segmentation29)
        .with(UaxFlags::Ucd44),
    UtsFlags::Uca10
        .with(UtsFlags::Regex18)
        .with(UtsFlags::Ldml35)
        .with(UtsFlags::Security39),
    UtrFlags::EncodingModel17,
    to_unicode,
    from_unicode,
);

fn write_u16(destination: &mut [u8], value: u16, little_endian: bool) {
    if little_endian {
        destination[0] = value as u8;
        destination[1] = (value >> 8) as u8;
    } else {
        destination[0] = (value >> 8) as u8;
        destination[1] = value as u8;
    }
}

fn read_u16(source: &[u8], little_endian: bool) -> u16 {
    if little_endian {
        u16::from(source[0]) | (u16::from(source[1]) << 8)
    } else {
        (u16::from(source[0]) << 8) | u16::from(source[1])
    }
}

fn encode_utf16(
    ch: CharValue,
    destination: &mut [u8],
    little_endian: bool,
) -> Result<usize, Status> {
    if !is_valid_unicode_char(ch) {
        return Err(Status::UnmappableChar);
    }

    if ch <= 0xFFFF {
        if destination.len() < 2 {
            return Err(Status::BufferTooSmall);
        }
        write_u16(destination, ch as u16, little_endian);
        Ok(2)
    } else {
        if destination.len() < 4 {
            return Err(Status::BufferTooSmall);
        }
        let value = (ch - 0x10000) as u32;
        let high = 0xD800 + (value >> 10) as u16;
        let low = 0xDC00 + (value & 0x3FF) as u16;
        write_u16(destination, high, little_endian);
        write_u16(&mut destination[2..], low, little_endian);
        Ok(4)
    }
}

fn decode_utf16(source: &[u8], little_endian: bool) -> Result<DecodeOutcome, Status> {
    if source.len() < 2 {
        return Err(Status::IncompleteSequence);
    }

    let first = read_u16(source, little_endian);

    if (0xD800..=0xDBFF).contains(&first) {
        if source.len() < 4 {
            return Err(Status::IncompleteSequence);
        }
        let second = read_u16(&source[2..], little_endian);
        if !(0xDC00..=0xDFFF).contains(&second) {
            return Err(Status::InvalidCodeUnit);
        }
        let value = (((first - 0xD800) as u32) << 10 | (second - 0xDC00) as u32) + 0x10000;
        return Ok(DecodeOutcome::Char {
            ch: value as CharValue,
            read: 4,
        });
    }

    if (0xDC00..=0xDFFF).contains(&first) {
        return Err(Status::InvalidCodeUnit);
    }

    Ok(DecodeOutcome::Char {
        ch: first as CharValue,
        read: 2,
    })
}

fn byte_count(chars: &[CharValue]) -> Result<usize, Status> {
    let mut total = 0usize;
    for &ch in chars {
        if !is_valid_unicode_char(ch) {
            return Err(Status::UnmappableChar);
        }
        total += if ch <= 0xFFFF { 2 } else { 4 };
    }
    Ok(total)
}

macro_rules! utf16_system {
    ($type_name:ident, $static_name:ident, $id:literal, $name:literal, $description:literal, $little_endian:expr) => {
        /// One UTF-16 byte order.
        #[derive(Debug, Default, Clone, Copy)]
        pub struct $type_name;

        /// Process-wide instance.
        pub static $static_name: $type_name = $type_name;

        impl EncodingSystem for $type_name {
            fn id(&self) -> &'static str {
                $id
            }

            fn name(&self) -> &'static str {
                $name
            }

            fn version(&self) -> &'static str {
                "1.0.0"
            }

            fn author(&self) -> &'static str {
                "Unicode Consortium"
            }

            fn description(&self) -> &'static str {
                $description
            }

            fn code_unit_bits(&self) -> u32 {
                16
            }

            fn min_bytes_per_char(&self) -> usize {
                2
            }

            fn max_bytes_per_char(&self) -> usize {
                4
            }

            fn model(&self) -> CharModel {
                CharModel::VariableWidth
            }

            fn endianness(&self) -> Endianness {
                if $little_endian {
                    Endianness::LittleEndian
                } else {
                    Endianness::BigEndian
                }
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
                encode_utf16(ch, destination, $little_endian)
            }

            fn decode_char(
                &self,
                _state: &mut [u8],
                source: &[u8],
            ) -> Result<DecodeOutcome, Status> {
                decode_utf16(source, $little_endian)
            }

            fn byte_count(&self, chars: &[CharValue]) -> Result<usize, Status> {
                byte_count(chars)
            }
        }
    };
}

utf16_system!(
    Utf16Le,
    UTF16LE,
    "org.unicode.utf16le",
    "UTF-16LE",
    "UTF-16 Little Endian",
    true
);

utf16_system!(
    Utf16Be,
    UTF16BE,
    "org.unicode.utf16be",
    "UTF-16BE",
    "UTF-16 Big Endian",
    false
);

/// Borrow the process-wide UTF-16LE system.
pub fn utf16le() -> &'static dyn EncodingSystem {
    &UTF16LE
}

/// Borrow the process-wide UTF-16BE system.
pub fn utf16be() -> &'static dyn EncodingSystem {
    &UTF16BE
}
