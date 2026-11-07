//! Conformance and algorithm tests for UTR #59 East Asian Spacing.
//!
//! The property is validated against the official derived data file
//! `data/ucd/18.0.0/tr59/east-asian-spacing.txt` for every listed code point.
//! The spacing algorithm is exercised on the cases described in UTR59 §3.3.

use std::fs;
use std::path::Path;

use unicodoid::utr59::{self, eas};

fn data_file() -> std::path::PathBuf {
    Path::new(env!("CARGO_MANIFEST_DIR"))
        .join("../data/ucd/18.0.0/tr59/east-asian-spacing.txt")
}

fn parse_range(field: &str) -> Option<(u32, u32)> {
    let field = field.trim();
    if let Some((first, last)) = field.split_once("..") {
        Some((
            u32::from_str_radix(first.trim(), 16).ok()?,
            u32::from_str_radix(last.trim(), 16).ok()?,
        ))
    } else {
        let cp = u32::from_str_radix(field, 16).ok()?;
        Some((cp, cp))
    }
}

#[test]
fn property_matches_official_data() {
    let path = data_file();
    let text = fs::read_to_string(&path)
        .unwrap_or_else(|e| panic!("cannot read {}: {e}", path.display()));

    let mut entries = 0usize;
    let mut failures = 0usize;

    for (lineno, raw) in text.lines().enumerate() {
        let line = raw.trim();
        if line.is_empty() || line.starts_with('#') || line.starts_with('@') {
            continue;
        }
        let Some((range_field, value_field)) = line.split_once(';') else {
            continue;
        };
        let expected = match value_field.trim().chars().next() {
            Some('W') => eas::W,
            Some('N') => eas::N,
            Some('O') => eas::O,
            Some('C') => eas::C,
            _ => continue,
        };
        let Some((first, last)) = parse_range(range_field) else {
            continue;
        };

        for cp in first..=last {
            if utr59::east_asian_spacing(cp) != expected {
                failures += 1;
                if failures <= 10 {
                    eprintln!(
                        "line {}: U+{cp:04X} expected {expected} got {}",
                        lineno + 1,
                        utr59::east_asian_spacing(cp)
                    );
                }
            }
        }
        entries += 1;
    }

    assert!(entries > 1000, "expected a populated east-asian-spacing.txt");
    assert_eq!(failures, 0, "UTR #59 property conformance failures");
    eprintln!("UTR59 data file: {entries} ranges validated");
}

#[test]
fn spacing_algorithm() {
    let offsets = |s: &[u32], chinese: bool, vertical: bool| {
        utr59::spacing_offsets(s, chinese, vertical)
    };

    // Wide / Narrow adjacency in both directions.
    assert_eq!(offsets(&[0x41, 0x6F22], false, false), vec![1]);
    assert_eq!(offsets(&[0x6F22, 0x41], false, false), vec![1]);
    assert!(offsets(&[0x6F22, 0x3002], false, false).is_empty());
    assert_eq!(offsets(&[0x6F22, 0x31], false, false), vec![1]);

    // Conditional value depends on the language context.
    assert!(offsets(&[0x6F22, 0x25], false, false).is_empty());
    assert_eq!(offsets(&[0x6F22, 0x25], true, false), vec![1]);

    // Combining marks stay in the preceding cluster.
    assert_eq!(offsets(&[0x41, 0x0301, 0x6F22], false, false), vec![2]);

    // Enclosing mark (Me) forces the cluster to Other.
    assert!(offsets(&[0x6F22, 0x20DD, 0x41], false, false).is_empty());

    // Multiple insertions.
    assert_eq!(
        offsets(&[0x6F22, 0x41, 0x6F22], false, false),
        vec![1, 2]
    );

    // Empty input.
    assert!(offsets(&[], false, false).is_empty());
}

#[test]
fn cluster_values_and_defaults() {
    assert_eq!(utr59::east_asian_spacing(0x0020), eas::O);
    assert_eq!(utr59::east_asian_spacing(0x4E00), eas::W);
    assert_eq!(utr59::east_asian_spacing(0x0041), eas::N);
    assert_eq!(utr59::east_asian_spacing(0x0025), eas::C);
    assert_eq!(utr59::east_asian_spacing(0x3002), eas::O);

    assert_eq!(utr59::cluster_value(&[0x41], false, false), eas::N);
    assert_eq!(utr59::cluster_value(&[0x41], false, true), eas::N);
    assert_eq!(utr59::cluster_value(&[0x6F22], false, false), eas::W);
    assert_eq!(utr59::cluster_value(&[0x0025], false, false), eas::O);
    assert_eq!(utr59::cluster_value(&[0x0025], true, false), eas::N);
    assert_eq!(utr59::cluster_value(&[], false, false), eas::O);
}
