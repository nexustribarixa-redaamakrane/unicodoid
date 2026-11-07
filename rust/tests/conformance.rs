//! Official Unicode 18.0.0 conformance tests for text segmentation.
//!
//! The test vectors are read directly from the UCD distribution in
//! `data/ucd/18.0.0/ucd/auxiliary/`; nothing is copied or curated.  Each case
//! is decoded from its scalar sequence and segmented through the public
//! [`Segmenter`] API.

use std::fs;
use std::path::{Path, PathBuf};

use unicodoid::{BoundaryType, Segmenter, utf8_system};

fn auxiliary_dir() -> PathBuf {
    Path::new(env!("CARGO_MANIFEST_DIR"))
        .join("../data/ucd/18.0.0/ucd/auxiliary")
}

/// Parse a `*BreakTest.txt` file into (code points, expected boundary indices).
///
/// The file format interleaves a boundary marker (`÷` or `×`) with each code
/// point: `÷ 0020 × 0308 ÷ 0020 ÷`.  A `÷` at position *k* means a boundary
/// before scalar *k*; the leading and trailing markers are always `÷`.
fn parse_break_test(path: &Path) -> Vec<(Vec<u32>, Vec<usize>)> {
    let text = fs::read_to_string(path)
        .unwrap_or_else(|e| panic!("cannot read {}: {e}", path.display()));

    let mut cases = Vec::new();
    for (lineno, raw) in text.lines().enumerate() {
        let data = raw.split('#').next().unwrap_or("").trim();
        if data.is_empty() {
            continue;
        }

        let tokens: Vec<&str> = data.split_whitespace().collect();
        let mut cps = Vec::new();
        let mut boundaries = Vec::new();
        let mut pos = 0usize;

        let mut idx = 0usize;
        // Leading boundary marker.
        if tokens.first() == Some(&"÷") {
            boundaries.push(0);
        }
        if tokens.first() == Some(&"÷") || tokens.first() == Some(&"×") {
            idx = 1;
        }

        while idx < tokens.len() {
            let cp = u32::from_str_radix(tokens[idx], 16).unwrap_or_else(|_| {
                panic!(
                    "{}:{}: invalid code point {:?}",
                    path.display(),
                    lineno + 1,
                    tokens[idx]
                )
            });
            cps.push(cp);
            pos += 1;
            idx += 1;

            if idx < tokens.len() {
                if tokens[idx] == "÷" {
                    boundaries.push(pos);
                }
                idx += 1;
            }
        }

        // Every boundary type has a start-of-text boundary.  The line break
        // test file marks it with `×` and omits it from the recorded breaks.
        if boundaries.first() != Some(&0) {
            boundaries.insert(0, 0);
        }

        cases.push((cps, boundaries));
    }
    cases
}

/// Segment a scalar sequence and return the cumulative boundary positions.
fn actual_boundaries(cps: &[u32], boundary: BoundaryType) -> Vec<usize> {
    let text: String = cps
        .iter()
        .map(|&cp| char::from_u32(cp).expect("test vector contains a non-scalar"))
        .collect();

    let mut segmenter = Segmenter::new(utf8_system(), text.as_bytes(), boundary)
        .expect("UTF-8 system must expose a Unicode profile");

    let mut boundaries = vec![0usize];
    while let Some(segment) = segmenter.next_segment() {
        let last = *boundaries.last().unwrap();
        boundaries.push(last + segment.char_count);
    }
    boundaries
}

fn run_suite(file: &str, boundary: BoundaryType) {
    let path = auxiliary_dir().join(file);
    let cases = parse_break_test(&path);
    assert!(!cases.is_empty(), "{} contained no test cases", file);

    let mut failures = 0usize;
    for (index, (cps, expected)) in cases.iter().enumerate() {
        let actual = actual_boundaries(cps, boundary);
        if &actual != expected {
            failures += 1;
            if failures <= 10 {
                eprintln!(
                    "{file} case {index}: cps={cps:04X?}\n  expected={expected:?}\n  actual  ={actual:?}"
                );
            }
        }
    }
    assert_eq!(failures, 0, "{file}: {failures} of {} cases failed", cases.len());
}

#[test]
fn grapheme_conformance() {
    run_suite("GraphemeBreakTest.txt", BoundaryType::Grapheme);
}

#[test]
fn word_conformance() {
    run_suite("WordBreakTest.txt", BoundaryType::Word);
}

#[test]
fn line_conformance() {
    run_suite("LineBreakTest.txt", BoundaryType::Line);
}

#[test]
fn sentence_conformance() {
    run_suite("SentenceBreakTest.txt", BoundaryType::Sentence);
}
