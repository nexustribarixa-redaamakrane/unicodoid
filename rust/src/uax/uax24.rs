//! UAX #24 Unicode Script Property (Unicode 18.0.0, Revision 41).

use crate::ucd;

/// Number of known Script values (including Unknown at index 0).
pub fn count() -> usize {
    ucd::script_count()
}

/// Script name for `value`, or None when out of range.
pub fn script_name(value: u16) -> Option<&'static str> {
    ucd::script_name(value)
}

/// Script value of `cp`.
#[inline]
pub fn script(cp: u32) -> u16 {
    ucd::script(cp)
}

/// True when `cp` belongs to Script value `script`.
#[inline]
pub fn is_script(cp: u32, script: u16) -> bool {
    ucd::script(cp) == script
}

/// Find a Script value by name; 0 (Unknown) when absent.
pub fn find(name: &str) -> u16 {
    for i in 0..ucd::script_count() {
        if ucd::script_name(i as u16) == Some(name) {
            return i as u16;
        }
    }
    0
}
