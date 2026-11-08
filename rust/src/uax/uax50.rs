//! UAX #50 Unicode Vertical Text Layout (Unicode 18.0.0, Revision 35).

use crate::ucd;

/// Vertical_Orientation values.
pub mod vo {
    pub const U: u8 = 0;
    pub const R: u8 = 1;
    pub const TU: u8 = 2;
    pub const TR: u8 = 3;
}

/// Raw Vertical_Orientation of `cp`.
#[inline]
pub fn vertical_orientation(cp: u32) -> u8 {
    ucd::vertical_orientation(cp)
}

/// Resolved orientation (U or R).
pub fn resolved_orientation(cp: u32) -> u8 {
    match vertical_orientation(cp) {
        vo::TR => vo::R,
        vo::TU => vo::U,
        other => other,
    }
}

/// True when `cp` should display upright in vertical layout.
pub fn is_upright(cp: u32) -> bool {
    resolved_orientation(cp) == vo::U
}
