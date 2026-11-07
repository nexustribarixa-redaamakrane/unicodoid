//! Unicode Collation Algorithm (UTS #10) for Unicode 18.0.0.
//!
//! This is a conformant implementation of the Default Unicode Collation
//! Element Table (DUCET) in its NON_IGNORABLE variable-weighting
//! configuration:
//!
//! * NFD normalization as the mandatory first step (S1.1),
//! * collation element construction with longest contiguous matches and
//!   discontiguous non-starter extension with blocking (S2.1-S2.5),
//! * three-level sort key formation with 16-bit weights (S3.1-S3.9).
//!
//! The DUCET weights come from the generated Unicode 18.0.0 tables in
//! [`crate::generated::ucd_data`].  CLDR tailorings, script reordering,
//! backward secondary levels and matching/searching are outside this module.
//!
//! [`CollationOptions::numeric_ordering`] and [`CollationOptions::case_level`]
//! are Unicodoid extensions layered on top of the UCA levels; the default
//! options exercise the standard algorithm.

use crate::codec::Codec;
use crate::generated::ucd_data as data;
use crate::properties::{Category, general_category};
use crate::text_string::TextString;
use crate::types::{DecodeOutcome, Status};
use crate::ucd;

/// Collation strength levels.
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash)]
#[repr(u8)]
pub enum CollationStrength {
    Primary = 1,
    Secondary = 2,
    Tertiary = 3,
    Quaternary = 4,
    Identical = 5,
}

/// Options controlling collation.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct CollationOptions {
    pub strength: CollationStrength,
    /// Compare a dedicated case weight after the secondary level.  This is an
    /// extension over plain UCA and is disabled by default.
    pub case_level: bool,
    /// Compare runs of ASCII digits numerically.  This is an extension over
    /// plain UCA and is disabled by default.
    pub numeric_ordering: bool,
}

impl Default for CollationOptions {
    fn default() -> Self {
        Self {
            strength: CollationStrength::Tertiary,
            case_level: false,
            numeric_ordering: false,
        }
    }
}

impl CollationOptions {
    pub const fn new() -> Self {
        Self {
            strength: CollationStrength::Tertiary,
            case_level: false,
            numeric_ordering: false,
        }
    }
}

/* ------------------------------------------------------------------------- */
/* NFD normalization (S1.1)                                                   */
/* ------------------------------------------------------------------------- */

const HANGUL_SBASE: u32 = 0xAC00;
const HANGUL_LBASE: u32 = 0x1100;
const HANGUL_VBASE: u32 = 0x1161;
const HANGUL_TBASE: u32 = 0x11A7;
const HANGUL_LCOUNT: u32 = 19;
const HANGUL_VCOUNT: u32 = 21;
const HANGUL_TCOUNT: u32 = 28;
const HANGUL_NCOUNT: u32 = HANGUL_VCOUNT * HANGUL_TCOUNT;
const HANGUL_SCOUNT: u32 = HANGUL_LCOUNT * HANGUL_NCOUNT;

fn decompose_into(cp: u32, out: &mut Vec<u32>) {
    if (HANGUL_SBASE..HANGUL_SBASE + HANGUL_SCOUNT).contains(&cp) {
        let sindex = cp - HANGUL_SBASE;
        out.push(HANGUL_LBASE + sindex / HANGUL_NCOUNT);
        out.push(HANGUL_VBASE + (sindex % HANGUL_NCOUNT) / HANGUL_TCOUNT);
        let t = HANGUL_TBASE + sindex % HANGUL_TCOUNT;
        if t != HANGUL_TBASE {
            out.push(t);
        }
        return;
    }

    if let Some(parts) = ucd::decomposition(cp, false) {
        for part in parts {
            decompose_into(part, out);
        }
        return;
    }

    out.push(cp);
}

/// Canonical NFD decomposition plus canonical ordering.
fn nfd(input: &[u32]) -> Vec<u32> {
    let mut out = Vec::with_capacity(input.len());
    for &cp in input {
        decompose_into(cp, &mut out);
    }

    // Stable insertion sort by Canonical_Combining_Class; starters (ccc 0)
    // act as barriers.
    for i in 1..out.len() {
        let cp = out[i];
        let cc = ucd::combining_class(cp);
        if cc == 0 {
            continue;
        }
        let mut j = i;
        while j > 0 {
            let prev = ucd::combining_class(out[j - 1]);
            if prev == 0 || prev <= cc {
                break;
            }
            out[j] = out[j - 1];
            j -= 1;
        }
        out[j] = cp;
    }

    out
}

