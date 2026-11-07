//! Unicode Character Database accessors backed by the generated Unicode 18.0.0
//! tables.
//!
//! The generated tables are shared with the C and C# implementations; this
//! module is the idiomatic Rust accessor layer over them.

use crate::generated::ucd_data as data;

/// General_Category values (UAX #44), in generated-table order.
pub mod gc {
    pub const CN: u8 = 0;
    pub const CC: u8 = 1;
    pub const CF: u8 = 2;
    pub const CO: u8 = 3;
    pub const CS: u8 = 4;
    pub const LL: u8 = 5;
    pub const LM: u8 = 6;
    pub const LO: u8 = 7;
    pub const LT: u8 = 8;
    pub const LU: u8 = 9;
    pub const MC: u8 = 10;
    pub const ME: u8 = 11;
    pub const MN: u8 = 12;
    pub const ND: u8 = 13;
    pub const NL: u8 = 14;
    pub const NO: u8 = 15;
    pub const PC: u8 = 16;
    pub const PD: u8 = 17;
    pub const PE: u8 = 18;
    pub const PF: u8 = 19;
    pub const PI: u8 = 20;
    pub const PO: u8 = 21;
    pub const PS: u8 = 22;
    pub const SC: u8 = 23;
    pub const SK: u8 = 24;
    pub const SM: u8 = 25;
    pub const SO: u8 = 26;
    pub const ZL: u8 = 27;
    pub const ZP: u8 = 28;
    pub const ZS: u8 = 29;
}

/// East_Asian_Width values (UAX #11), in generated-table order.
pub mod eaw {
    pub const N: u8 = 0;
    pub const A: u8 = 1;
    pub const H: u8 = 2;
    pub const F: u8 = 3;
    pub const NA: u8 = 4;
    pub const W: u8 = 5;
}

/// Grapheme_Cluster_Break values (UAX #29), in generated-table order.
pub mod gcb {
    pub const OTHER: u8 = 0;
    pub const CR: u8 = 1;
    pub const LF: u8 = 2;
    pub const CONTROL: u8 = 3;
    pub const EXTEND: u8 = 4;
    pub const ZWJ: u8 = 5;
    pub const RI: u8 = 6;
    pub const PREPEND: u8 = 7;
    pub const SPACINGMARK: u8 = 8;
    pub const L: u8 = 9;
    pub const V: u8 = 10;
    pub const T: u8 = 11;
    pub const LV: u8 = 12;
    pub const LVT: u8 = 13;
}

/// Word_Break values (UAX #29), in generated-table order.
pub mod wb {
    pub const OTHER: u8 = 0;
    pub const CR: u8 = 1;
    pub const LF: u8 = 2;
    pub const NEWLINE: u8 = 3;
    pub const EXTEND: u8 = 4;
    pub const ZWJ: u8 = 5;
    pub const RI: u8 = 6;
    pub const FORMAT: u8 = 7;
    pub const KATAKANA: u8 = 8;
    pub const HEBREW_LETTER: u8 = 9;
    pub const ALETTER: u8 = 10;
    pub const SINGLE_QUOTE: u8 = 11;
    pub const DOUBLE_QUOTE: u8 = 12;
    pub const MIDNUMLET: u8 = 13;
    pub const MIDLETTER: u8 = 14;
    pub const MIDNUM: u8 = 15;
    pub const NUMERIC: u8 = 16;
    pub const EXTENDNUMLET: u8 = 17;
    pub const WSEGSPACE: u8 = 18;
}

/// Sentence_Break values (UAX #29), in generated-table order.
pub mod sb {
    pub const OTHER: u8 = 0;
    pub const CR: u8 = 1;
    pub const LF: u8 = 2;
    pub const EXTEND: u8 = 3;
    pub const SEP: u8 = 4;
    pub const FORMAT: u8 = 5;
    pub const SP: u8 = 6;
    pub const LOWER: u8 = 7;
    pub const UPPER: u8 = 8;
    pub const OLETTER: u8 = 9;
    pub const NUMERIC: u8 = 10;
    pub const ATERM: u8 = 11;
    pub const SCONTINUE: u8 = 12;
    pub const STERM: u8 = 13;
    pub const CLOSE: u8 = 14;
}

