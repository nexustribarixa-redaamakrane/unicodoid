//! Official Unicode 18.0.0 conformance test for UTS #10.
//!
//! The vectors are read directly from the UCA distribution in
//! `data/ucd/18.0.0/uca/test/CollationTest/`.  Each line of
//! `CollationTest_NON_IGNORABLE_SHORT.txt` is expected to sort after (or equal
//! to) the previous line; the official file is non-decreasing.

use std::fs;
use std::path::{Path, PathBuf};

use unicodoid::{CollationOptions, compare_scalars};

fn test_file() -> PathBuf {
    Path::new(env!("CARGO_MANIFEST_DIR")).join(
        "../data/ucd/18.0.0/uca/test/CollationTest/CollationTest_NON_IGNORABLE_SHORT.txt",
    )
}

fn parse_scalars(line: &str) -> Vec<u32> {
    let field = line.split(';').next().unwrap_or("");
    field
        .split_whitespace()
        .filter_map(|token| u32::from_str_radix(token, 16).ok())
        .collect()
}

fn is_ill_formed(cps: &[u32]) -> bool {
    cps.iter().any(|&cp| (0xD800..=0xDFFF).contains(&cp))
}

#[test]
fn non_ignorable_conformance() {
    let path = test_file();
    let text = fs::read_to_string(&path)
        .unwrap_or_else(|e| panic!("cannot read {}: {e}", path.display()));

    let options = CollationOptions::default();
    let mut previous: Option<Vec<u32>> = None;
    let mut cases = 0usize;
    let mut skipped = 0usize;
    let mut failures = 0usize;

    for (lineno, raw) in text.lines().enumerate() {
        let line = raw.trim();
        if line.is_empty() || line.starts_with('#') || line.starts_with('@') {
            continue;
        }

        let cps = parse_scalars(line);
        if cps.is_empty() {
            continue;
        }
        if is_ill_formed(&cps) {
            skipped += 1;
            continue;
        }

        if let Some(prev) = &previous {
            let cmp = compare_scalars(prev, &cps, &options);
            if cmp > 0 {
                failures += 1;
                if failures <= 10 {
                    eprintln!("line {}: decreasing order", lineno + 1);
                }
            }
        }

        previous = Some(cps);
        cases += 1;
    }

    eprintln!(
        "CollationTest_NON_IGNORABLE_SHORT.txt: {cases} lines, {skipped} skipped (surrogates), {failures} failures"
    );
    assert_eq!(failures, 0, "UCA conformance failures");
    assert!(cases > 200_000, "expected the full SHORT test file");
}
