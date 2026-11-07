//! KupoConlang-14, an independent 14-bit constructed-script system.
//!
//! This system deliberately has **no** Unicode profile.  Its character space
//! is `0x0000..=0x3FFF` and its representation is one byte for values below
//! `0x80` and two bytes otherwise.

use crate::system::EncodingSystem;
use crate::types::{CharModel, CharValue, CompatibilityLevel, DecodeOutcome, Endianness, Status};

/// KupoConlang-14 encoding system.
#[derive(Debug, Default, Clone, Copy)]
pub struct KupoConlang;

/// Process-wide KupoConlang-14 instance.
pub static KUPOCONLANG: KupoConlang = KupoConlang;

/// Borrow the process-wide KupoConlang-14 system.
pub fn system() -> &'static dyn EncodingSystem {
    &KUPOCONLANG
}

impl EncodingSystem for KupoConlang {
    fn id(&self) -> &'static str {
        "org.unicodoid.custom.kupoconlang"
    }

    fn name(&self) -> &'static str {
        "KupoConlang-14"
    }

    fn version(&self) -> &'static str {
        "1.0.0"
    }

    fn author(&self) -> &'static str {
        "Unicodoid Conlang Working Group"
    }

    fn description(&self) -> &'static str {
        "Independent 14-bit Variable-Width Conlang Script System"
    }

    fn code_unit_bits(&self) -> u32 {
        8
    }

    fn min_bytes_per_char(&self) -> usize {
        1
    }

    fn max_bytes_per_char(&self) -> usize {
        2
    }

    fn model(&self) -> CharModel {
        CharModel::VariableWidth
    }

    fn endianness(&self) -> Endianness {
        Endianness::None
    }

    fn compatibility_level(&self) -> CompatibilityLevel {
        CompatibilityLevel::None
    }

    fn max_char_value(&self) -> CharValue {
        0x3FFF
    }

    fn replacement_char(&self) -> CharValue {
        0x003F
    }

    fn encode_char(
        &self,
        _state: &mut [u8],
        ch: CharValue,
        destination: &mut [u8],
    ) -> Result<usize, Status> {
        if ch > 0x3FFF {
            return Err(Status::UnmappableChar);
        }

        if ch <= 0x7F {
            if destination.is_empty() {
                return Err(Status::BufferTooSmall);
            }
            destination[0] = ch as u8;
            Ok(1)
        } else {
            if destination.len() < 2 {
                return Err(Status::BufferTooSmall);
            }
            destination[0] = 0x80 | ((ch >> 8) & 0x3F) as u8;
            destination[1] = (ch & 0xFF) as u8;
            Ok(2)
        }
    }

    fn decode_char(&self, _state: &mut [u8], source: &[u8]) -> Result<DecodeOutcome, Status> {
        let Some(&first) = source.first() else {
            return Err(Status::IncompleteSequence);
        };

        if first & 0x80 == 0 {
            return Ok(DecodeOutcome::Char {
                ch: first as CharValue,
                read: 1,
            });
        }

        if first & 0xC0 == 0x80 {
            if source.len() < 2 {
                return Err(Status::IncompleteSequence);
            }
            let value = ((first as u32 & 0x3F) << 8) | source[1] as u32;
            if value < 0x80 {
                return Err(Status::InvalidCodeUnit);
            }
            return Ok(DecodeOutcome::Char {
                ch: value as CharValue,
                read: 2,
            });
        }

        // Bytes beginning with 11xxxxxx are not valid in KupoConlang-14.
        Err(Status::InvalidCodeUnit)
    }

    fn byte_count(&self, chars: &[CharValue]) -> Result<usize, Status> {
        let mut total = 0usize;
        for &ch in chars {
            if ch <= 0x7F {
                total += 1;
            } else if ch <= 0x3FFF {
                total += 2;
            } else {
                return Err(Status::UnmappableChar);
            }
        }
        Ok(total)
    }
}
