//! UTR #56 Unicode Cuneiform Sign Lists (Revision 5).

/// True when `cp` is in Cuneiform or its extensions.
pub fn is_cuneiform(cp: u32) -> bool {
    (0x12000..=0x1247F).contains(&cp)
        || (0x12480..=0x125FF).contains(&cp)
        || (0x14400..=0x1467F).contains(&cp)
}

/// True when `cp` is a Cuneiform number.
pub fn is_number(cp: u32) -> bool {
    (0x12480..=0x125FF).contains(&cp)
}
