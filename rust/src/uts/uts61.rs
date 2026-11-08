//! UTS #61 Unicode Set Notation (Version 1, Draft 4).
//!
//! Draft core: single `\p{...}` membership and `X..Y` hex ranges.

use crate::uts::uts18;

/// Parse `X..Y` hex range text.
pub fn parse_range(text: &str) -> Option<(u32, u32)> {
    let (lo_s, hi_s) = text.split_once("..")?;
    let lo = u32::from_str_radix(lo_s.trim(), 16).ok()?;
    let hi = u32::from_str_radix(hi_s.trim(), 16).ok()?;
    if lo > 0x10FFFF || hi > 0x10FFFF || lo > hi {
        return None;
    }
    Some((lo, hi))
}

/// True when `cp` lies in `[lo, hi]`.
pub fn in_range(cp: u32, lo: u32, hi: u32) -> bool {
    (lo..=hi).contains(&cp)
}

/// Membership test: 1 in, 0 out, -1 on parse error.
pub fn set_membership(set: &str, cp: u32) -> i32 {
    if set.starts_with(r"\p{") || set.starts_with(r"\P{") {
        let m = uts18::property_match(set, cp);
        if m == uts18::kind::INVALID {
            return -1;
        }
        return i32::from(m != uts18::kind::NONE);
    }
    match parse_range(set) {
        Some((lo, hi)) => i32::from(in_range(cp, lo, hi)),
        None => -1,
    }
}
