//! UTR #25 Unicode Support for Mathematics (Revision 16, Draft 5).

use crate::ucd;

/// True when `cp` has Math = Yes.
pub fn is_math(cp: u32) -> bool {
    ucd::is_math(cp)
}

/// True when `cp` is in the Mathematical Alphanumeric block.
pub fn is_math_alphanumeric(cp: u32) -> bool {
    (0x1D400..=0x1D7FF).contains(&cp)
}

/// True when `cp` is a mathematical operator (Math + Sm/Sc).
pub fn is_operator(cp: u32) -> bool {
    is_math(cp) && matches!(ucd::general_category(cp), ucd::gc::SM | ucd::gc::SC)
}
