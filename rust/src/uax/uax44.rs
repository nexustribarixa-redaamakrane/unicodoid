//! UAX #44 Unicode Character Database (Unicode 18.0.0, Revision 38).

use crate::ucd;

/// Canonical UCD version.
pub const VERSION: &str = ucd::VERSION;

/// Long property name for a short alias, or None when unknown.
pub fn property_alias(alias: &str) -> Option<&'static str> {
    match alias {
        "gc" => Some("General_Category"),
        "bc" => Some("Bidi_Class"),
        "ea" => Some("East_Asian_Width"),
        "lb" => Some("Line_Break"),
        "sc" => Some("Script"),
        _ => None,
    }
}

/// True when `cp` has a casefold mapping distinct from itself.
pub fn has_casefold(cp: u32) -> bool {
    ucd::simple_casefold(cp) != cp
}

/// Block name for `cp`.
pub fn block_name(cp: u32) -> Option<&'static str> {
    crate::generated::ucd_data::BLOCK_NAMES
        .get(ucd::block(cp) as usize)
        .copied()
}