/// Indic_Conjunct_Break values (UAX #29), in generated-table order.
pub mod incb {
    pub const NONE: u8 = 0;
    pub const LINKER: u8 = 1;
    pub const CONSONANT: u8 = 2;
    pub const EXTEND: u8 = 3;
}

/// Line_Break values (UAX #14), in generated-table order.
pub mod lb {
    pub const XX: u8 = 0;
    pub const AI: u8 = 1;
    pub const AK: u8 = 2;
    pub const AL: u8 = 3;
    pub const AP: u8 = 4;
    pub const AS: u8 = 5;
    pub const B2: u8 = 6;
    pub const BA: u8 = 7;
    pub const BB: u8 = 8;
    pub const BK: u8 = 9;
    pub const CB: u8 = 10;
    pub const CJ: u8 = 11;
    pub const CL: u8 = 12;
    pub const CM: u8 = 13;
    pub const CP: u8 = 14;
    pub const CR: u8 = 15;
    pub const EB: u8 = 16;
    pub const EM: u8 = 17;
    pub const EX: u8 = 18;
    pub const GL: u8 = 19;
    pub const H2: u8 = 20;
    pub const H3: u8 = 21;
    pub const HH: u8 = 22;
    pub const HL: u8 = 23;
    pub const HY: u8 = 24;
    pub const ID: u8 = 25;
    pub const IN: u8 = 26;
    pub const IS: u8 = 27;
    pub const JL: u8 = 28;
    pub const JT: u8 = 29;
    pub const JV: u8 = 30;
    pub const LF: u8 = 31;
    pub const NL: u8 = 32;
    pub const NS: u8 = 33;
    pub const NU: u8 = 34;
    pub const OP: u8 = 35;
    pub const PO: u8 = 36;
    pub const PR: u8 = 37;
    pub const QU: u8 = 38;
    pub const RI: u8 = 39;
    pub const SA: u8 = 40;
    pub const SG: u8 = 41;
    pub const SP: u8 = 42;
    pub const SY: u8 = 43;
    pub const VF: u8 = 44;
    pub const VI: u8 = 45;
    pub const WJ: u8 = 46;
    pub const ZW: u8 = 47;
    pub const ZWJ: u8 = 48;
}

/// Binary search a generated range table.
#[inline]
fn lookup(table: &[data::Range], cp: u32) -> u16 {
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
            return value;
        }
    }
    0
}

#[inline]
pub fn general_category(cp: u32) -> u8 {
    lookup(data::GC, cp) as u8
}

#[inline]
pub fn east_asian_width(cp: u32) -> u8 {
    lookup(data::EAW, cp) as u8
}

#[inline]
pub fn grapheme_break(cp: u32) -> u8 {
    lookup(data::GRAPHEME, cp) as u8
}

#[inline]
pub fn word_break(cp: u32) -> u8 {
    lookup(data::WORD, cp) as u8
}

#[inline]
pub fn sentence_break(cp: u32) -> u8 {
    lookup(data::SENTENCE, cp) as u8
}

#[inline]
pub fn line_break(cp: u32) -> u8 {
    lookup(data::LINE_BREAK_RANGE, cp) as u8
}

#[inline]
pub fn indic_conjunct_break(cp: u32) -> u8 {
    lookup(data::INCB, cp) as u8
}

/// Canonical Combining_Class (UAX #44); zero means a starter.
#[inline]
pub fn combining_class(cp: u32) -> u8 {
    lookup(data::CCC, cp) as u8
}

/// Vertical_Orientation (UAX #50), in generated-table order.
pub mod vo {
    pub const U: u8 = 0;
    pub const R: u8 = 1;
    pub const TU: u8 = 2;
    pub const TR: u8 = 3;
}

/// Vertical_Orientation value (UAX #50); unlisted code points are U.
#[inline]
pub fn vertical_orientation(cp: u32) -> u8 {
    lookup(data::VERTICAL, cp) as u8
}

/// Block index (UAX #44); the value indexes `BLOCK_NAMES`.
#[inline]
pub fn block(cp: u32) -> u16 {
    lookup(data::BLOCK_RANGE, cp) as u16
}

