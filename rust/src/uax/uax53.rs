//! UAX #53 Unicode Arabic Mark Rendering (Unicode 18.0.0, Revision 13).

use crate::ucd;

/// Joining_Type values.
pub mod joining {
    pub const U: u8 = 0;
    pub const C: u8 = 1;
    pub const D: u8 = 2;
    pub const L: u8 = 3;
    pub const R: u8 = 4;
    pub const T: u8 = 5;
}

/// Joining_Type of `cp`.
#[inline]
pub fn joining_type(cp: u32) -> u8 {
    ucd::joining_type(cp)
}

/// True when `cp` joins transparently (Mn/Me/Cf).
pub fn is_joining_transparent(cp: u32) -> bool {
    matches!(ucd::general_category(cp), ucd::gc::MN | ucd::gc::ME | ucd::gc::CF)
}

/// Shaping integrity check (invalid scalars rejected).
pub fn is_consistent(cps: &[u32]) -> bool {
    cps.iter().all(|&cp| cp <= 0x10FFFF && !(0xD800..=0xDFFF).contains(&cp))
}
