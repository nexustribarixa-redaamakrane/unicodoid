//! UTS #39 Unicode Security Mechanisms (Unicode 18.0.0, Revision 34).

use crate::ucd;

/// Confusable mapping of one scalar, if any.
pub fn confusable(cp: u32) -> Option<Vec<u32>> {
    ucd::confusable(cp)
}

/// True when `cp` is Default_Ignorable_Code_Point.
pub fn is_default_ignorable(cp: u32) -> bool {
    ucd::has_property(
        cp,
        crate::generated::ucd_data::PROP_DEFAULT_IGNORABLE_CODE_POINT,
    )
}

/// Visual skeleton of a sequence (confusable folding, bounded rounds).
pub fn skeleton(cps: &[u32]) -> Vec<u32> {
    let mut out = Vec::new();
    for &cp in cps {
        let mut cur = vec![cp];
        for _ in 0..3 {
            let mut next = Vec::new();
            for &c in &cur {
                match ucd::confusable(c) {
                    Some(mapped) => next.extend(mapped),
                    None => next.push(c),
                }
            }
            if next == cur {
                break;
            }
            cur = next;
            if cur.is_empty() {
                break;
            }
        }
        out.extend(cur);
    }
    out
}

/// True when skeletons of `a` and `b` are equal.
pub fn are_confusable(a: &[u32], b: &[u32]) -> bool {
    skeleton(a) == skeleton(b)
}
