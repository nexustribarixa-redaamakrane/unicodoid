//! UAX #38 Unicode Han Database (Unihan) (Unicode 18.0.0, Revision 41).

fn lookup(table: &[crate::generated::ucd_data::Range], cp: u32) -> bool {
    let mut lo = 0usize;
    let mut hi = table.len();
    while lo < hi {
        let mid = lo + (hi - lo) / 2;
        let (start, end, value) = table[mid];
        if cp < start {
            hi = mid;
        } else if cp > end {
            lo = mid + 1;
        } else {
            return value != 0;
        }
    }
    false
}

/// True when `cp` is a Unified_Ideograph.
pub fn is_unified_ideograph(cp: u32) -> bool {
    lookup(crate::generated::ucd_data::PROP_UNIFIED_IDEOGRAPH, cp)
}

/// True when `cp` is a CJK Radical.
pub fn is_radical(cp: u32) -> bool {
    lookup(crate::generated::ucd_data::PROP_RADICAL, cp)
        || (0x2F00..=0x2FDF).contains(&cp)
        || (0x2E80..=0x2FFF).contains(&cp)
}

/// Radical index (1-based), or 0 when not a radical.
pub fn radical_index(cp: u32) -> u32 {
    if (0x2F00..=0x2FDF).contains(&cp) {
        cp - 0x2F00 + 1
    } else if is_radical(cp) {
        1
    } else {
        0
    }
}

/// True when `cp` lies in a CJK block.
pub fn is_cjk_block(cp: u32) -> bool {
    matches!(cp,
        0x3000..=0x303F | 0x3040..=0x309F | 0x30A0..=0x30FF | 0x3100..=0x312F
        | 0x3130..=0x318F | 0x3190..=0x31FF | 0x3200..=0x32FF | 0x3300..=0x33FF
        | 0x3400..=0x4DBF | 0x4E00..=0x9FFF | 0xA000..=0xA4CF | 0xAC00..=0xD7AF
        | 0xF900..=0xFAFF | 0xFE30..=0xFE4F | 0xFF00..=0xFF60
        | 0x20000..=0x2A6DF | 0x2A700..=0x2CEAF | 0x2CEB0..=0x2EBEF
        | 0x30000..=0x323AF)
}
