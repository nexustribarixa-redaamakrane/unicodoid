//! UTF-32 in little-endian and big-endian byte order.

use crate::system::EncodingSystem;
use crate::types::{
    CharModel, CharValue, CompatibilityLevel, DecodeError, DecodeOutcome, Endianness, Status,
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

/// Unicode profile shared by both UTF-32 byte orders.
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

fn write_u32(destination: &mut [u8], value: u32, little_endian: bool) {
    if little_endian {
        destination[0] = value as u8;
        destination[1] = (value >> 8) as u8;
        destination[2] = (value >> 16) as u8;
        destination[3] = (value >> 24) as u8;
    } else {
        destination[0] = (value >> 24) as u8;
        destination[1] = (value >> 16) as u8;
        destination[2] = (value >> 8) as u8;
        destination[3] = value as u8;
    }
}

fn read_u32(source: &[u8], little_endian: bool) -> u32 {
    if little_endian {
        u32::from(source[0])
            | (u32::from(source[1]) << 8)
            | (u32::from(source[2]) << 16)
            | (u32::from(source[3]) << 24)
    } else {
        (u32::from(source[0]) << 24)
            | (u32::from(source[1]) << 16)
            | (u32::from(source[2]) << 8)
            | u32::from(source[3])
    }
}

fn encode_utf32(
    ch: CharValue,
    destination: &mut [u8],
    little_endian: bool,
) -> Result<usize, Status> {
    if !is_valid_unicode_char(ch) {
        return Err(Status::UnmappableChar);
    }
    if destination.len() < 4 {
        return Err(Status::BufferTooSmall);
    }
    write_u32(destination, ch as u32, little_endian);
    Ok(4)
}

fn decode_utf32(source: &[u8], little_endian: bool) -> Result<DecodeOutcome, Status> {
    if source.len() < 4 {
        return Err(Status::IncompleteSequence);
    }
    let value = read_u32(source, little_endian);
    if !is_valid_unicode_scalar(value) {
        return Err(Status::InvalidCodeUnit);
    }
    Ok(DecodeOutcome::Char {
        ch: value as CharValue,
        read: 4,
    })
}

fn byte_count(chars: &[CharValue]) -> Result<usize, Status> {
    for &ch in chars {
        if !is_valid_unicode_char(ch) {
            return Err(Status::UnmappableChar);
        }
    }
    chars.len().checked_mul(4).ok_or(Status::Overflow)
}

macro_rules! utf32_system {
    ($type_name:ident, $static_name:ident, $id:literal, $name:literal, $description:literal, $little_endian:expr) => {
        /// One UTF-32 byte order.
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
                32
            }

            fn min_bytes_per_char(&self) -> usize {
                4
            }

            fn max_bytes_per_char(&self) -> usize {
                4
            }

            fn model(&self) -> CharModel {
                CharModel::FixedWidth
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
                encode_utf32(ch, destination, $little_endian)
            }

            fn decode_char(
                &self,
                _state: &mut [u8],
                source: &[u8],
            ) -> Result<DecodeOutcome, Status> {
                decode_utf32(source, $little_endian)
            }

            fn validate(&self, state: &mut [u8], source: &[u8]) -> Result<(), DecodeError> {
                if source.len() % 4 != 0 {
                    return Err(DecodeError::new(
                        Status::IncompleteSequence,
                        source.len() - (source.len() % 4),
                    ));
                }
                let mut offset = 0usize;
                while offset < source.len() {
                    match self.decode_char(state, &source[offset..]) {
                        Ok(DecodeOutcome::Char { read, .. }) => {
                            if read == 0 {
                                return Err(DecodeError::new(Status::Internal, offset));
                            }
                            offset += read;
                        }
                        Ok(DecodeOutcome::NoCharacter { read }) => {
                            if read == 0 {
                                return Err(DecodeError::new(Status::Internal, offset));
                            }
                            offset += read;
                        }
                        Err(status) => return Err(DecodeError::new(status, offset)),
                    }
                }
                Ok(())
            }

            fn char_count(&self, _state: &mut [u8], source: &[u8]) -> Result<usize, DecodeError> {
                if source.len() % 4 != 0 {
                    return Err(DecodeError::new(
                        Status::IncompleteSequence,
                        source.len() - (source.len() % 4),
                    ));
                }
                Ok(source.len() / 4)
            }

            fn byte_count(&self, chars: &[CharValue]) -> Result<usize, Status> {
                byte_count(chars)
            }
        }
    };
}

utf32_system!(
    Utf32Le,
    UTF32LE,
    "org.unicode.utf32le",
    "UTF-32LE",
    "Fixed-width 32-bit Unicode Transformation Format (Little Endian)",
    true
);

utf32_system!(
    Utf32Be,
    UTF32BE,
    "org.unicode.utf32be",
    "UTF-32BE",
    "Fixed-width 32-bit Unicode Transformation Format (Big Endian)",
    false
);

/// Borrow the process-wide UTF-32LE system.
pub fn utf32le() -> &'static dyn EncodingSystem {
    &UTF32LE
}

/// Borrow the process-wide UTF-32BE system.
pub fn utf32be() -> &'static dyn EncodingSystem {
    &UTF32BE
}