/* ------------------------------------------------------------------------- */
/* Collation elements (S2)                                                    */
/* ------------------------------------------------------------------------- */

#[derive(Clone, Copy, Debug, Default)]
struct Ce {
    primary: u16,
    secondary: u16,
    tertiary: u16,
}

fn push_weights(out: &mut Vec<Ce>, weights: &[ucd::CollationWeight]) {
    for w in weights {
        out.push(Ce {
            primary: w.primary,
            secondary: w.secondary,
            tertiary: w.tertiary,
        });
    }
}

fn block_is(cp: u32, name: &str) -> bool {
    let index = ucd::block(cp) as usize;
    data::BLOCK_NAMES.get(index).copied() == Some(name)
}

/// Produce the CE sequence for one code point with no explicit mapping.
fn ce_from_implicit(out: &mut Vec<Ce>, cp: u32) {
    // Siniform ideographic scripts: base from @implicitweights.
    if let Some((base, offset)) = ucd::implicit_weight(cp) {
        out.push(Ce {
            primary: base,
            secondary: 0x0020,
            tertiary: 0x0002,
        });
        out.push(Ce {
            primary: (offset | 0x8000) as u16,
            secondary: 0,
            tertiary: 0,
        });
        return;
    }

    let unified = ucd::has_property(cp, data::PROP_UNIFIED_IDEOGRAPH);
    let aaa = if unified {
        if block_is(cp, "CJK Unified Ideographs")
            || block_is(cp, "CJK Compatibility Ideographs")
        {
            0xFB40u16 + (cp >> 15) as u16
        } else {
            0xFB80u16 + (cp >> 15) as u16
        }
    } else {
        0xFBC0u16 + (cp >> 15) as u16
    };
    let bbb = ((cp & 0x7FFF) | 0x8000) as u16;
    out.push(Ce {
        primary: aaa,
        secondary: 0x0020,
        tertiary: 0x0002,
    });
    out.push(Ce {
        primary: bbb,
        secondary: 0,
        tertiary: 0,
    });
}

/// Build the collation element array following UTS #10 §7.2 (S2.1-S2.5).
///
/// The scan is driven by a `consumed` map: a code point absorbed into a
/// discontiguous contraction is removed from the string (S2.1.3), while
/// non-starters that could not be absorbed stay in place and are revisited by
/// the main loop (S2.5 proceeds past S only).
fn build_ces(nfd_cps: &[u32]) -> Vec<Ce> {
    let mut out = Vec::new();
    let mut consumed = vec![false; nfd_cps.len()];
    let mut i = 0usize;

    while i < nfd_cps.len() {
        if consumed[i] {
            i += 1;
            continue;
        }

        // Gather the contiguous run of remaining code points starting at i.
        let mut seq = Vec::with_capacity(nfd_cps.len() - i);
        let mut idx = Vec::with_capacity(nfd_cps.len() - i);
        for k in i..nfd_cps.len() {
            if consumed[k] {
                continue;
            }
            idx.push(k);
            seq.push(nfd_cps[k]);
        }

        // S2.1: longest contiguous match.  Contractions take precedence over
        // a single mapping; a single with no table entry falls back to the
        // implicit weights of S2.2.
        let mut weights = Vec::new();
        let (match_len, mut cur_w) =
            match ucd::collation_contraction(&seq, &mut weights) {
                Some((matched, _count)) if matched >= 2 => (matched, weights),
                _ => {
                    let mut single = Vec::new();
                    let n = ucd::collation_weights(seq[0], &mut single);
                    if n > 0 {
                        (1usize, single)
                    } else {
                        (0usize, Vec::new())
                    }
                }
            };

        if match_len == 0 {
            ce_from_implicit(&mut out, nfd_cps[i]);
            consumed[i] = true;
            i += 1;
            continue;
        }

        // S2.1.1-S2.1.3: discontiguous non-starter extension.  Only
        // unblocked non-starters are considered; the lookahead stops at the
        // first starter or the first blocked non-starter.  Absorbed code
        // points are removed; the rest remain for later iterations.
        let mut cur: Vec<u32> = seq[..match_len].to_vec();
        let mut blocking: u8 = 0;
        let after = if match_len < seq.len() {
            idx[match_len]
        } else {
            nfd_cps.len()
        };
        let mut j = after;

        while j < nfd_cps.len() {
            if consumed[j] {
                j += 1;
                continue;
            }
            let cp = nfd_cps[j];
            let ccc = ucd::combining_class(cp);
            if ccc == 0 {
                break; // starter terminates the lookahead
            }
            if ccc <= blocking {
                break; // blocked non-starter terminates it
            }

            cur.push(cp);
            let mut trial_w = Vec::new();
            match ucd::collation_contraction(&cur, &mut trial_w) {
                Some((m, _)) if m == cur.len() => {
                    cur_w = trial_w;
                    consumed[j] = true; // S2.1.3: remove C
                    j += 1;
                    continue; // absorbed C does not block
                }
                _ => {
                    cur.pop();
                    // Not an extension: keep it for a later iteration and let
                    // it contribute to the blocking context.
                    if ccc > blocking {
                        blocking = ccc;
                    }
                    j += 1;
                }
            }
        }

        push_weights(&mut out, &cur_w);
        for &k in &idx[..match_len] {
            consumed[k] = true;
        }
        i = after;
    }

    out
}

