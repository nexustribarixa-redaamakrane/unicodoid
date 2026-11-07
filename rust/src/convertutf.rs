//! UTF-8 / UTF-16 / UTF-32 conversion: the ConvertUTF port.
//!
//! Ported from the discontinued W3C/Unicode ConvertUTF library and updated to
//! the Unicode 18.0.0 data set.  The routines convert between UTF-8, UTF-16 and
//! UTF-32 code unit sequences, report the first encountered problem through a
//! [`ConversionError`], and enforce well-formedness independently of the
//! strict/lenient policy.
//!
//! * Strict conversions reject irregular sequences and isolated surrogates.
//! * Lenient conversions convert irregular sequences and isolated surrogates
//!   to the replacement character.
//! * All illegal sequences are errors in both modes: sequences such as
//!   `0xC0 0x80` in UTF-8, or values above U+10FFFF in UTF-32.

use crate::unicode::UnicodeVersion;

/// Official Unicode version targeted by this conversion module.
pub const UNICODE_VERSION: UnicodeVersion = UnicodeVersion::new(18, 0, 0);

/// The Unicode replacement character (U+FFFD).
pub const UNICODE_REPLACEMENT_CHAR: u32 = 0x0000_FFFD;

/// The largest code point of the Basic Multilingual Plane (U+FFFF).
pub const UNICODE_MAX_BMP: u32 = 0x0000_FFFF;

/// The largest encoded code point (U+10FFFF).
pub const UNICODE_MAX_UTF16: u32 = 0x0010_FFFF;

/// The largest 32-bit value addressable by the Unicode value space.
pub const UNICODE_MAX_UTF32: u32 = 0x7FFF_FFFF;

/// The largest legal Unicode code point (U+10FFFF).
pub const UNICODE_MAX_LEGAL_UTF32: u32 = 0x0010_FFFF;

/// Start of the high surrogate range (U+D800).
pub const UNI_SUR_HIGH_START: u32 = 0x0000_D800;

/// End of the high surrogate range (U+DBFF).
pub const UNI_SUR_HIGH_END: u32 = 0x0000_DBFF;

/// Start of the low surrogate range (U+DC00).
pub const UNI_SUR_LOW_START: u32 = 0x0000_DC00;

/// End of the low surrogate range (U+DFFF).
pub const UNI_SUR_LOW_END: u32 = 0x0000_DFFF;

/// Shift used to split a code point into surrogate halves.
pub const HALF_SHIFT: u32 = 10;

/// Half-base offset for supplementary code points (U+10000).
pub const HALF_BASE: u32 = 0x0010_000;

/// Mask holding the low part of a supplementary code point.
pub const HALF_MASK: u32 = 0x003FF;

/// Outcome codes returned by the ConvertUTF conversion routines.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ConversionResult {
    ConversionOk = 0,
    SourceExhausted = 1,
    TargetExhausted = 2,
    SourceIllegal = 3,
}

impl ConversionResult {
    /// Numeric code of the result; part of the cross-language contract.
    pub const fn code(self) -> i32 {
        self as i32
    }

    /// Returns `true` when the conversion completed successfully.
    pub fn is_ok(self) -> bool {
        self == ConversionResult::ConversionOk
    }

    /// Stable human-readable name of the result.
    pub const fn message(self) -> &'static str {
        match self {
            ConversionResult::ConversionOk => "ok",
            ConversionResult::SourceExhausted => "source exhausted",
            ConversionResult::TargetExhausted => "target exhausted",
            ConversionResult::SourceIllegal => "source illegal",
        }
    }
}

impl core::fmt::Display for ConversionResult {
    fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
        f.write_str(self.message())
    }
}

impl std::error::Error for ConversionResult {}

/// Error yielded by a whole-buffer conversion that could not complete.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct ConversionError(pub ConversionResult);

impl ConversionError {
    /// Wraps a conversion result as an error.
    pub const fn new(result: ConversionResult) -> Self {
        Self(result)
    }

    /// Numeric code of the underlying conversion result.
    pub const fn code(self) -> i32 {
        self.0.code()
    }
}

impl From<ConversionResult> for ConversionError {
    fn from(result: ConversionResult) -> Self {
        Self(result)
    }
}

impl core::fmt::Display for ConversionError {
    fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
        f.write_str(self.0.message())
    }
}

impl std::error::Error for ConversionError {}

/// Strictness of a conversion.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ConversionFlags {
    StrictConversion = 0,
    LenientConversion = 1,
}

