//! UTS #46 Unicode IDNA Compatibility Processing (18.0.0, Revision 36).

use crate::uax::uax15;
use crate::ucd;

/// UTS #46 dispositions.
pub mod disp {
    pub const VALID: u8 = 0;
    pub const MAPPED: u8 = 1;
    pub const DEVIATION: u8 = 2;
    pub const IGNORED: u8 = 3;
    pub const DISALLOWED: u8 = 4;
}

/// Disposition of one scalar.
pub fn disposition(cp: u32) -> u8 {
    ucd::idna_status(cp)
}

/// Map a label (map + NFC).
pub fn map(input: &[u32]) -> Vec<u32> {
    let mut mapped = Vec::new();
    for &cp in input {
        match disposition(cp) {
            disp::IGNORED => {}
            disp::MAPPED | disp::DEVIATION => {
                if let Some(seq) = ucd::idna_mapping(cp) {
                    mapped.extend(seq);
                } else {
                    mapped.push(cp);
                }
            }
            _ => mapped.push(cp),
        }
    }
    uax15::normalize(&mapped, crate::normalization::NormalizationForm::Nfc)
}

/// True when `label` has no disallowed scalars.
pub fn check_label(label: &[u32]) -> bool {
    !label.is_empty() && label.iter().all(|&cp| disposition(cp) != disp::DISALLOWED)
}

/// True when `cp` is transitional-disallowed (NV8).
pub fn is_transitional_disallowed(cp: u32) -> bool {
    ucd::idna_nv8(cp)
}
