//! UAX #15 Unicode Normalization Forms (Unicode 18.0.0, Revision 58).
//!
//! Scalar API delegating to the byte normalizer so both paths agree.

use crate::normalization::{self, NormalizationForm};
use crate::systems;

/// True when `cp` has a decomposition of the requested kind.
pub fn has_decomposition(cp: u32, compatibility: bool) -> bool {
    crate::ucd::has_decomposition(cp, compatibility)
}

/// Normalize a scalar sequence.
pub fn normalize(cps: &[u32], form: NormalizationForm) -> Vec<u32> {
    let system = systems::utf8::system();
    let profile = system.unicode_profile().expect("utf8 profile");
    let bytes: Vec<u8> = cps
        .iter()
        .filter_map(|&cp| char::from_u32(cp))
        .flat_map(|ch| {
            let mut buf = [0u8; 4];
            ch.encode_utf8(&mut buf).as_bytes().to_vec()
        })
        .collect();
    let out = normalization::normalize(system, &bytes, form)
        .unwrap_or_default();
    let mut scalars = Vec::new();
    let mut i = 0;
    while i < out.len() {
        match std::str::from_utf8(&out[i..]) {
            Ok(s) => {
                for ch in s.chars() {
                    scalars.push(ch as u32);
                }
                break;
            }
            Err(e) => {
                let valid = e.valid_up_to();
                if valid > 0 {
                    for ch in std::str::from_utf8(&out[i..i + valid])
                        .unwrap()
                        .chars()
                    {
                        scalars.push(ch as u32);
                    }
                    i += valid;
                } else {
                    // Skip invalid byte via lossy decode to stay total.
                    let s = String::from_utf8_lossy(&out[i..i + 1]);
                    for ch in s.chars() {
                        scalars.push(ch as u32);
                    }
                    i += 1;
                }
            }
        }
    }
    let _ = profile;
    scalars
}

/// True when `cps` is already in the requested normal form.
pub fn is_normalized(cps: &[u32], form: NormalizationForm) -> bool {
    let system = systems::utf8::system();
    let bytes: Vec<u8> = cps
        .iter()
        .filter_map(|&cp| char::from_u32(cp))
        .flat_map(|ch| {
            let mut buf = [0u8; 4];
            ch.encode_utf8(&mut buf).as_bytes().to_vec()
        })
        .collect();
    normalization::is_normalized(system, &bytes, form)
}