/// Convert a complete UTF-8 buffer into UTF-16 code units.
///
/// Returns the converted code units, or an error describing why the source
/// could not be converted.
pub fn utf8_to_utf16(source: &[u8], flags: ConversionFlags) -> Result<Vec<u16>, ConversionError> {
    let mut target = Vec::new();
    let source_end = source.len();
    let mut source_idx = 0usize;
    let mut result = ConversionResult::ConversionOk;

    while source_idx < source_end {
        let extra_bytes_to_read = trailing_bytes_for_utf8(source[source_idx]);
        if source_idx + extra_bytes_to_read >= source_end {
            result = ConversionResult::SourceExhausted;
            break;
        }
        if !is_legal_utf8_sequence(source, source_idx, extra_bytes_to_read + 1) {
            result = ConversionResult::SourceIllegal;
            break;
        }

        let mut ch: u32 = 0;
        let mut bytes_to_read = extra_bytes_to_read + 1;
        loop {
            ch += u32::from(source[source_idx]);
            source_idx += 1;
            bytes_to_read -= 1;
            if bytes_to_read > 0 {
                ch <<= 6;
            } else {
                break;
            }
        }
        ch -= offsets_from_utf8(extra_bytes_to_read);

        if ch <= UNICODE_MAX_BMP {
            if ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END {
                if flags == ConversionFlags::StrictConversion {
                    source_idx -= extra_bytes_to_read + 1;
                    result = ConversionResult::SourceIllegal;
                    break;
                }
                target.push(UNICODE_REPLACEMENT_CHAR as u16);
            } else {
                target.push(ch as u16);
            }
        } else if ch > UNICODE_MAX_UTF16 {
            if flags == ConversionFlags::StrictConversion {
                source_idx -= extra_bytes_to_read + 1;
                result = ConversionResult::SourceIllegal;
                break;
            }
            target.push(UNICODE_REPLACEMENT_CHAR as u16);
        } else {
            let ch2 = ch - HALF_BASE;
            let high = ((ch2 >> HALF_SHIFT) + UNI_SUR_HIGH_START) as u16;
            let low = ((ch2 & HALF_MASK) + UNI_SUR_LOW_START) as u16;
            target.push(high);
            target.push(low);
        }
    }

    if result == ConversionResult::ConversionOk {
        Ok(target)
    } else {
        Err(ConversionError::new(result))
    }
}

/// Convert a complete UTF-16 buffer into UTF-8 bytes.
pub fn utf16_to_utf8(source: &[u16], flags: ConversionFlags) -> Result<Vec<u8>, ConversionError> {
    let mut target = Vec::new();
    let source_end = source.len();
    let mut source_idx = 0usize;
    let mut result = ConversionResult::ConversionOk;

    while source_idx < source_end {
        let mut ch = u32::from(source[source_idx]);
        source_idx += 1;

        if ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END {
            if source_idx < source_end {
                let ch2 = u32::from(source[source_idx]);
                if ch2 >= UNI_SUR_LOW_START && ch2 <= UNI_SUR_LOW_END {
                    ch = ((ch - UNI_SUR_HIGH_START) << HALF_SHIFT) + (ch2 - UNI_SUR_LOW_START) + HALF_BASE;
                    source_idx += 1;
                } else if flags == ConversionFlags::StrictConversion {
                    source_idx -= 1;
                    result = ConversionResult::SourceIllegal;
                    break;
                }
            } else {
                source_idx -= 1;
                result = ConversionResult::SourceExhausted;
                break;
            }
        } else if flags == ConversionFlags::StrictConversion {
            if ch >= UNI_SUR_LOW_START && ch <= UNI_SUR_LOW_END {
                source_idx -= 1;
                result = ConversionResult::SourceIllegal;
                break;
            }
        }

        let bytes_to_write: u32 = if ch < 0x80 {
            1
        } else if ch < 0x800 {
            2
        } else if ch < 0x10000 {
            3
        } else if ch < 0x110000 {
            4
        } else {
            ch = UNICODE_REPLACEMENT_CHAR;
            result = ConversionResult::SourceIllegal;
            3
        };

        let n = bytes_to_write as usize;
        let mut buf = [0u8; 4];
        for j in 0..n {
            let shift = 6 * (n as u32 - 1 - j as u32);
            let byte = if j == 0 {
                ((ch >> shift) & 0xFF) as u8 | first_byte_mark(bytes_to_write)
            } else {
                (((ch >> shift) & 0x3F) as u8) | 0x80
            };
            buf[j] = byte;
        }
        target.extend_from_slice(&buf[..n]);
    }

    if result == ConversionResult::ConversionOk {
        Ok(target)
    } else {
        Err(ConversionError::new(result))
    }
}