#[inline]
fn pair_lookup(table: &[data::Pair], cp: u32, fallback: u32) -> u32 {
    let mut lo = 0usize;
    let mut hi = table.len();
    while lo < hi {
        let mid = lo + (hi - lo) / 2;
        let (start, mapped) = table[mid];
        if cp < start {
            hi = mid;
        } else if cp > start {
            lo = mid + 1;
        } else {
            return mapped;
        }
    }
    fallback
}

#[inline]
pub fn simple_uppercase(cp: u32) -> u32 {
    pair_lookup(data::SIMPLE_UPPER, cp, cp)
}

#[inline]
pub fn simple_lowercase(cp: u32) -> u32 {
    pair_lookup(data::SIMPLE_LOWER, cp, cp)
}

#[inline]
pub fn simple_titlecase(cp: u32) -> u32 {
    pair_lookup(data::SIMPLE_TITLE, cp, cp)
}

#[inline]
pub fn simple_casefold(cp: u32) -> u32 {
    pair_lookup(data::SIMPLE_FOLD, cp, cp)
}

#[inline]
pub fn has_property(cp: u32, table: &[data::Range]) -> bool {
    lookup(table, cp) != 0
}

/// Decomposition mapping for `cp`, if any.
///
/// When `compatibility` is false only canonical decompositions are returned.
pub fn decomposition(cp: u32, compatibility: bool) -> Option<Vec<u32>> {
    let mut lo = 0usize;
    let mut hi = data::DECOMP.len();
    while lo < hi {
        let mid = lo + (hi - lo) / 2;
        let (start, offset, length, kind) = data::DECOMP[mid];
        if cp < start {
            hi = mid;
        } else if cp > start {
            lo = mid + 1;
        } else {
            if !compatibility && kind == 1 {
                return None;
            }
            let begin = offset as usize;
            let end = begin + length as usize;
            return Some(data::POOL[begin..end].to_vec());
        }
    }
    None
}

/// True when `cp` has a decomposition mapping (canonical only, or any).
pub fn has_decomposition(cp: u32, compatibility: bool) -> bool {
    decomposition(cp, compatibility).is_some()
}

/// Canonical composition for the pair (`first`, `second`), if any.
pub fn compose_pair(first: u32, second: u32) -> Option<u32> {
    let mut lo = 0usize;
    let mut hi = data::COMPOSITION.len();
    while lo < hi {
        let mid = lo + (hi - lo) / 2;
        let (f, s, composed) = data::COMPOSITION[mid];
        if first < f || (first == f && second < s) {
            hi = mid;
        } else if first > f || (first == f && second > s) {
            lo = mid + 1;
        } else {
            return Some(composed);
        }
    }
    None
}

/// UTS #58 Link_Term values: 1 Include, 2 Hard, 3 Soft, 4 Close, 5 Open.
/// Code points not listed have the value 2 (Hard).
#[inline]
pub fn link_term(cp: u32) -> u8 {
    lookup(data::LINK_TERM_RANGE, cp) as u8
}

/// UTS #58 Link_Bracket: maps a Close bracket code point to its opening
/// counterpart.  `None` when there is no mapping.
pub fn link_bracket(cp: u32) -> Option<u32> {
    pair_lookup_checked(data::LINK_BRACKET, cp)
}

/// UTS #58 Link_Email binary property.
#[inline]
pub fn link_email(cp: u32) -> bool {
    lookup(data::LINK_EMAIL_RANGE, cp) != 0
}

/// UTR #59 East_Asian_Spacing values: 1 W, 2 N, 3 O, 4 C.
/// Code points not listed have the value 3 (O).
#[inline]
pub fn east_asian_spacing(cp: u32) -> u8 {
    lookup(data::EAST_ASIAN_SPACING_RANGE, cp) as u8
}

/// UAX #34 named sequences.
#[inline]
pub fn named_seq_count() -> usize {
    data::NAMED_SEQ_NAMES.len()
}

pub fn named_seq_name(index: usize) -> Option<&'static str> {
    data::NAMED_SEQ_NAMES.get(index).copied()
}

pub fn named_seq_cps(index: usize) -> Option<&'static [u32]> {
    let (offset, length) = *data::NAMED_SEQ_INDEX.get(index)?;
    let begin = offset as usize;
    Some(&data::POOL[begin..begin + length as usize])
}

