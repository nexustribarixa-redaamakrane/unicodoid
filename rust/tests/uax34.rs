//! Conformance test for UAX #34 Unicode Named Character Sequences.
//!
//! Every entry of the official Unicode 18.0.0 `NamedSequences.txt` is looked
//! up by name and by sequence membership.

use std::fs;
use std::path::Path;

use unicodoid::uax34;

fn data_file() -> std::path::PathBuf {
    Path::new(env!("CARGO_MANIFEST_DIR")).join("../data/ucd/18.0.0/ucd/NamedSequences.txt")
}

fn parse_scalars(field: &str) -> Vec<u32> {
    field
        .split_whitespace()
        .filter_map(|token| u32::from_str_radix(token, 16).ok())
        .collect()
}

#[test]
fn named_sequences_conformance() {
    let path = data_file();
    let text = fs::read_to_string(&path)
        .unwrap_or_else(|e| panic!("cannot read {}: {e}", path.display()));

    let mut lines = 0usize;
    let mut failures = 0usize;

    for (lineno, raw) in text.lines().enumerate() {
        let line = raw.trim();
        if line.is_empty() || line.starts_with('#') || line.starts_with('@') {
            continue;
        }
        let Some((name, sequence)) = line.split_once(';') else {
            continue;
        };
        let name = name.trim();
        let expected = parse_scalars(sequence);

        let found = uax34::find(name);
        if found != Some(expected.as_slice()) {
            failures += 1;
            if failures <= 10 {
                eprintln!("line {}: lookup mismatch for {name:?}", lineno + 1);
            }
        }
        if !uax34::is_registered(&expected) {
            failures += 1;
            if failures <= 10 {
                eprintln!("line {}: sequence not registered for {name:?}", lineno + 1);
            }
        }
        lines += 1;
    }

    assert!(lines > 400, "expected a populated NamedSequences.txt");
    assert_eq!(
        uax34::count(),
        lines,
        "registered sequence count must match the data file"
    );
    assert_eq!(failures, 0, "UAX #34 conformance failures");
    eprintln!("UAX34: {lines} sequences validated");
}

#[test]
fn rejects_ill_formed_and_unknown() {
    assert!(!uax34::is_registered(&[]));
    assert!(!uax34::is_registered(&[0xD800, 0x0020]));
    assert!(!uax34::is_registered(&[0x110000]));
    assert!(uax34::find("NOT A REAL SEQUENCE NAME").is_none());
    assert!(uax34::sequence(uax34::count()).is_none());
}
