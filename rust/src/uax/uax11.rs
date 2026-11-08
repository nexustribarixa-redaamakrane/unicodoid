//! UAX #11 East Asian Width (Unicode 18.0.0, Revision 46).
//!
//! Thin scalar API over the generated East_Asian_Width data.

use crate::ucd;

/// East_Asian_Width values.
pub mod eaw {
    pub const N: u8 = 0;
    pub const A: u8 = 1;
    pub const H: u8 = 2;
    pub const F: u8 = 3;
    pub const NA: u8 = 4;
    pub const W: u8 = 5;
}

/// Raw East_Asian_Width of `cp`.
#[inline]
pub fn eaw(cp: u32) -> u8 {
    ucd::east_asian_width(cp)
}

/// True when `cp` is Fullwidth or Wide.
#[inline]
pub fn is_wide(cp: u32) -> bool {
    matches!(eaw(cp), eaw::F | eaw::W)
}

/// Terminal column count of `cp` (A counts as wide when `ambiguous_as_wide`).
pub fn column_count(cp: u32, ambiguous_as_wide: bool) -> usize {
    match eaw(cp) {
        eaw::F | eaw::W => 2,
        eaw::A if ambiguous_as_wide => 2,
        _ => 1,
    }
}

/// Column count of a scalar sequence.
pub fn columns(cps: &[u32], ambiguous_as_wide: bool) -> usize {
    cps.iter().map(|&cp| column_count(cp, ambiguous_as_wide)).sum()
}