/// Convert a complete UTF-8 buffer into UTF-32 code points.
pub fn utf8_to_utf32(source: &[u8], flags: ConversionFlags) -> Result<Vec<u32>, ConversionError> {
    let mut target = Vec::new();
    let source_end = source.len();
    let mut source_idx = 0usize;
    let mut result = ConversionResult::ConversionOk;

    while source_idx < source_end {
        let extra_bytes_to_read = trailing_bytes_for_utf8(source[source_idx]);
        if source_idx + extra_bytes_to_read >= source_end {
            result = ConversionResult::SourceExhausted;
            break;
        }
        if !is_legal_utf8_sequence(source, source_idx, extra_bytes_to_read + 1) {
            result = ConversionResult::SourceIllegal;
            break;
        }

        let mut ch: u32 = 0;
        let mut bytes_to_read = extra_bytes_to_read + 1;
        loop {
            ch += u32::from(source[source_idx]);
            source_idx += 1;
            bytes_to_read -= 1;
            if bytes_to_read > 0 {
                ch <<= 6;
            } else {
                break;
            }
        }
        ch -= offsets_from_utf8(extra_bytes_to_read);

        if ch <= UNICODE_MAX_LEGAL_UTF32 {
            if ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END {
                if flags == ConversionFlags::StrictConversion {
                    source_idx -= extra_bytes_to_read + 1;
                    result = ConversionResult::SourceIllegal;
                    break;
                }
                target.push(UNICODE_REPLACEMENT_CHAR);
            } else {
                target.push(ch);
            }
        } else {
            result = ConversionResult::SourceIllegal;
            target.push(UNICODE_REPLACEMENT_CHAR);
        }
    }

    if result == ConversionResult::ConversionOk {
        Ok(target)
    } else {
        Err(ConversionError::new(result))
    }
}

/// Convert a complete UTF-32 buffer into UTF-8 bytes.
pub fn utf32_to_utf8(source: &[u32], flags: ConversionFlags) -> Result<Vec<u8>, ConversionError> {
    let mut target = Vec::new();
    let source_end = source.len();
    let mut source_idx = 0usize;
    let mut result = ConversionResult::ConversionOk;

    while source_idx < source_end {
        let mut ch = source[source_idx];
        source_idx += 1;

        if flags == ConversionFlags::StrictConversion {
            if ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END {
                source_idx -= 1;
                result = ConversionResult::SourceIllegal;
                break;
            }
        }

        let bytes_to_write: u32 = if ch < 0x80 {
            1
        } else if ch < 0x800 {
            2
        } else if ch < 0x10000 {
            3
        } else if ch <= UNICODE_MAX_LEGAL_UTF32 {
            4
        } else {
            ch = UNICODE_REPLACEMENT_CHAR;
            result = ConversionResult::SourceIllegal;
            3
        };

        let n = bytes_to_write as usize;
        let mut buf = [0u8; 4];
        for j in 0..n {
            let shift = 6 * (n as u32 - 1 - j as u32);
            let byte = if j == 0 {
                ((ch >> shift) & 0xFF) as u8 | first_byte_mark(bytes_to_write)
            } else {
                (((ch >> shift) & 0x3F) as u8) | 0x80
            };
            buf[j] = byte;
        }
        target.extend_from_slice(&buf[..n]);
    }

    if result == ConversionResult::ConversionOk {
        Ok(target)
    } else {
        Err(ConversionError::new(result))
    }
}

/// Convert a complete UTF-16 buffer into UTF-32 code points.
pub fn utf16_to_utf32(source: &[u16], flags: ConversionFlags) -> Result<Vec<u32>, ConversionError> {
    let mut target = Vec::new();
    let source_end = source.len();
    let mut source_idx = 0usize;
    let mut result = ConversionResult::ConversionOk;

    while source_idx < source_end {
        let mut ch = u32::from(source[source_idx]);
        source_idx += 1;

        if ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END {
            if source_idx < source_end {
                let ch2 = u32::from(source[source_idx]);
                if ch2 >= UNI_SUR_LOW_START && ch2 <= UNI_SUR_LOW_END {
                    ch = ((ch - UNI_SUR_HIGH_START) << HALF_SHIFT) + (ch2 - UNI_SUR_LOW_START) + HALF_BASE;
                    source_idx += 1;
                } else if flags == ConversionFlags::StrictConversion {
                    source_idx -= 1;
                    result = ConversionResult::SourceIllegal;
                    break;
                }
            } else {
                source_idx -= 1;
                result = ConversionResult::SourceExhausted;
                break;
            }
        } else if flags == ConversionFlags::StrictConversion {
            if ch >= UNI_SUR_LOW_START && ch <= UNI_SUR_LOW_END {
                source_idx -= 1;
                result = ConversionResult::SourceIllegal;
                break;
            }
        }

        target.push(ch);
    }

    if result == ConversionResult::ConversionOk {
        Ok(target)
    } else {
        Err(ConversionError::new(result))
    }
}

