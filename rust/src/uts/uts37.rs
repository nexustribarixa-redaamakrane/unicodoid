//! UTS #37 Unicode Ideographic Variation Database (Revision 16).

/// True when `cp` is an ideographic variation selector.
pub fn is_variation_selector(cp: u32) -> bool {
    (0xE0100..=0xE01EF).contains(&cp)
}

/// Selector offset (0-based), or None.
pub fn selector_offset(cp: u32) -> Option<u32> {
    if is_variation_selector(cp) {
        Some(cp - 0xE0100)
    } else {
        None
    }
}

/// True when `cps` is an IVS (base + selector).
pub fn is_ivs(cps: &[u32]) -> bool {
    cps.len() == 2
        && cps[0] <= 0x10FFFF
        && !(0xD800..=0xDFFF).contains(&cps[0])
        && is_variation_selector(cps[1])
}
