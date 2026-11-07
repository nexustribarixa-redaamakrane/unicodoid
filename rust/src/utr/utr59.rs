//! UTR #59 East Asian Spacing (Unicode 18.0.0, Revision 1 draft).
//!
//! Implements the informative `East_Asian_Spacing` property and the spacing
//! algorithm of UTR59 §3.3 against the official derived data file
//! (`east-asian-spacing.txt`) generated for Unicode 18.0.0.
//!
//! The algorithm operates on extended grapheme clusters (UAX #29).  It does
//! not decide the visual width of the inserted space; callers choose whether
//! to add a glyph advance or a space character such as U+2009.

use crate::uax::uax29;
use crate::ucd;

/// `East_Asian_Spacing` property values (UTR59 Table 1).
pub mod eas {
    /// Wide: East Asian script character.
    pub const W: u8 = 1;
    /// Narrow: needs spacing next to W.
    pub const N: u8 = 2;
    /// Other: no auto-spacing.
    pub const O: u8 = 3;
    /// Conditional: N for Chinese, else O.
    pub const C: u8 = 4;
}

/// Raw `East_Asian_Spacing` value for a code point (never resolves C).
pub fn east_asian_spacing(cp: u32) -> u8 {
    ucd::east_asian_spacing(cp)
}

/// UTR59 §3.3.1 step 4: upright when Vertical_Orientation is U, Tu or Tr.
fn is_upright(cp: u32) -> bool {
    let vo = ucd::vertical_orientation(cp);
    vo == ucd::vo::U || vo == ucd::vo::TU || vo == ucd::vo::TR
}

/// Resolve the property value of one extended grapheme cluster.
///
/// Implements UTR59 §3.3.1 steps 2-7:
///
/// * take the value of the first code point,
/// * force O when the cluster contains an enclosing mark (General_Category
///   Me),
/// * resolve C to N when `chinese` is true, otherwise to O,
/// * in vertical layout, turn N into O when the first code point is displayed
///   upright (Vertical_Orientation U, Tu or Tr).
pub fn cluster_value(cps: &[u32], chinese: bool, vertical: bool) -> u8 {
    if cps.is_empty() {
        return eas::O;
    }

    // Step 3: the first code point determines the initial value.
    let mut value = east_asian_spacing(cps[0]);

    // UTR59 §3.4.1: an enclosing mark forces the whole cluster to O.
    if cps
        .iter()
        .any(|&cp| ucd::general_category(cp) == ucd::gc::ME)
    {
        value = eas::O;
    }

    // Steps 4-5: vertical layout, upright Narrow characters become Other.
    if vertical && value == eas::N && is_upright(cps[0]) {
        value = eas::O;
    }

    // Step 6: resolve Conditional.
    if value == eas::C {
        value = if chinese { eas::N } else { eas::O };
    }

    value
}

/// Compute where auto-spacing is inserted.
///
/// Iterates extended grapheme clusters and records every boundary between a W
/// cluster and an N cluster (in either order).  Each recorded offset is the
/// scalar index of the first code point of the following cluster, i.e. a space
/// belongs immediately before `cps[offset]`.
pub fn spacing_offsets(cps: &[u32], chinese: bool, vertical: bool) -> Vec<usize> {
    let mut offsets = Vec::new();
    if cps.is_empty() {
        return offsets;
    }

    let mut start = 0usize;
    let mut end = uax29::next_grapheme(cps, start);
    let mut prev = cluster_value(&cps[start..end], chinese, vertical);

    while end < cps.len() {
        let next_start = end;
        let next_end = uax29::next_grapheme(cps, next_start);
        let next = cluster_value(&cps[next_start..next_end], chinese, vertical);

        let insert = (prev == eas::W && next == eas::N) || (prev == eas::N && next == eas::W);
        if insert {
            offsets.push(next_start);
        }

        prev = next;
        start = next_start;
        end = next_end;
    }

    let _ = start;
    offsets
}