/// Convert a complete UTF-32 buffer into UTF-16 code units.
pub fn utf32_to_utf16(source: &[u32], flags: ConversionFlags) -> Result<Vec<u16>, ConversionError> {
    let mut target = Vec::new();
    let source_end = source.len();
    let mut source_idx = 0usize;
    let mut result = ConversionResult::ConversionOk;

    while source_idx < source_end {
        let ch = source[source_idx];
        source_idx += 1;

        if flags == ConversionFlags::StrictConversion {
            if ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END {
                source_idx -= 1;
                result = ConversionResult::SourceIllegal;
                break;
            }
        }

        if ch <= UNICODE_MAX_BMP {
            if ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END {
                if flags == ConversionFlags::StrictConversion {
                    source_idx -= 1;
                    result = ConversionResult::SourceIllegal;
                    break;
                }
                target.push(UNICODE_REPLACEMENT_CHAR as u16);
            } else {
                target.push(ch as u16);
            }
        } else if ch > UNICODE_MAX_UTF16 {
            if flags == ConversionFlags::StrictConversion {
                source_idx -= 1;
                result = ConversionResult::SourceIllegal;
                break;
            }
            target.push(UNICODE_REPLACEMENT_CHAR as u16);
        } else {
            let ch2 = ch - HALF_BASE;
            let high = ((ch2 >> HALF_SHIFT) + UNI_SUR_HIGH_START) as u16;
            let low = ((ch2 & HALF_MASK) + UNI_SUR_LOW_START) as u16;
            target.push(high);
            target.push(low);
        }
    }

    if result == ConversionResult::ConversionOk {
        Ok(target)
    } else {
        Err(ConversionError::new(result))
    }
}

/// Returns whether the UTF-8 sequence starting at `source_start` within
/// `source[0..source_end]` is legal.
pub fn is_legal_utf8(source: &[u8], source_start: usize, source_end: usize) -> bool {
    if source_start >= source_end {
        return false;
    }
    let length = trailing_bytes_for_utf8(source[source_start]) + 1;
    if source_start + length > source_end {
        return false;
    }
    is_legal_utf8_sequence(source, source_start, length)
}

/// Returns whether a UTF-8 sequence of the given length is legal.
fn is_legal_utf8_sequence(source: &[u8], source_start: usize, length: usize) -> bool {
    if length < 1 || length > 4 {
        return false;
    }
    let first = source[source_start];
    if length == 1 {
        if first >= 0x80 && first < 0xC2 {
            return false;
        }
        return first <= 0xF4;
    }

    for i in (source_start + 1)..(source_start + length) {
        if source[i] < 0x80 || source[i] > 0xBF {
            return false;
        }
    }

    let second = source[source_start + 1];
    let legal = match first {
        0xE0 => second >= 0xA0,
        0xED => second <= 0x9F,
        0xF0 => second >= 0x90,
        0xF4 => second <= 0x8F,
        _ => second >= 0x80,
    };
    if !legal {
        return false;
    }
    first <= 0xF4
}

/// Number of trailing bytes expected in a UTF-8 sequence, indexed by the first
/// byte.  A sequence of N bytes has N - 1 trailing bytes.
fn trailing_bytes_for_utf8(first: u8) -> usize {
    if first < 0x80 {
        0
    } else if first < 0xC2 {
        0
    } else if first < 0xE0 {
        1
    } else if first < 0xF0 {
        2
    } else if first <= 0xF4 {
        3
    } else {
        4
    }
}

/// Magic offsets subtracted from accumulated UTF-8 values.
fn offsets_from_utf8(extra: usize) -> u32 {
    match extra {
        0 => 0x0000_0000,
        1 => 0x0003_080,
        2 => 0x000E_2080,
        3 => 0x03C8_2080,
        4 => 0xFA082080,
        _ => 0x82082080,
    }
}

/// First byte mark for a UTF-8 sequence of the given length.
fn first_byte_mark(bytes_to_write: u32) -> u8 {
    match bytes_to_write {
        1 => 0x00,
        2 => 0xC0,
        3 => 0xE0,
        4 => 0xF0,
        5 => 0xF8,
        _ => 0xFC,
    }
}
