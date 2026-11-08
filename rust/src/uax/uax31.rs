//! UAX #31 Unicode Identifiers and Syntax (Unicode 18.0.0, Revision 45).

use crate::generated::ucd_data as data;

fn lookup(table: &[data::Range], cp: u32) -> bool {
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

fn valid_scalar(cp: u32) -> bool {
    cp <= 0x10FFFF && !(0xD800..=0xDFFF).contains(&cp)
}

/// True when `cp` is XID_Start.
#[inline]
pub fn is_xid_start(cp: u32) -> bool {
    lookup(data::PROP_XID_START, cp)
}

/// True when `cp` is XID_Continue.
#[inline]
pub fn is_xid_continue(cp: u32) -> bool {
    lookup(data::PROP_XID_CONTINUE, cp)
}

/// True when `cp` is ID_Start.
#[inline]
pub fn is_id_start(cp: u32) -> bool {
    lookup(data::PROP_ID_START, cp)
}

/// True when `cp` is ID_Continue.
#[inline]
pub fn is_id_continue(cp: u32) -> bool {
    lookup(data::PROP_ID_CONTINUE, cp)
}

/// Validate an identifier: XID_Start (XID_Continue | ZWNJ | ZWJ)*.
pub fn is_valid(cps: &[u32]) -> bool {
    if cps.is_empty() || !cps.iter().all(|&cp| valid_scalar(cp)) {
        return false;
    }
    if !is_xid_start(cps[0]) {
        return false;
    }
    cps[1..].iter().all(|&cp| cp == 0x200C || cp == 0x200D || is_xid_continue(cp))
}

/// True when `cps` is NFKC-stable.
pub fn is_nfkc_stable(cps: &[u32]) -> bool {
    if cps.is_empty() {
        return true;
    }
    crate::uax::uax15::normalize(cps, crate::normalization::NormalizationForm::Nfkc) == cps
}