pub fn named_seq_find(name: &str) -> Option<usize> {
    data::NAMED_SEQ_NAMES.iter().position(|n| *n == name)
}

/// UTS #51 RGI emoji sequences (emoji-sequences.txt and
/// emoji-zwj-sequences.txt); `emoji_seq_type` returns the official type
/// string from the data files.
#[inline]
pub fn emoji_seq_count() -> usize {
    data::EMOJI_SEQ_TYPES.len()
}

pub fn emoji_seq_type(index: usize) -> Option<&'static str> {
    data::EMOJI_SEQ_TYPES.get(index).copied()
}

pub fn emoji_seq_cps(index: usize) -> Option<&'static [u32]> {
    let (offset, length) = *data::EMOJI_SEQ_INDEX.get(index)?;
    let begin = offset as usize;
    Some(&data::POOL[begin..begin + length as usize])
}

/// One collation element weight triple (UTS #10 DUCET).
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct CollationWeight {
    pub primary: u16,
    pub secondary: u16,
    pub tertiary: u16,
    pub variable: bool,
}

fn push_collation_weights(offset: usize, length: usize, out: &mut Vec<CollationWeight>) {
    for i in 0..length {
        let base = (offset + i) * 4;
        out.push(CollationWeight {
            primary: data::COLLATION_KEYS[base] as u16,
            secondary: data::COLLATION_KEYS[base + 1] as u16,
            tertiary: data::COLLATION_KEYS[base + 2] as u16,
            variable: data::COLLATION_KEYS[base + 3] != 0,
        });
    }
}

/// Single-code-point collation elements.  Returns the number of elements, or
/// zero when the code point has no explicit DUCET mapping.
pub fn collation_weights(cp: u32, out: &mut Vec<CollationWeight>) -> usize {
    let mut lo = 0usize;
    let mut hi = data::COLLATION_SINGLE.len();
    while lo < hi {
        let mid = lo + (hi - lo) / 2;
        let (entry_cp, offset, length) = data::COLLATION_SINGLE[mid];
        if cp < entry_cp {
            hi = mid;
        } else if cp > entry_cp {
            lo = mid + 1;
        } else {
            push_collation_weights(offset as usize, length as usize, out);
            return length as usize;
        }
    }
    0
}

/// Longest contraction matching the prefix of `cps`.  Returns
/// `(matched_code_points, element_count)` or `None`.
pub fn collation_contraction(
    cps: &[u32],
    out: &mut Vec<CollationWeight>,
) -> Option<(usize, usize)> {
    let mut best_len = 0usize;
    let mut best_offset = 0usize;
    let mut best_count = 0usize;

    for &(encoded, offset, length) in data::COLLATION_MULTI {
        let n = (encoded >> 24) as usize;
        let off = (encoded & 0x00FF_FFFF) as usize;
        if n == 0 || n > cps.len() || n <= best_len {
            continue;
        }
        if data::POOL[off..off + n] == cps[..n] {
            best_len = n;
            best_offset = offset as usize;
            best_count = length as usize;
        }
    }

    if best_len == 0 {
        return None;
    }
    push_collation_weights(best_offset, best_count, out);
    Some((best_len, best_count))
}

/// Implicit weight parameters for a code point in an `@implicitweights` range:
/// `(base_AAAA, offset_from_group_origin)`.
pub fn implicit_weight(cp: u32) -> Option<(u16, u32)> {
    for &(start, end, base, offset_base) in data::IMPLICIT {
        if cp >= start && cp <= end {
            return Some((base as u16, cp - offset_base));
        }
    }
    None
}

#[inline]
fn pair_lookup_checked(table: &[data::Pair], cp: u32) -> Option<u32> {
    let mut lo = 0usize;
    let mut hi = table.len();
    while lo < hi {
        let mid = lo + (hi - lo) / 2;
        let (start, mapped) = table[mid];
        if cp < start {
            hi = mid;
        } else if cp > start {
            lo = mid + 1;
        } else {
            return Some(mapped);
        }
    }
    None
}

#[inline]
pub fn extended_pictographic(cp: u32) -> bool {
    lookup(data::PROP_EXTENDED_PICTOGRAPHIC, cp) != 0
}

/// The Unicode version described by the generated tables.
pub const VERSION: &str = data::UCD_VERSION;
