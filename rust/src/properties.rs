//! Unicode character properties.
//!
//! The scalar-level queries operate directly on Unicode scalar values and are
//! used by the Unicode algorithms.  The system-level queries require a Unicode
//! profile and translate native values through the profile.
//!
//! The property tables are the generated Unicode 18.0.0 tables from
//! `ucd_data`; every query is exact for the declared Unicode version.

use crate::system::{EncodingSystem, native_to_scalar, scalar_to_native};
use crate::ucd;
use crate::types::CharValue;

/// General category groups from UAX #44.
#[repr(u8)]
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub enum Category {
    Other = 0,
    UppercaseLetter = 1,
    LowercaseLetter = 2,
    TitlecaseLetter = 3,
    ModifierLetter = 4,
    OtherLetter = 5,
    DecimalNumber = 6,
    LetterNumber = 7,
    OtherNumber = 8,
    SpaceSeparator = 9,
    LineSeparator = 10,
    ParagraphSeparator = 11,
    Control = 12,
    Format = 13,
    Surrogate = 14,
    PrivateUse = 15,
    Unassigned = 16,
    Punctuation = 17,
    Symbol = 18,
    Mark = 19,
}

/// East Asian Width classes from UAX #11.
#[repr(u8)]
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub enum EastAsianWidth {
    Neutral = 0,
    Ambiguous = 1,
    Halfwidth = 2,
    Fullwidth = 3,
    Narrow = 4,
    Wide = 5,
}

/// General category of a Unicode scalar value (UAX #44).
pub fn general_category(cp: u32) -> Category {
    use crate::ucd::gc;
    match ucd::general_category(cp) {
        gc::LU => Category::UppercaseLetter,
        gc::LL => Category::LowercaseLetter,
        gc::LT => Category::TitlecaseLetter,
        gc::LM => Category::ModifierLetter,
        gc::LO => Category::OtherLetter,
        gc::ND => Category::DecimalNumber,
        gc::NL => Category::LetterNumber,
        gc::NO => Category::OtherNumber,
        gc::ZS => Category::SpaceSeparator,
        gc::ZL => Category::LineSeparator,
        gc::ZP => Category::ParagraphSeparator,
        gc::CC => Category::Control,
        gc::CF => Category::Format,
        gc::CS => Category::Surrogate,
        gc::CO => Category::PrivateUse,
        gc::CN => Category::Unassigned,
        gc::PC | gc::PD | gc::PE | gc::PF | gc::PI | gc::PO | gc::PS => Category::Punctuation,
        gc::SC | gc::SK | gc::SM | gc::SO => Category::Symbol,
        gc::MC | gc::ME | gc::MN => Category::Mark,
        _ => Category::Other,
    }
}

/// East Asian Width class of a Unicode scalar value (UAX #11).
pub fn east_asian_width(cp: u32) -> EastAsianWidth {
    use crate::ucd::eaw;
    match ucd::east_asian_width(cp) {
        eaw::N => EastAsianWidth::Neutral,
        eaw::A => EastAsianWidth::Ambiguous,
        eaw::H => EastAsianWidth::Halfwidth,
        eaw::F => EastAsianWidth::Fullwidth,
        eaw::NA => EastAsianWidth::Narrow,
        eaw::W => EastAsianWidth::Wide,
        _ => EastAsianWidth::Neutral,
    }
}

pub fn is_alphabetic(cp: u32) -> bool {
    ucd::has_property(cp, crate::generated::ucd_data::PROP_ALPHABETIC)
}

pub fn is_numeric(cp: u32) -> bool {
    matches!(
        general_category(cp),
        Category::DecimalNumber | Category::LetterNumber | Category::OtherNumber
    )
}

pub fn is_whitespace(cp: u32) -> bool {
    ucd::has_property(cp, crate::generated::ucd_data::PROP_WHITE_SPACE)
}

pub fn is_control(cp: u32) -> bool {
    general_category(cp) == Category::Control
}

pub fn is_punctuation(cp: u32) -> bool {
    general_category(cp) == Category::Punctuation
}

pub fn is_mark(cp: u32) -> bool {
    general_category(cp) == Category::Mark
}

/// Canonical combining class (UAX #44); zero means a starter.
pub fn combining_class(cp: u32) -> u8 {
    ucd::combining_class(cp)
}

pub fn to_uppercase(cp: u32) -> u32 {
    ucd::simple_uppercase(cp)
}

pub fn to_lowercase(cp: u32) -> u32 {
    ucd::simple_lowercase(cp)
}

// ---- System-aware queries (require a Unicode profile) --------------------

pub fn system_general_category(
    system: &dyn EncodingSystem,
    ch: CharValue,
) -> Result<Category, crate::types::Status> {
    Ok(general_category(native_to_scalar(system, ch)?))
}

pub fn system_east_asian_width(
    system: &dyn EncodingSystem,
    ch: CharValue,
) -> Result<EastAsianWidth, crate::types::Status> {
    Ok(east_asian_width(native_to_scalar(system, ch)?))
}

pub fn system_is_alphabetic(
    system: &dyn EncodingSystem,
    ch: CharValue,
) -> Result<bool, crate::types::Status> {
    Ok(is_alphabetic(native_to_scalar(system, ch)?))
}

pub fn system_is_numeric(
    system: &dyn EncodingSystem,
    ch: CharValue,
) -> Result<bool, crate::types::Status> {
    Ok(is_numeric(native_to_scalar(system, ch)?))
}

pub fn system_is_whitespace(
    system: &dyn EncodingSystem,
    ch: CharValue,
) -> Result<bool, crate::types::Status> {
    Ok(is_whitespace(native_to_scalar(system, ch)?))
}

pub fn system_to_uppercase(
    system: &dyn EncodingSystem,
    ch: CharValue,
) -> Result<CharValue, crate::types::Status> {
    scalar_to_native(system, to_uppercase(native_to_scalar(system, ch)?))
}

pub fn system_to_lowercase(
    system: &dyn EncodingSystem,
    ch: CharValue,
) -> Result<CharValue, crate::types::Status> {
    scalar_to_native(system, to_lowercase(native_to_scalar(system, ch)?))
}
