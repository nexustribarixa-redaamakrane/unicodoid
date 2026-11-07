//! UAX #15 normalization for Unicode-compatible systems.
//!
//! The module is opt-in: the system must expose a Unicode profile.  It
//! implements the full canonical and compatibility decomposition tables,
//! canonical pair composition, Hangul decomposition/composition and canonical
//! ordering of combining marks, all taken from the generated Unicode 18.0.0
//! tables (see `ucd_data`).  It is exercised against the official
//! `NormalizationTest.txt` in `tests/normalization.rs`.

use crate::codec::Codec;
use crate::system::EncodingSystem;
use crate::text_string::TextString;
use crate::types::{DecodeOutcome, Status};

/// Normalization forms from UAX #15.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
#[repr(u8)]
pub enum NormalizationForm {
    Nfc = 0,
    Nfd = 1,
    Nfkc = 2,
    Nfkd = 3,
    /// Reserved for system-defined forms; this implementation reports
    /// [`Status::Unsupported`].
    Custom = 4,
}

const HANGUL_SBASE: u32 = 0xAC00;
const HANGUL_LBASE: u32 = 0x1100;
const HANGUL_VBASE: u32 = 0x1161;
const HANGUL_TBASE: u32 = 0x11A7;
const HANGUL_LCOUNT: u32 = 19;
const HANGUL_VCOUNT: u32 = 21;
const HANGUL_TCOUNT: u32 = 28;
const HANGUL_NCOUNT: u32 = HANGUL_VCOUNT * HANGUL_TCOUNT;
const HANGUL_SCOUNT: u32 = HANGUL_LCOUNT * HANGUL_NCOUNT;

fn decompose_scalar(cp: u32, compatibility: bool, out: &mut Vec<u32>) {
    if (HANGUL_SBASE..HANGUL_SBASE + HANGUL_SCOUNT).contains(&cp) {
        let sindex = cp - HANGUL_SBASE;
        let lead = HANGUL_LBASE + sindex / HANGUL_NCOUNT;
        let vowel = HANGUL_VBASE + (sindex % HANGUL_NCOUNT) / HANGUL_TCOUNT;
        let trail = HANGUL_TBASE + sindex % HANGUL_TCOUNT;
        out.push(lead);
        out.push(vowel);
        if trail != HANGUL_TBASE {
            out.push(trail);
        }
        return;
    }

    if let Some(parts) = crate::ucd::decomposition(cp, compatibility) {
        for part in parts {
            decompose_scalar(part, compatibility, out);
        }
        return;
    }

    out.push(cp);
}

fn compose_pair(starter: u32, ch: u32) -> Option<u32> {
    // Hangul LV composition.
    if (HANGUL_LBASE..HANGUL_LBASE + HANGUL_LCOUNT).contains(&starter)
        && (HANGUL_VBASE..HANGUL_VBASE + HANGUL_VCOUNT).contains(&ch)
    {
        let lindex = starter - HANGUL_LBASE;
        let vindex = ch - HANGUL_VBASE;
        return Some(HANGUL_SBASE + (lindex * HANGUL_VCOUNT + vindex) * HANGUL_TCOUNT);
    }

    // Hangul LVT composition.
    if (HANGUL_SBASE..HANGUL_SBASE + HANGUL_SCOUNT).contains(&starter)
        && (starter - HANGUL_SBASE) % HANGUL_TCOUNT == 0
        && ch > HANGUL_TBASE
        && ch < HANGUL_TBASE + HANGUL_TCOUNT
    {
        return Some(starter + (ch - HANGUL_TBASE));
    }

    // Canonical pair composition from the generated table.
    crate::ucd::compose_pair(starter, ch)
}

fn canonical_order(sequence: &mut [u32]) {
    for i in 1..sequence.len() {
        let cp = sequence[i];
        let class = crate::ucd::combining_class(cp);
        if class == 0 {
            continue;
        }

        let mut j = i;
        while j > 0 {
            let previous = crate::ucd::combining_class(sequence[j - 1]);
            if previous == 0 || previous <= class {
                break;
            }
            sequence[j] = sequence[j - 1];
            j -= 1;
        }
        sequence[j] = cp;
    }
}

