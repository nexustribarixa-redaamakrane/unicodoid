//! Fundamental value types shared by every Unicodoid encoding system.
//!
//! Unicodoid does not assume that a character is a Unicode scalar value.  A
//! character is an integer in the value space declared by its encoding system.
//! The value may be a scalar, a token identifier, or a composite reference.

use core::fmt;

/// The general character value type used at every SDK boundary.
pub type CharValue = u64;

/// Sentinel used where no character value exists.
pub const INVALID_CHAR: CharValue = u64::MAX;

/// Status and error codes returned across the SDK.
///
/// The numeric values are part of the cross-language contract and match the C,
/// C++, and C# SDKs.
#[repr(i32)]
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub enum Status {
    Ok = 0,
    InvalidParam = -1,
    BufferTooSmall = -2,
    InvalidCodeUnit = -3,
    UnmappableChar = -4,
    IncompleteSequence = -5,
    NotFound = -6,
    AlreadyExists = -7,
    Unsupported = -8,
    CapacityExceeded = -9,
    AllocationFailed = -10,
    Internal = -11,
    State = -12,
    Overflow = -13,
    /// Input was consumed but produced no character value.  Modal systems use
    /// this for shift controls.  Streaming loops must advance by the reported
    /// byte count and continue.
    NoCharacter = -14,
}

impl Status {
    pub const fn code(self) -> i32 {
        self as i32
    }

    pub const fn is_error(self) -> bool {
        self.code() < 0
    }

    pub const fn message(self) -> &'static str {
        match self {
            Status::Ok => "ok",
            Status::InvalidParam => "invalid parameter",
            Status::BufferTooSmall => "buffer too small",
            Status::InvalidCodeUnit => "invalid code unit",
            Status::UnmappableChar => "unmappable character",
            Status::IncompleteSequence => "incomplete sequence",
            Status::NotFound => "not found",
            Status::AlreadyExists => "already exists",
            Status::Unsupported => "unsupported",
            Status::CapacityExceeded => "capacity exceeded",
            Status::AllocationFailed => "allocation failed",
            Status::Internal => "internal error",
            Status::State => "invalid codec state",
            Status::Overflow => "numeric overflow",
            Status::NoCharacter => "no character produced",
        }
    }
}

impl fmt::Display for Status {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_str(self.message())
    }
}

impl std::error::Error for Status {}

/// Character modeling architecture declared by an encoding system.
///
/// This is descriptive metadata only; behaviour always comes from the system's
/// operations.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
#[repr(u8)]
pub enum CharModel {
    FixedWidth = 0,
    VariableWidth = 1,
    ModalStateful = 2,
    TokenBased = 3,
    Custom = 4,
}

/// Byte order expectation for multi-byte code units.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
#[repr(u8)]
pub enum Endianness {
    None = 0,
    LittleEndian = 1,
    BigEndian = 2,
}

/// Level of optional Unicode compatibility declared by an encoding system.
///
/// `None` is a first-class state: a system need not have any relationship with
/// Unicode.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
#[repr(u8)]
pub enum CompatibilityLevel {
    None = 0,
    Partial = 1,
    Full = 2,
    Profile = 3,
}

/// Strategy applied during transcoding when a character cannot be mapped.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
#[repr(u8)]
pub enum TranscodePolicy {
    Strict = 0,
    Replace = 1,
    Skip = 2,
    Callback = 3,
}

/// Unicode scalar value constants.  These are only meaningful to systems that
/// declare a Unicode profile; they do not constrain independent systems.
pub const UNICODE_MAX_SCALAR: u32 = 0x0010_FFFF;
pub const UNICODE_SURROGATE_MIN: u32 = 0x0000_D800;
pub const UNICODE_SURROGATE_MAX: u32 = 0x0000_DFFF;
pub const UNICODE_REPLACEMENT: u32 = 0x0000_FFFD;

/// Check whether a 32-bit value is a valid Unicode scalar value.
pub const fn is_valid_unicode_scalar(cp: u32) -> bool {
    cp <= UNICODE_MAX_SCALAR && (cp < UNICODE_SURROGATE_MIN || cp > UNICODE_SURROGATE_MAX)
}

/// Check whether a 64-bit character value is a valid Unicode scalar value.
pub const fn is_valid_unicode_char(value: CharValue) -> bool {
    value <= UNICODE_MAX_SCALAR as u64 && is_valid_unicode_scalar(value as u32)
}

/// Error returned by validating or counting a complete byte stream.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct DecodeError {
    pub status: Status,
    pub offset: usize,
}

impl DecodeError {
    pub const fn new(status: Status, offset: usize) -> Self {
        Self { status, offset }
    }
}

impl fmt::Display for DecodeError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{} at byte {}", self.status, self.offset)
    }
}

impl std::error::Error for DecodeError {}

/// Result of decoding one character from a byte stream.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum DecodeOutcome {
    /// A character value and the number of bytes consumed.
    Char { ch: CharValue, read: usize },
    /// Input was consumed but yielded no character (modal shift controls).
    NoCharacter { read: usize },
}
