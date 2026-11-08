//! UAX #45 U-source Ideographs (Unicode 18.0.0, Revision 34).

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

/// True when `cp` is a U-source ideograph.
pub fn is_usource(cp: u32) -> bool {
    lookup(crate::generated::ucd_data::PROP_UNIFIED_IDEOGRAPH, cp)
        || (0x20000..=0x2FA1F).contains(&cp)
        || (0x30000..=0x3134F).contains(&cp)
}

/// True when `cp` is in the US1 staging area.
pub fn is_us1_staging(cp: u32) -> bool {
    (0xF900..=0xFAFF).contains(&cp)
}

/// US-source family name, or None.
pub fn usource_family(cp: u32) -> Option<&'static str> {
    if is_us1_staging(cp) {
        Some("US1")
    } else if is_usource(cp) {
        Some("US")
    } else {
        None
    }
}
