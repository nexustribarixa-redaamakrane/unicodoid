//! # Unicodoid
//!
//! Unicodoid is a general-purpose SDK for designing, implementing, and managing
//! independent Unicode-like encoding systems.
//!
//! A character is a 64-bit integer in the value space declared by its encoding
//! system.  Unicode compatibility is optional: a system may expose a Unicode
//! profile, or it may have no relationship with Unicode at all.
//!
//! ## Layers
//!
//! * [`system`] defines the [`EncodingSystem`] contract.
//! * [`codec`] owns per-stream state and turns character values into bytes.
//! * [`text_string`] is a length-bounded byte string with persistent state.
//! * [`transcode`] maps between systems through Unicode profiles or an explicit
//!   caller-supplied bridge.
//! * [`properties`], [`normalization`], [`segmentation`], and [`collation`] are
//!   opt-in Unicode algorithms that require a Unicode profile.
//! * [`registry`] tracks the built-in and user-registered systems.
//!
//! The Unicode algorithms are intentionally partial and documented as such.
//! They are suitable for demonstrations and tests, not as conformant
//! replacements for a complete UCD/UAX/UTS implementation.

pub mod codec;
pub mod convertutf;
pub mod generated;
pub mod normalization;
pub mod properties;
pub mod registry;
pub mod segmentation;
pub mod system;
pub mod systems;
pub mod text_string;
pub mod transcode;
pub mod types;
pub mod uax;
pub mod ucd;
pub mod unicode;
pub mod utr;
pub mod uts;

pub use codec::Codec;
pub use convertutf::{
    ConversionError, ConversionFlags, ConversionResult, is_legal_utf8, utf16_to_utf32,
    utf16_to_utf8, utf32_to_utf16, utf32_to_utf8, utf8_to_utf16, utf8_to_utf32,
};
pub use uax::{uax29, uax34};
pub use utr::utr59;
pub use uts::uts10::{
    CollationOptions, CollationStrength, collate, compare_scalars, sort_key, sort_key_into,
    sort_key_units, try_collate,
};
pub use uts::{uts51, uts58};
pub use normalization::{
    NormalizationForm, is_normalized, normalize, normalize_into, normalize_string,
};
pub use segmentation::{BoundaryType, Segment, Segmenter};
pub use system::{EncodingSystem, native_to_scalar, scalar_to_native};
pub use text_string::TextString;
pub use transcode::{
    BridgeFn, SubstitutionFn, TranscodeConfig, transcode, transcode_char, transcode_string,
};
pub use types::{
    CharModel, CharValue, CompatibilityLevel, DecodeError, DecodeOutcome, Endianness, INVALID_CHAR,
    Status, TranscodePolicy, UNICODE_MAX_SCALAR, UNICODE_REPLACEMENT, UNICODE_SURROGATE_MAX,
    UNICODE_SURROGATE_MIN, is_valid_unicode_char, is_valid_unicode_scalar,
};
pub use unicode::{
    SPECS, SpecRef, UaxFlags, UnicodeDocCategory, UnicodeProfile, UnicodeVersion, UtrFlags,
    UtsFlags, get_spec_ref, spec_at, spec_count,
};

// Convenient access to the built-in systems.
pub use systems::ascii::system as ascii_system;
pub use systems::custom_conlang::system as conlang_system;
pub use systems::shiftmodal16::system as shiftmodal16_system;
pub use systems::utf8::system as utf8_system;
pub use systems::utf16::{utf16be, utf16le};
pub use systems::utf32::{utf32be, utf32le};
