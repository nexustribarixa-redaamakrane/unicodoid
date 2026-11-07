//! Optional Unicode compatibility metadata and the curated specification
//! catalog.

use core::ops::{BitOr, BitOrAssign};

use crate::types::{CharValue, is_valid_unicode_scalar};

/// Official Unicode publication categories.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
#[repr(u8)]
pub enum UnicodeDocCategory {
    Uax = 1,
    Uts = 2,
    Utr = 3,
    Utn = 4,
}

impl UnicodeDocCategory {
    pub const fn label(self) -> &'static str {
        match self {
            UnicodeDocCategory::Uax => "UAX",
            UnicodeDocCategory::Uts => "UTS",
            UnicodeDocCategory::Utr => "UTR",
            UnicodeDocCategory::Utn => "UTN",
        }
    }
}

macro_rules! flags {
    ($name:ident { $($flag:ident = $bit:expr),* $(,)? }) => {
        #[derive(Debug, Clone, Copy, PartialEq, Eq, Default, Hash)]
        pub struct $name(pub u32);

        #[allow(non_upper_case_globals)]
        impl $name {
            pub const NONE: Self = Self(0);
            $(pub const $flag: Self = Self(1 << $bit);)*

            pub const fn contains(self, other: Self) -> bool {
                (self.0 & other.0) == other.0
            }

            /// Combine two flag sets in a `const` context.
            pub const fn with(self, other: Self) -> Self {
                Self(self.0 | other.0)
            }
        }

        impl BitOr for $name {
            type Output = Self;
            fn bitor(self, rhs: Self) -> Self::Output {
                Self(self.0 | rhs.0)
            }
        }

        impl BitOrAssign for $name {
            fn bitor_assign(&mut self, rhs: Self) {
                self.0 |= rhs.0;
            }
        }
    };
}

flags!(UaxFlags {
    Bidi9 = 0,
    EastAsianWidth11 = 1,
    LineBreak14 = 2,
    Normalization15 = 3,
    Script24 = 4,
    Segmentation29 = 5,
    IdSyntax31 = 6,
    Ucd44 = 7,
    NamedSequences34 = 8,
    Unihan38 = 9,
    References41 = 10,
    UcdXml42 = 11,
    USource45 = 12,
    VerticalText50 = 13,
    ArabicMark53 = 14,
    Unikemet57 = 15,
    EastAsianData60 = 16,
});

flags!(UtsFlags {
    Uca10 = 0,
    Regex18 = 1,
    Ldml35 = 2,
    Security39 = 3,
    Idna46 = 4,
    Emoji51 = 5,
    Ivd37 = 6,
    SourceCode55 = 7,
    Links58 = 8,
    SetNotation61 = 9,
});

flags!(UtrFlags {
    EncodingModel17 = 0,
    PropertyModel23 = 1,
    Math25 = 2,
    Conformance33 = 3,
    Mongolian54 = 4,
    Cuneiform56 = 5,
    EastAsianSpacing59 = 6,
});

/// Unicode Standard version declared by a profile.
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct UnicodeVersion {
    pub major: u16,
    pub minor: u16,
    pub patch: u16,
}

impl UnicodeVersion {
    pub const fn new(major: u16, minor: u16, patch: u16) -> Self {
        Self {
            major,
            minor,
            patch,
        }
    }
}

impl core::fmt::Display for UnicodeVersion {
    fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
        write!(f, "{}.{}.{}", self.major, self.minor, self.patch)
    }
}

/// Optional Unicode compatibility profile attached to an encoding system.
///
/// A profile owns the mapping between native character values and Unicode
/// scalar values.  It is never required: systems may declare
/// [`CompatibilityLevel::None`](crate::types::CompatibilityLevel::None) and
/// expose no profile at all.
#[derive(Clone, Copy)]
pub struct UnicodeProfile {
    pub version: UnicodeVersion,
    pub supported_uax: UaxFlags,
    pub supported_uts: UtsFlags,
    pub supported_utr: UtrFlags,
    pub to_unicode: fn(CharValue) -> Option<u32>,
    pub from_unicode: fn(u32) -> Option<CharValue>,
}

impl UnicodeProfile {
    pub const fn new(
        version: UnicodeVersion,
        supported_uax: UaxFlags,
        supported_uts: UtsFlags,
        supported_utr: UtrFlags,
        to_unicode: fn(CharValue) -> Option<u32>,
        from_unicode: fn(u32) -> Option<CharValue>,
    ) -> Self {
        Self {
            version,
            supported_uax,
            supported_uts,
            supported_utr,
            to_unicode,
            from_unicode,
        }
    }

    /// Attempt to map a native character value to a Unicode scalar value.
    pub fn try_to_unicode(&self, native: CharValue) -> Option<u32> {
        (self.to_unicode)(native).filter(|cp| is_valid_unicode_scalar(*cp))
    }

