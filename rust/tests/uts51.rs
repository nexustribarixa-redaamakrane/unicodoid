//! Conformance test for UTS #51 RGI emoji sequence data.
//!
//! Every entry of the official Unicode 18.0.0 `emoji-sequences.txt` and
//! `emoji-zwj-sequences.txt` must be recognised by the validator, including
//! the `Basic_Emoji` ranges which are expanded to individual code points.

use std::fs;
use std::path::{Path, PathBuf};

use unicodoid::uts51;

fn emoji_dir() -> PathBuf {
    Path::new(env!("CARGO_MANIFEST_DIR")).join("../data/ucd/18.0.0/emoji")
}

fn parse_scalars(field: &str) -> Vec<u32> {
    field
        .split_whitespace()
        .filter_map(|token| u32::from_str_radix(token, 16).ok())
        .collect()
}

/// Validate one sequence and return how many sequences it contributed.
fn validate(cps: &[u32], expected_type: &str, failures: &mut usize, checked: &mut usize) -> usize {
    let actual = uts51::is_rgi(cps);
    *checked += 1;
    if actual != Some(expected_type) {
        *failures += 1;
        if *failures <= 10 {
            eprintln!(
                "RGI mismatch for {cps:X?}: expected {expected_type}, got {actual:?}"
            );
        }
    }
    *checked += 1;
    if !uts51::is_well_formed(cps) {
        *failures += 1;
        if *failures <= 10 {
            eprintln!("RGI sequence not well formed: {cps:X?} ({expected_type})");
        }
    }
    1
}

fn validate_file(path: &Path, failures: &mut usize, checked: &mut usize) -> usize {
    let text = fs::read_to_string(path)
        .unwrap_or_else(|e| panic!("cannot read {}: {e}", path.display()));
    let mut sequences = 0usize;

    for raw in text.lines() {
        let line = raw.trim();
        if line.is_empty() || line.starts_with('#') || line.starts_with('@') {
            continue;
        }
        let Some((fields, _rest)) = line.split_once(';') else {
            continue;
        };
        let Some((_code_points, type_field)) = line.split_once(';') else {
            continue;
        };
        let type_field = type_field.split(';').next().unwrap_or("").trim();
        if type_field.is_empty() {
            continue;
        }

        let field = fields.trim();
        if let Some((first, last)) = field.split_once("..") {
            let Ok(first) = u32::from_str_radix(first.trim(), 16) else {
                continue;
            };
            let Ok(last) = u32::from_str_radix(last.trim(), 16) else {
                continue;
            };
            for cp in first..=last {
                sequences += validate(&[cp], type_field, failures, checked);
            }
            continue;
        }

        let cps = parse_scalars(field);
        if cps.is_empty() {
            continue;
        }
        sequences += validate(&cps, type_field, failures, checked);
    }

    sequences
}

#[test]
fn rgi_emoji_conformance() {
    let dir = emoji_dir();
    let mut failures = 0usize;
    let mut checked = 0usize;

    let a = validate_file(&dir.join("emoji-sequences.txt"), &mut failures, &mut checked);
    let b = validate_file(
        &dir.join("emoji-zwj-sequences.txt"),
        &mut failures,
        &mut checked,
    );

    assert_eq!(
        uts51::count(),
        a + b,
        "generated RGI sequence count must match the data files"
    );
    assert_eq!(failures, 0, "UTS #51 conformance failures");
    eprintln!("UTS51: {} RGI sequences validated", a + b);
}

#[test]
fn structural_checks() {
    // Keycap sequence: '1' FE0F 20E3.
    let keycap = [0x31, 0xFE0F, 0x20E3];
    assert_eq!(uts51::is_rgi(&keycap), Some("Emoji_Keycap_Sequence"));
    assert!(uts51::is_well_formed(&keycap));

    // Flag sequence: two regional indicators.
    let flag = [0x1F1EB, 0x1F1F7];
    assert_eq!(uts51::is_rgi(&flag), Some("RGI_Emoji_Flag_Sequence"));

    // Family ZWJ sequence.
    let family = [0x1F468, 0x200D, 0x1F469, 0x200D, 0x1F467];
    assert_eq!(uts51::is_rgi(&family), Some("RGI_Emoji_ZWJ_Sequence"));

    // Negative cases.
    let bogus = [0x41, 0x200D, 0x42];
    assert!(uts51::is_rgi(&bogus).is_none());
    assert!(!uts51::is_well_formed(&bogus));
    assert!(!uts51::is_well_formed(&[0xD83D, 0xDE00]));
    assert!(!uts51::is_well_formed(&[]));
    assert!(uts51::is_rgi(&[]).is_none());
}
