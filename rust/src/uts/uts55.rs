//! UTS #55 Unicode Source Code Handling (Revision 5).

use crate::generated::ucd_data as data;
use crate::ucd;

fn lookup(table: &[crate::generated::ucd_data::Range], cp: u32) -> bool {
    ucd::has_property(cp, table)
}

/// True when `cp` is a Bidi control.
pub fn is_bidi_control(cp: u32) -> bool {
    lookup(data::PROP_BIDI_CONTROL, cp)
}

/// True when `cp` is ZWJ/ZWNJ.
pub fn is_joiner(cp: u32) -> bool {
    matches!(cp, 0x200C | 0x200D)
}

/// Number of flagged characters in `cps`.
pub fn flag_count(cps: &[u32]) -> usize {
    cps.iter().filter(|&&cp| is_bidi_control(cp) || is_joiner(cp)).count()
}
