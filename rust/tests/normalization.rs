//! Official Unicode 18.0.0 normalization conformance test (UAX #15).
//!
//! The vectors are read directly from the UCD distribution in
//! `data/ucd/18.0.0/ucd/`; nothing is copied or curated.  The first five
//! columns of each `NormalizationTest.txt` line are the source, NFC, NFD,
//! NFKC and NFKD scalar sequences.

use std::fs;
use std::path::{Path, PathBuf};

use unicodoid::{NormalizationForm, normalize, utf8_system};

fn ucd_dir() -> PathBuf {
    Path::new(env!("CARGO_MANIFEST_DIR")).join("../data/ucd/18.0.0/ucd")
}

fn parse_cps(field: &str) -> Vec<u32> {
    field
        .split_whitespace()
        .map(|token| u32::from_str_radix(token, 16).expect("invalid code point in test vector"))
        .collect()
}

fn utf8_encode(cps: &[u32]) -> Vec<u8> {
    let mut out = Vec::with_capacity(cps.len() * 4);
    for &cp in cps {
        match cp {
            0x0000..=0x007F => out.push(cp as u8),
            0x0080..=0x07FF => {
                out.push(0xC0 | (cp >> 6) as u8);
                out.push(0x80 | (cp & 0x3F) as u8);
            }
            0x0800..=0xFFFF => {
                out.push(0xE0 | (cp >> 12) as u8);
                out.push(0x80 | ((cp >> 6) & 0x3F) as u8);
                out.push(0x80 | (cp & 0x3F) as u8);
            }
            _ => {
                out.push(0xF0 | (cp >> 18) as u8);
                out.push(0x80 | ((cp >> 12) & 0x3F) as u8);
                out.push(0x80 | ((cp >> 6) & 0x3F) as u8);
                out.push(0x80 | (cp & 0x3F) as u8);
            }
        }
    }
    out
}

fn normalize_cps(cps: &[u32], form: NormalizationForm) -> Vec<u32> {
    let input = utf8_encode(cps);
    let system = utf8_system();
    normalize(system, &input, form)
        .map(|bytes| {
            let mut out = Vec::with_capacity(bytes.len());
            let mut index = 0usize;
            while index < bytes.len() {
                let b0 = bytes[index];
                let cp = if b0 < 0x80 {
                    bytes[index] as u32
                } else if b0 < 0xE0 {
                    ((b0 as u32 & 0x1F) << 6) | (bytes[index + 1] as u32 & 0x3F)
                } else if b0 < 0xF0 {
                    ((b0 as u32 & 0x0F) << 12)
                        | ((bytes[index + 1] as u32 & 0x3F) << 6)
                        | (bytes[index + 2] as u32 & 0x3F)
                } else {
                    ((b0 as u32 & 0x07) << 18)
                        | ((bytes[index + 1] as u32 & 0x3F) << 12)
                        | ((bytes[index + 2] as u32 & 0x3F) << 6)
                        | (bytes[index + 3] as u32 & 0x3F)
                };
                let advance = if b0 < 0x80 {
                    1
                } else if b0 < 0xE0 {
                    2
                } else if b0 < 0xF0 {
                    3
                } else {
                    4
                };
                index += advance;
                out.push(cp);
            }
            out
        })
        .expect("normalization must succeed on well-formed input")
}

#[test]
fn normalization_conformance() {
    let path = ucd_dir().join("NormalizationTest.txt");
    let text = fs::read_to_string(&path)
        .unwrap_or_else(|e| panic!("cannot read {}: {e}", path.display()));

    let mut checks = 0usize;
    let mut failures = 0usize;
    let mut cases = 0usize;

    for raw in text.lines() {
        let line = raw.split('#').next().unwrap_or("").trim();
        if line.is_empty() || line.starts_with('@') {
            continue;
        }
        let fields: Vec<&str> = line.split(';').map(str::trim).collect();
        if fields.len() < 5 {
            continue;
        }
        let source = parse_cps(fields[0]);
        let nfc = parse_cps(fields[1]);
        let nfd = parse_cps(fields[2]);
        let nfkc = parse_cps(fields[3]);
        let nfkd = parse_cps(fields[4]);
        if source.is_empty() {
            continue;
        }
        cases += 1;

        let cases_inputs: &[(NormalizationForm, &Vec<u32>, &Vec<u32>)] = &[
            (NormalizationForm::Nfc, &source, &nfc),
            (NormalizationForm::Nfc, &nfc, &nfc),
            (NormalizationForm::Nfc, &nfd, &nfc),
            (NormalizationForm::Nfc, &nfkc, &nfkc),
            (NormalizationForm::Nfc, &nfkd, &nfkc),
            (NormalizationForm::Nfd, &source, &nfd),
            (NormalizationForm::Nfd, &nfc, &nfd),
            (NormalizationForm::Nfd, &nfd, &nfd),
            (NormalizationForm::Nfd, &nfkc, &nfkd),
            (NormalizationForm::Nfd, &nfkd, &nfkd),
            (NormalizationForm::Nfkc, &source, &nfkc),
            (NormalizationForm::Nfkc, &nfc, &nfkc),
            (NormalizationForm::Nfkc, &nfd, &nfkc),
            (NormalizationForm::Nfkc, &nfkc, &nfkc),
            (NormalizationForm::Nfkc, &nfkd, &nfkc),
            (NormalizationForm::Nfkd, &source, &nfkd),
            (NormalizationForm::Nfkd, &nfc, &nfkd),
            (NormalizationForm::Nfkd, &nfd, &nfkd),
            (NormalizationForm::Nfkd, &nfkc, &nfkd),
            (NormalizationForm::Nfkd, &nfkd, &nfkd),
        ];
        for (form, input, expected) in cases_inputs {
            let actual = normalize_cps(input, *form);
            checks += 1;
            if actual.as_slice() != expected.as_slice() {
                failures += 1;
                if failures <= 10 {
                    eprintln!("case {cases}: {form:?} input={input:04X?}\n  expected={expected:04X?}\n  actual  ={actual:04X?}");
                }
            }
        }
    }

    assert_eq!(failures, 0, "{failures} of {checks} normalization checks failed");
    eprintln!("NormalizationTest.txt: {cases} cases, {checks} checks passed");
}
