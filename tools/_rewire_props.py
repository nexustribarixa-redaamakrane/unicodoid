path = r"C:\Users\KARIMABENDA\Documents\unicodoid\rust\src\ucd.rs"
src = open(path, encoding="utf-8").read()
src = src.replace("""#[inline]
pub fn combining_class(cp: u32) -> u8 {
    lookup(data::CCC, cp) as u8
}""",
"""#[inline]
pub fn combining_class(cp: u32) -> u8 {
    lookup(data::CCC, cp) as u8
}

#[inline]
fn pair_lookup(table: &[data::Pair], cp: u32, fallback: u32) -> u32 {
    let mut lo = 0usize;
    let mut hi = table.len();
    while lo < hi {
        let mid = lo + (hi - lo) / 2;
        let (start, mapped) = table[mid];
        if cp < start {
            hi = mid;
        } else if cp > start {
            lo = mid + 1;
        } else {
            return mapped;
        }
    }
    fallback
}

#[inline]
pub fn simple_uppercase(cp: u32) -> u32 {
    pair_lookup(data::SIMPLE_UPPER, cp, cp)
}

#[inline]
pub fn simple_lowercase(cp: u32) -> u32 {
    pair_lookup(data::SIMPLE_LOWER, cp, cp)
}

#[inline]
pub fn simple_titlecase(cp: u32) -> u32 {
    pair_lookup(data::SIMPLE_TITLE, cp, cp)
}

#[inline]
pub fn simple_casefold(cp: u32) -> u32 {
    pair_lookup(data::SIMPLE_FOLD, cp, cp)
}

#[inline]
pub fn has_property(cp: u32, table: &[data::Range]) -> bool {
    lookup(table, cp) != 0
}""")
open(path, "w", encoding="utf-8").write(src)

path = r"C:\Users\KARIMABENDA\Documents\unicodoid\rust\src\properties.rs"
src = open(path, encoding="utf-8").read()

old_doc = """//! The property tables are a compact, hand-maintained subset of the UCD.  They
//! are sufficient for the SDK's opt-in algorithms and examples, but they are
//! not a generated property database and must not be presented as complete."""
new_doc = """//! The property tables are the generated Unicode 18.0.0 tables from
//! `ucd_data`; every query is exact for the declared Unicode version."""
assert old_doc in src
src = src.replace(old_doc, new_doc)

src = src.replace("use crate::system::{EncodingSystem, native_to_scalar, scalar_to_native};",
"""use crate::system::{EncodingSystem, native_to_scalar, scalar_to_native};
use crate::ucd;""")

start = src.index("/// General category of a Unicode scalar value.")
end = src.index("// ---- System-aware queries")
new_mid = """/// General category of a Unicode scalar value (UAX #44).
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

"""
src = src[:start] + new_mid + src[end:]
open(path, "w", encoding="utf-8").write(src)
print("done")