/* ------------------------------------------------------------------------- */
/* Sort key formation (S3)                                                    */
/* ------------------------------------------------------------------------- */

fn case_weight(cp: u32) -> u16 {
    match general_category(cp) {
        Category::UppercaseLetter => 2,
        Category::LowercaseLetter => 1,
        _ => 0,
    }
}

/// Build the three-level UCA sort key as a sequence of 16-bit units.
///
/// Each level is terminated by a zero unit.  The optional case level is
/// inserted after the secondary level, matching the extension exposed by
/// [`CollationOptions::case_level`].
pub fn sort_key_units(cps: &[u32], options: &CollationOptions) -> Vec<u16> {
    let nfd_cps = nfd(cps);
    let ces = build_ces(&nfd_cps);
    let strength = options.strength as u8;

    let mut units = Vec::with_capacity(ces.len() * 3 + 3);

    // Level 1.
    for ce in &ces {
        if ce.primary != 0 {
            units.push(ce.primary);
        }
    }
    units.push(0);

    // Level 2.
    if strength >= CollationStrength::Secondary as u8 {
        for ce in &ces {
            if ce.secondary != 0 {
                units.push(ce.secondary);
            }
        }
        units.push(0);
    }

    // Optional dedicated case level.
    if options.case_level && strength >= CollationStrength::Secondary as u8 {
        for &cp in &nfd_cps {
            units.push(case_weight(cp));
        }
        units.push(0);
    }

    // Level 3.
    if strength >= CollationStrength::Tertiary as u8 {
        for ce in &ces {
            if ce.tertiary != 0 {
                units.push(ce.tertiary);
            }
        }
        units.push(0);
    }

    units
}

fn compare_units(a: &[u16], b: &[u16]) -> i32 {
    let n = a.len().min(b.len());
    for i in 0..n {
        if a[i] < b[i] {
            return -1;
        }
        if a[i] > b[i] {
            return 1;
        }
    }
    if a.len() < b.len() {
        -1
    } else if a.len() > b.len() {
        1
    } else {
        0
    }
}

fn uca_compare(a: &[u32], b: &[u32], options: &CollationOptions) -> i32 {
    compare_units(&sort_key_units(a, options), &sort_key_units(b, options))
}

fn is_ascii_digit(cp: u32) -> bool {
    (u32::from(b'0')..=u32::from(b'9')).contains(&cp)
}

fn compare_numeric_run(
    a: &[u32],
    a_start_input: usize,
    b: &[u32],
    b_start_input: usize,
) -> (i32, usize, usize) {
    let mut a_end = a_start_input;
    while a_end < a.len() && is_ascii_digit(a[a_end]) {
        a_end += 1;
    }
    let mut b_end = b_start_input;
    while b_end < b.len() && is_ascii_digit(b[b_end]) {
        b_end += 1;
    }

    let mut a_start = a_start_input;
    while a_start + 1 < a_end && a[a_start] == u32::from(b'0') {
        a_start += 1;
    }
    let mut b_start = b_start_input;
    while b_start + 1 < b_end && b[b_start] == u32::from(b'0') {
        b_start += 1;
    }

    let a_digits = a_end - a_start;
    let b_digits = b_end - b_start;

    let mut result = 0;
    if a_digits != b_digits {
        result = if a_digits < b_digits { -1 } else { 1 };
    } else {
        for i in 0..a_digits {
            if a[a_start + i] != b[b_start + i] {
                result = if a[a_start + i] < b[b_start + i] {
                    -1
                } else {
                    1
                };
                break;
            }
        }
    }

    (result, a_end, b_end)
}

