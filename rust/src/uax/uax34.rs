//! UAX #34 Unicode Named Character Sequences (Unicode 18.0.0, Revision 23).
//!
//! Exposes the normative `NamedSequences.txt` data and validates scalar
//! sequences against it.  Names are compared exactly; callers that need loose
//! matching apply their own normalization first.

use crate::ucd;

/// Number of registered named character sequences.
pub fn count() -> usize {
    ucd::named_seq_count()
}

/// Name of the sequence at `index`, or `None` when out of range.
pub fn name(index: usize) -> Option<&'static str> {
    ucd::named_seq_name(index)
}

/// Code points of the sequence at `index`, or `None` when out of range.
pub fn sequence(index: usize) -> Option<&'static [u32]> {
    ucd::named_seq_cps(index)
}

/// Look up a sequence by exact name.
pub fn find(name: &str) -> Option<&'static [u32]> {
    ucd::named_seq_find(name).and_then(ucd::named_seq_cps)
}

fn valid_scalar(cp: u32) -> bool {
    cp <= 0x10FFFF && !(0xD800..=0xDFFF).contains(&cp)
}

/// True when `cps` is exactly one of the registered sequences.
///
/// Ill-formed scalar values (surrogates and out-of-range code points) are
/// rejected before the table is consulted.
pub fn is_registered(cps: &[u32]) -> bool {
    if cps.is_empty() || !cps.iter().all(|&cp| valid_scalar(cp)) {
        return false;
    }
    (0..count()).any(|index| sequence(index) == Some(cps))
}