    /// Attempt to map a Unicode scalar value to a native character value.
    pub fn try_from_unicode(&self, scalar: u32) -> Option<CharValue> {
        if !is_valid_unicode_scalar(scalar) {
            return None;
        }
        (self.from_unicode)(scalar).filter(|value| *value != crate::types::INVALID_CHAR)
    }
}

impl core::fmt::Debug for UnicodeProfile {
    fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
        f.debug_struct("UnicodeProfile")
            .field("version", &self.version)
            .field("supported_uax", &self.supported_uax)
            .field("supported_uts", &self.supported_uts)
            .field("supported_utr", &self.supported_utr)
            .finish_non_exhaustive()
    }
}

/// Metadata for one Unicode specification.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct SpecRef {
    pub category: UnicodeDocCategory,
    pub number: u16,
    pub title: &'static str,
    pub url: &'static str,
    pub status: &'static str,
}

impl core::fmt::Display for SpecRef {
    fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
        write!(
            f,
            "{} #{}: {}",
            self.category.label(),
            self.number,
            self.title
        )
    }
}

/// Generated catalog of the current Unicode publications relevant to this SDK,
/// plus the one Unicode Technical Note it references.
///
/// Titles, URLs and statuses are taken verbatim from
/// `docs/unicode18/inventory.json` (generated from the official Unicode
/// reports index and report pages) and the official Unicode Technical Notes
/// index.  Stabilized, superseded, withdrawn, suspended or otherwise
/// non-current reports are intentionally not listed; their dispositions are
/// documented under `docs/unicode18/`.
pub const SPECS: &[SpecRef] = &[
    // UAX: Unicode Standard Annexes (current in Unicode 18.0.0)
    SpecRef { category: UnicodeDocCategory::Uax, number: 9, title: "Unicode Bidirectional Algorithm", url: "https://www.unicode.org/reports/tr9/tr9-52.html", status: "Current" },
    SpecRef { category: UnicodeDocCategory::Uax, number: 11, title: "East Asian Width", url: "https://www.unicode.org/reports/tr11/tr11-46.html", status: "Current" },
    SpecRef { category: UnicodeDocCategory::Uax, number: 14, title: "Unicode Line Breaking Algorithm", url: "https://www.unicode.org/reports/tr14/tr14-57.html", status: "Current" },
    SpecRef { category: UnicodeDocCategory::Uax, number: 15, title: "Unicode Normalization Forms", url: "https://www.unicode.org/reports/tr15/tr15-58.html", status: "Current" },
    SpecRef { category: UnicodeDocCategory::Uax, number: 24, title: "Unicode Script Property", url: "https://www.unicode.org/reports/tr24/tr24-41.html", status: "Current" },
    SpecRef { category: UnicodeDocCategory::Uax, number: 29, title: "Unicode Text Segmentation", url: "https://www.unicode.org/reports/tr29/tr29-49.html", status: "Current" },
    SpecRef { category: UnicodeDocCategory::Uax, number: 31, title: "Unicode Identifiers and Syntax", url: "https://www.unicode.org/reports/tr31/tr31-45.html", status: "Current" },
    SpecRef { category: UnicodeDocCategory::Uax, number: 34, title: "Unicode Named Character Sequences", url: "https://www.unicode.org/reports/tr34/tr34-33.html", status: "Current" },
    SpecRef { category: UnicodeDocCategory::Uax, number: 38, title: "Unicode Han Database (Unihan)", url: "https://www.unicode.org/reports/tr38/tr38-41.html", status: "Current" },
    SpecRef { category: UnicodeDocCategory::Uax, number: 41, title: "Common References for UAXs", url: "https://www.unicode.org/reports/tr41/tr41-38.html", status: "Current" },
    SpecRef { category: UnicodeDocCategory::Uax, number: 42, title: "Unicode Character Database in XML", url: "https://www.unicode.org/reports/tr42/tr42-40.html", status: "Current" },
    SpecRef { category: UnicodeDocCategory::Uax, number: 44, title: "Unicode Character Database", url: "https://www.unicode.org/reports/tr44/tr44-38.html", status: "Current" },
    SpecRef { category: UnicodeDocCategory::Uax, number: 45, title: "U-source Ideographs", url: "https://www.unicode.org/reports/tr45/tr45-34.html", status: "Current" },
    SpecRef { category: UnicodeDocCategory::Uax, number: 50, title: "Unicode Vertical Text Layout", url: "https://www.unicode.org/reports/tr50/tr50-35.html", status: "Current" },
    SpecRef { category: UnicodeDocCategory::Uax, number: 53, title: "Unicode Arabic Mark Rendering", url: "https://www.unicode.org/reports/tr53/tr53-13.html", status: "Current" },
    SpecRef { category: UnicodeDocCategory::Uax, number: 57, title: "Unicode Egyptian Hieroglyph Database", url: "https://www.unicode.org/reports/tr57/tr57-7.html", status: "Current" },
    SpecRef { category: UnicodeDocCategory::Uax, number: 60, title: "Data for East Asian Scripts", url: "https://www.unicode.org/reports/tr60/tr60-3.html", status: "Current" },
    // UTS: Unicode Technical Standards (current in Unicode 18.0.0)
    SpecRef { category: UnicodeDocCategory::Uts, number: 10, title: "Unicode Collation Algorithm", url: "https://www.unicode.org/reports/tr10/tr10-55.html", status: "Current" },
    SpecRef { category: UnicodeDocCategory::Uts, number: 18, title: "Unicode Regular Expressions", url: "https://www.unicode.org/reports/tr18/tr18-25.html", status: "Current" },
    SpecRef { category: UnicodeDocCategory::Uts, number: 35, title: "Unicode Common Locale Data & Rules (CLDR) Part 1: Core", url: "https://www.unicode.org/reports/tr35/", status: "Current" },
    SpecRef { category: UnicodeDocCategory::Uts, number: 37, title: "Unicode Ideographic Variation Database", url: "https://www.unicode.org/reports/tr37/tr37-16.html", status: "Current" },
    SpecRef { category: UnicodeDocCategory::Uts, number: 39, title: "Unicode Security Mechanisms", url: "https://www.unicode.org/reports/tr39/tr39-34.html", status: "Current" },
    SpecRef { category: UnicodeDocCategory::Uts, number: 46, title: "Unicode IDNA Compatibility Processing", url: "https://www.unicode.org/reports/tr46/tr46-36.html", status: "Current" },
    SpecRef { category: UnicodeDocCategory::Uts, number: 51, title: "Unicode Emoji", url: "https://www.unicode.org/reports/tr51/tr51-31.html", status: "Current" },
    SpecRef { category: UnicodeDocCategory::Uts, number: 55, title: "Unicode Source Code Handling", url: "https://www.unicode.org/reports/tr55/tr55-5.html", status: "Current" },
    SpecRef { category: UnicodeDocCategory::Uts, number: 58, title: "Unicode Link Detection and Formatting: URLs and Email Addresses", url: "https://www.unicode.org/reports/tr58/tr58-3.html", status: "Current" },
    SpecRef { category: UnicodeDocCategory::Uts, number: 61, title: "Unicode Set Notation", url: "https://www.unicode.org/reports/tr61/tr61-2.html", status: "Draft" },
    // UTR: Unicode Technical Reports (current in Unicode 18.0.0)
    SpecRef { category: UnicodeDocCategory::Utr, number: 17, title: "Unicode Character Encoding Model", url: "https://www.unicode.org/reports/tr17/tr17-9.html", status: "Current" },
    SpecRef { category: UnicodeDocCategory::Utr, number: 23, title: "The Unicode Character Property Model", url: "https://www.unicode.org/reports/tr23/tr23-15.html", status: "Current" },
    SpecRef { category: UnicodeDocCategory::Utr, number: 25, title: "Unicode Support for Mathematics", url: "https://www.unicode.org/reports/tr25/tr25-16.html", status: "Proposed Update" },
    SpecRef { category: UnicodeDocCategory::Utr, number: 33, title: "Conformance Model", url: "https://www.unicode.org/reports/tr33/tr33-7.html", status: "Current" },
    SpecRef { category: UnicodeDocCategory::Utr, number: 54, title: "Unicode Mongolian 12.1 Baseline", url: "https://www.unicode.org/reports/tr54/tr54-1.html", status: "Current" },
    SpecRef { category: UnicodeDocCategory::Utr, number: 56, title: "Unicode Cuneiform Sign Lists", url: "https://www.unicode.org/reports/tr56/tr56-5.html", status: "Current" },
    SpecRef { category: UnicodeDocCategory::Utr, number: 59, title: "East Asian Spacing", url: "https://www.unicode.org/reports/tr59/tr59-1.html", status: "Current" },
    // UTN: the one Unicode Technical Note referenced by this SDK (docs only).
    // UTNs are independent author works, not specifications; see docs/unicode18/utn/.
    SpecRef { category: UnicodeDocCategory::Utn, number: 1, title: "Issues in Indic Language Collation", url: "https://www.unicode.org/notes/tn1/", status: "Technical Note" },
];

/// Number of entries in the curated catalog.
pub const fn spec_count() -> usize {
    SPECS.len()
}

/// Retrieve a catalog entry by index.
pub fn spec_at(index: usize) -> Option<&'static SpecRef> {
    SPECS.get(index)
}

/// Retrieve a catalog entry by category and number.
pub fn get_spec_ref(category: UnicodeDocCategory, number: u16) -> Option<&'static SpecRef> {
    SPECS
        .iter()
        .find(|spec| spec.category == category && spec.number == number)
}