fn next_non_digit_run_end(cps: &[u32], start: usize) -> usize {
    let mut end = start;
    while end < cps.len() && !is_ascii_digit(cps[end]) {
        end += 1;
    }
    end
}

fn numeric_compare(a: &[u32], b: &[u32], options: &CollationOptions) -> i32 {
    let mut ai = 0usize;
    let mut bi = 0usize;

    while ai < a.len() && bi < b.len() {
        let a_digit = is_ascii_digit(a[ai]);
        let b_digit = is_ascii_digit(b[bi]);
        if a_digit && b_digit {
            let (result, next_a, next_b) = compare_numeric_run(a, ai, b, bi);
            ai = next_a;
            bi = next_b;
            if result != 0 {
                return result;
            }
        } else {
            let next_a = next_non_digit_run_end(a, ai);
            let next_b = next_non_digit_run_end(b, bi);
            let result = uca_compare(&a[ai..next_a], &b[bi..next_b], options);
            ai = next_a;
            bi = next_b;
            if result != 0 {
                return result;
            }
        }
    }

    if ai < a.len() {
        1
    } else if bi < b.len() {
        -1
    } else {
        0
    }
}

/// Compare two scalar sequences with the configured collation options.
///
/// This is the primitive used by the conformance tests and by [`collate`].
pub fn compare_scalars(a: &[u32], b: &[u32], options: &CollationOptions) -> i32 {
    if options.numeric_ordering {
        numeric_compare(a, b, options)
    } else {
        uca_compare(a, b, options)
    }
}

/* ------------------------------------------------------------------------- */
/* TextString-facing API                                                      */
/* ------------------------------------------------------------------------- */

fn decode_string(source: &TextString<'_>) -> Result<Vec<u32>, Status> {
    let system = source.system();
    if !system.has_unicode_profile() {
        return Err(Status::Unsupported);
    }
    let profile = system.unicode_profile().ok_or(Status::Unsupported)?;

    let mut codec = Codec::new(system);
    let mut scalars = Vec::new();
    let mut offset = 0usize;

    while offset < source.len() {
        match codec.decode_char(&source.bytes()[offset..])? {
            DecodeOutcome::Char { ch, read } => {
                if read == 0 {
                    return Err(Status::Internal);
                }
                let cp = profile.try_to_unicode(ch).ok_or(Status::UnmappableChar)?;
                scalars.push(cp);
                offset += read;
            }
            DecodeOutcome::NoCharacter { read } => {
                if read == 0 {
                    return Err(Status::Internal);
                }
                offset += read;
            }
        }
    }

    Ok(scalars)
}

/// Compare two strings.
///
/// Returns a negative value when `a` sorts before `b`, zero when they compare
/// equal at the requested strength, and a positive value otherwise.  Errors are
/// reported as zero; use [`try_collate`] when the distinction matters.
pub fn collate(a: &TextString<'_>, b: &TextString<'_>, options: &CollationOptions) -> i32 {
    try_collate(a, b, options).unwrap_or(0)
}

/// Compare two strings, reporting unsupported systems and malformed input.
pub fn try_collate(
    a: &TextString<'_>,
    b: &TextString<'_>,
    options: &CollationOptions,
) -> Result<i32, Status> {
    let ac = decode_string(a)?;
    let bc = decode_string(b)?;
    Ok(compare_scalars(&ac, &bc, options))
}

/// Build a deterministic sort key.
///
/// The key serializes the UCA levels as big-endian 16-bit units.  For
/// [`CollationStrength::Quaternary`] and above the original encoded bytes are
/// appended as a final level, preserving the previous byte-level tie-break.
pub fn sort_key(source: &TextString<'_>, options: &CollationOptions) -> Result<Vec<u8>, Status> {
    let scalars = decode_string(source)?;
    let units = sort_key_units(&scalars, options);

    let mut key = Vec::with_capacity(units.len() * 2 + source.len() + 2);
    for unit in units {
        key.push((unit >> 8) as u8);
        key.push(unit as u8);
    }

    if options.strength >= CollationStrength::Quaternary {
        key.extend_from_slice(source.bytes());
        key.push(0);
        key.push(0);
    }

    Ok(key)
}

/// Build a sort key into a caller-provided buffer.
pub fn sort_key_into(
    source: &TextString<'_>,
    options: &CollationOptions,
    destination: &mut [u8],
) -> Result<usize, Status> {
    let key = sort_key(source, options)?;
    if key.len() > destination.len() {
        return Err(Status::BufferTooSmall);
    }
    destination[..key.len()].copy_from_slice(&key);
    Ok(key.len())
}
