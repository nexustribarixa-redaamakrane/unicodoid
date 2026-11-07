//! UAX #29 Unicode Text Segmentation (Unicode 18.0.0, Revision 47).
//!
//! Scalar-based extended grapheme cluster boundaries.  This is the shared
//! primitive used by the UTR #59 East Asian spacing algorithm and is exposed
//! for callers that already hold decoded scalar values.  The byte-oriented
//! [`crate::segmentation::Segmenter`] applies the same rules.

use crate::segmentation;

/// Find the end of the extended grapheme cluster starting at `start`.
///
/// Applies the extended grapheme cluster boundary rules GB3..GB13 and GB999
/// against the generated Unicode 18.0.0 Grapheme_Cluster_Break,
/// Indic_Conjunct_Break and Extended_Pictographic data.
///
/// Returns `scalars.len()` when `start` is at or beyond the end.
pub fn next_grapheme(scalars: &[u32], start: usize) -> usize {
    if start >= scalars.len() {
        return scalars.len();
    }
    segmentation::next_grapheme_cluster(scalars, start)
}

/// True when a grapheme cluster boundary occurs immediately before `index`.
///
/// `index == 0` and `index == scalars.len()` are always boundaries.
pub fn is_grapheme_boundary(scalars: &[u32], index: usize) -> bool {
    if index == 0 || index == scalars.len() {
        return true;
    }
    if index > scalars.len() {
        return false;
    }
    let mut start = 0usize;
    while start < scalars.len() {
        if start == index {
            return true;
        }
        if start > index {
            return false;
        }
        start = next_grapheme(scalars, start);
    }
    index == scalars.len()
}
