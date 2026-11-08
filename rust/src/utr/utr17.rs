//! UTR #17 Unicode Character Encoding Model (Revision 9).

use crate::ucd;

/// Number of UTF-8 code units for a valid scalar, 0 for invalid.
pub fn utf8_length(cp: u32) -> usize {
    if !(cp <= 0x10FFFF && !(0xD800..=0xDFFF).contains(&cp)) {
        return 0;
    }
    if cp <= 0x7F {
        1
    } else if cp <= 0x7FF {
        2
    } else if cp <= 0xFFFF {
        3
    } else {
        4
    }
}

/// Number of UTF-16 code units for a valid scalar, 0 for invalid.
pub fn utf16_length(cp: u32) -> usize {
    if !(cp <= 0x10FFFF && !(0xD800..=0xDFFF).contains(&cp)) {
        return 0;
    }
    if cp <= 0xFFFF {
        1
    } else {
        2
    }
}

/// True when `cp` is a valid scalar.
pub fn is_valid(cp: u32) -> bool {
    cp <= 0x10FFFF && !(0xD800..=0xDFFF).contains(&cp)
}

/// True when `cp` is a replacement candidate (noncharacter/unassigned).
pub fn is_replacement(cp: u32) -> bool {
    ucd::has_property(cp, crate::generated::ucd_data::PROP_NONCHARACTER_CODE_POINT)
        || ucd::general_category(cp) == ucd::gc::CN
}
