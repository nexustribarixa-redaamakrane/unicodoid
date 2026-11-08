//! UAX #60 Data for East Asian Scripts (Unicode 18.0.0, Revision 3).

use crate::ucd;

/// True when `cp` is Han.
pub fn is_han(cp: u32) -> bool {
    ucd::script_name(ucd::script(cp)) == Some("Han")
}

/// True when `cp` has an East Asian script.
pub fn is_ea_script(cp: u32) -> bool {
    matches!(
        ucd::script_name(ucd::script(cp)),
        Some("Han")
            | Some("Hiragana")
            | Some("Katakana")
            | Some("Hangul")
            | Some("Bopomofo")
            | Some("Yi")
            | Some("Tangut")
            | Some("Nushu")
            | Some("Khitan_Small_Script")
            | Some("Jurchen")
    )
}

/// True when `cp` is East Asian Wide/Fullwidth.
pub fn is_ea_wide(cp: u32) -> bool {
    matches!(ucd::east_asian_width(cp), ucd::eaw::W | ucd::eaw::F)
}
