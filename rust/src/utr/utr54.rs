//! UTR #54 Unicode Mongolian 12.1 Baseline (Revision 1).

/// True when `cp` is in the Mongolian block.
pub fn is_mongolian(cp: u32) -> bool {
    (0x1800..=0x18AF).contains(&cp)
}

/// True when `cp` is a Mongolian Free Variation Selector.
pub fn is_fvs(cp: u32) -> bool {
    matches!(cp, 0x180B | 0x180C | 0x180D | 0x180F)
}

/// True when `cp` is the Mongolian Vowel Separator.
pub fn is_mvs(cp: u32) -> bool {
    cp == 0x180E
}