fn compose_scalars(sequence: &[u32]) -> Vec<u32> {
    if sequence.is_empty() {
        return Vec::new();
    }

    let mut out = Vec::with_capacity(sequence.len());
    out.push(sequence[0]);

    let mut starter_pos = 0usize;
    let mut starter = sequence[0];
    let mut last_class = 0u8;

    for &cp in &sequence[1..] {
        let class = crate::ucd::combining_class(cp);

        if last_class < class || last_class == 0 {
            if let Some(composed) = compose_pair(starter, cp) {
                out[starter_pos] = composed;
                starter = composed;
                continue;
            }
        }

        if class == 0 {
            starter_pos = out.len();
            starter = cp;
        }
        out.push(cp);
        last_class = class;
    }

    out
}

fn decode_scalars(system: &dyn EncodingSystem, input: &[u8]) -> Result<Vec<u32>, Status> {
    let profile = system.unicode_profile().ok_or(Status::Unsupported)?;
    let mut codec = Codec::new(system);
    let mut scalars = Vec::new();
    let mut offset = 0usize;

    while offset < input.len() {
        match codec.decode_char(&input[offset..])? {
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

fn encode_scalars(system: &dyn EncodingSystem, scalars: &[u32]) -> Result<Vec<u8>, Status> {
    let profile = system.unicode_profile().ok_or(Status::Unsupported)?;
    let mut codec = Codec::new(system);
    let mut out = Vec::new();
    let mut buffer = vec![0u8; system.max_bytes_per_char().max(8)];

    for &cp in scalars {
        let native = profile.try_from_unicode(cp).ok_or(Status::UnmappableChar)?;
        let written = codec.encode_char(native, &mut buffer)?;
        out.extend_from_slice(&buffer[..written]);
    }

    Ok(out)
}

/// Normalize a byte buffer and return newly allocated bytes.
pub fn normalize(
    system: &dyn EncodingSystem,
    input: &[u8],
    form: NormalizationForm,
) -> Result<Vec<u8>, Status> {
    if form == NormalizationForm::Custom {
        return Err(Status::Unsupported);
    }
    if !system.has_unicode_profile() {
        return Err(Status::Unsupported);
    }
    if input.is_empty() {
        return Ok(Vec::new());
    }

    let compatibility = matches!(form, NormalizationForm::Nfkc | NormalizationForm::Nfkd);
    let compose = matches!(form, NormalizationForm::Nfc | NormalizationForm::Nfkc);

    let decoded = decode_scalars(system, input)?;
    let mut decomposed = Vec::with_capacity(decoded.len());
    for cp in decoded {
        decompose_scalar(cp, compatibility, &mut decomposed);
    }
    canonical_order(&mut decomposed);

    let normalized = if compose {
        compose_scalars(&decomposed)
    } else {
        decomposed
    };

    encode_scalars(system, &normalized)
}

/// Normalize into a caller-provided buffer.
///
/// Returns [`Status::BufferTooSmall`] when the destination cannot hold the
/// result.  Use [`normalize`] when an owned result is convenient.
pub fn normalize_into(
    system: &dyn EncodingSystem,
    input: &[u8],
    form: NormalizationForm,
    destination: &mut [u8],
) -> Result<usize, Status> {
    let bytes = normalize(system, input, form)?;
    if bytes.len() > destination.len() {
        return Err(Status::BufferTooSmall);
    }
    destination[..bytes.len()].copy_from_slice(&bytes);
    Ok(bytes.len())
}

/// Normalize a [`TextString`] using the same encoding system.
pub fn normalize_string<'a>(
    source: &TextString<'a>,
    form: NormalizationForm,
) -> Result<TextString<'a>, Status> {
    let system = source.system();
    let bytes = normalize(system, source.bytes(), form)?;
    TextString::from_bytes(system, &bytes)
}

/// Check whether a byte buffer is already in the requested normalization form.
///
/// An empty buffer is trivially normalized, matching the C and C++ SDKs.
pub fn is_normalized(system: &dyn EncodingSystem, input: &[u8], form: NormalizationForm) -> bool {
    if input.is_empty() {
        return true;
    }
    match normalize(system, input, form) {
        Ok(bytes) => bytes == input,
        Err(_) => false,
    }
}
