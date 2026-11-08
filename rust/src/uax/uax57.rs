//! UAX #57 Unicode Egyptian Hieroglyph Database (Unikemet)
//! (Unicode 18.0.0, Revision 7).

/// True when `cp` is an Egyptian hieroglyph.
pub fn is_hieroglyph(cp: u32) -> bool {
    (0x13000..=0x1342F).contains(&cp) || (0x13460..=0x143FF).contains(&cp)
}

/// True when `cp` is in the Core set (Extended-A block).
pub fn is_core(cp: u32) -> bool {
    (0x13460..=0x143FF).contains(&cp)
}

/// True when `cp` is a Legacy sign (original block).
pub fn is_legacy(cp: u32) -> bool {
    (0x13000..=0x1342F).contains(&cp)
}

/// True when `cp` must not be mirrored (well-known pair).
pub fn is_no_mirror(cp: u32) -> bool {
    matches!(cp, 0x130BB | 0x130BD)
}
