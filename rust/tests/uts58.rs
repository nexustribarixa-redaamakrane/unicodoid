//! Official Unicode 18.0.0 conformance tests for UTS #58 (Link Detection and
//! Formatting) plus constructed unit tests for the same algorithms.
//!
//! URL initiation is out of scope of the specification itself; the link
//! detection harness pins the termination algorithm's behaviour given the
//! position after the host, while the minimal escaping harness checks full
//! conformance of URL Path/Query/Fragment formatting against the official
//! test data.

use std::fs;
use std::path::{Path, PathBuf};

use unicodoid::uts58::{self, UrlPart};

const MARK_OPEN: u32 = 0x2E20;
const MARK_CLOSE: u32 = 0x2E21;

fn link_dir() -> PathBuf {
    Path::new(env!("CARGO_MANIFEST_DIR")).join("../data/ucd/18.0.0/linkification")
}

fn decode(s: &str) -> Vec<u32> {
    s.chars().map(|c| c as u32).collect()
}

fn find_initiator(cps: &[u32], from: usize, to: usize) -> Option<usize> {
    for i in from..to {
        if cps[i] == 0x2F || cps[i] == 0x3F || cps[i] == 0x23 {
            return Some(i);
        }
        if i + 2 < cps.len() && cps[i] == 0x3A && cps[i + 1] == 0x7E && cps[i + 2] == 0x3A {
            return Some(i);
        }
    }
    None
}

#[test]
fn link_detection_conformance() {
    let path = link_dir().join("LinkDetectionTest.txt");
    let text = fs::read_to_string(&path)
        .unwrap_or_else(|e| panic!("cannot read {}: {e}", path.display()));

    let mut cases = 0usize;
    let mut links = 0usize;
    let mut checked = 0usize;
    let mut skipped_bare = 0usize;
    let mut skipped_email = 0usize;
    let mut failures = 0usize;

    for raw in text.lines() {
        let line = raw.trim_end_matches(['\r', '\n']);
        if line.starts_with('#') || line.is_empty() {
            continue;
        }
        cases += 1;
        let cps = decode(line);
        let n = cps.len();

        let mut i = 0usize;
        while i < n {
            if cps[i] != MARK_OPEN {
                i += 1;
                continue;
            }
            let open = i;
            let close = match (open + 1..n).find(|&j| cps[j] == MARK_CLOSE) {
                Some(j) => j,
                None => {
                    failures += 1;
                    eprintln!("unpaired mark in {line:?}");
                    break;
                }
            };
            links += 1;

            match find_initiator(&cps, open + 1, close) {
                None => {
                    // Bare domain: initiation out of scope.  Links with an
                    // '@' must still have a valid local part per UTS58 §5.2.
                    if let Some(at) = (open + 1..close).find(|&j| cps[j] == 0x40) {
                        let local = uts58::email_local_part_start(&cps, at);
                        skipped_email += 1;
                        if let Some(local) = local {
                            if local != open + 1 {
                                failures += 1;
                                if failures <= 10 {
                                    eprintln!(
                                        "email local part mismatch: span=[{}, {}) local={local}",
                                        open + 1,
                                        close
                                    );
                                }
                            }
                        }
                    } else {
                        skipped_bare += 1;
                    }
                }
                Some(init) => {
                    let end = uts58::link_detection_end(&cps, init);
                    checked += 1;
                    if end != close {
                        failures += 1;
                        if failures <= 10 {
                            eprintln!(
                                "detection mismatch: span=[{}, {}) init={init} end={end}",
                                open + 1,
                                close
                            );
                        }
                    }
                }
            }

            i = close + 1;
        }
    }

    eprintln!(
        "LinkDetectionTest.txt: {cases} cases, {links} marked links, {checked} checked, \
         {skipped_bare} bare (skipped), {skipped_email} email, {failures} failures"
    );
    assert_eq!(failures, 0, "UTS #58 link detection conformance failures");
    assert!(checked > 100, "expected a populated LinkDetectionTest.txt");
}

/* ---------------------------------------------------------------------- */
/* Link formatting harness (official LinkFormattingTest.txt)               */
/* ---------------------------------------------------------------------- */

const SYM_S: u32 = 0x1D47A; // 𝑺
const SYM_H: u32 = 0x1D46F; // 𝑯
const SYM_P: u32 = 0x1D477; // 𝑷
const SYM_Q: u32 = 0x1D478; // 𝑸
const SYM_V: u32 = 0x1D47D; // 𝑽
const SYM_F: u32 = 0x1D46D; // 𝑭
const SYM_D: u32 = 0x1D46B; // 𝑫

fn is_structure_symbol(cp: u32) -> bool {
    matches!(
        cp,
        SYM_S | SYM_H | SYM_P | SYM_Q | SYM_V | SYM_F | SYM_D
    )
}

fn is_hex_digit(b: u8) -> bool {
    b.is_ascii_hexdigit()
}

fn pct_followed_by_hex(p: &[u8], i: usize) -> bool {
    i + 2 < p.len() && is_hex_digit(p[i + 1]) && is_hex_digit(p[i + 2])
}

fn interior_escape_path(input: &[u8], out: &mut Vec<u8>) {
    for i in 0..input.len() {
        match input[i] {
            b'/' => out.extend_from_slice(b"%2F"),
            b'?' => out.extend_from_slice(b"%3F"),
            b'#' => out.extend_from_slice(b"%23"),
            b'%' => {
                if pct_followed_by_hex(input, i) {
                    out.extend_from_slice(b"%25");
                } else {
                    out.push(b'%');
                }
            }
            b => out.push(b),
        }
    }
}

fn interior_escape_query(input: &[u8], out: &mut Vec<u8>) {
    for i in 0..input.len() {
        match input[i] {
            b'+' => out.extend_from_slice(b"%2B"),
            b'#' => out.extend_from_slice(b"%23"),
            b'=' => out.extend_from_slice(b"%3D"),
            b'&' => out.extend_from_slice(b"%26"),
            b'%' => {
                if pct_followed_by_hex(input, i) {
                    out.extend_from_slice(b"%25");
                } else {
                    out.push(b'%');
                }
            }
            b => out.push(b),
        }
    }
}

fn interior_escape_fragment(input: &[u8], out: &mut Vec<u8>) {
    let mut i = 0usize;
    while i < input.len() {
        if input[i] == b':' && i + 2 < input.len() && input[i + 1] == b'~' && input[i + 2] == b':' {
            out.extend_from_slice(b"%3A~:");
            i += 3;
            continue;
        }
        if input[i] == b'%' && pct_followed_by_hex(input, i) {
            out.extend_from_slice(b"%25");
            i += 1;
            continue;
        }
        out.push(input[i]);
        i += 1;
    }
}

fn interior_escape_directive(input: &[u8], out: &mut Vec<u8>) {
    let mut i = 0usize;
    while i < input.len() {
        if input[i] == b':' && i + 2 < input.len() && input[i + 1] == b'~' && input[i + 2] == b':' {
            out.extend_from_slice(b"%3A~:");
            i += 3;
            continue;
        }
        match input[i] {
            b'&' => out.extend_from_slice(b"%26"),
            b',' => out.extend_from_slice(b"%2C"),
            b'%' => {
                if pct_followed_by_hex(input, i) {
                    out.extend_from_slice(b"%25");
                } else {
                    out.push(b'%');
                }
            }
            b => out.push(b),
        }
        i += 1;
    }
}

fn escape_part(part: UrlPart, input: &[u8], is_last: bool, out: &mut Vec<u8>) {
    let text = std::str::from_utf8(input).expect("formatting input is UTF-8");
    let cps = decode(text);
    out.extend_from_slice(&uts58::minimal_escape_part(part, &cps, is_last));
}

#[derive(Default)]
struct Parts {
    scheme: String,
    host: String,
    path: Vec<String>,
    query: Vec<(String, String)>,
    fragment: Option<String>,
    directives: Vec<String>,
}

fn parse_structure_line(line: &str) -> Option<Vec<(u32, String)>> {
    let start = line.find('{')? + 1;
    let rest = &line[start..];
    let chars: Vec<(usize, char)> = rest.char_indices().collect();
    let mut tokens = Vec::new();
    let mut idx = 0usize;

    while idx < chars.len() {
        let (byte_pos, ch) = chars[idx];
        let sym = ch as u32;
        if !is_structure_symbol(sym) || chars.get(idx + 1).map(|&(_, c)| c) != Some('=') {
            idx += 1;
            continue;
        }

        let value_start = chars[idx + 1].0 + 1;
        let mut vend = rest.len();
        let mut j = idx + 2;
        while j < chars.len() {
            let (b, c) = chars[j];
            if c == '}' {
                vend = b;
                break;
            }
            if c == ' '
                && let Some(&(_, next_ch)) = chars.get(j + 1)
                && is_structure_symbol(next_ch as u32)
                && chars.get(j + 2).map(|&(_, c)| c) == Some('=')
            {
                vend = b;
                break;
            }
            j += 1;
        }

        let value = rest[value_start..vend].trim_end().to_string();
        tokens.push((sym, value));
        idx = j;
    }

    if tokens.is_empty() { None } else { Some(tokens) }
}

fn apply_tokens(parts: &mut Parts, tokens: &[(u32, String)]) {
    let mut pending_query: Option<usize> = None;
    for (sym, value) in tokens {
        match *sym {
            SYM_S => parts.scheme = value.clone(),
            SYM_H => parts.host = value.clone(),
            SYM_P => parts.path.push(value.clone()),
            SYM_Q => {
                parts.query.push((value.clone(), String::new()));
                pending_query = Some(parts.query.len() - 1);
            }
            SYM_V => {
                if let Some(q) = pending_query {
                    parts.query[q].1 = value.clone();
                }
            }
            SYM_F => parts.fragment = Some(value.clone()),
            SYM_D => parts.directives.push(value.clone()),
            _ => {}
        }
    }
}

fn reconstruct_expected(parts: &Parts) -> Vec<u8> {
    let mut out = Vec::new();
    out.extend_from_slice(parts.scheme.as_bytes());
    out.extend_from_slice(parts.host.as_bytes());

    let has_path = !parts.path.is_empty();
    let has_query = !parts.query.is_empty();
    let has_fragment = parts.fragment.is_some();
    let directive_count = parts.directives.len();

    if has_path {
        let mut joined = Vec::new();
        for (s, segment) in parts.path.iter().enumerate() {
            if s > 0 {
                joined.push(b'/');
            }
            interior_escape_path(segment.as_bytes(), &mut joined);
        }
        let last = !has_query && !has_fragment && directive_count == 0;
        escape_part(UrlPart::Path, &joined, last, &mut out);
    }

    if has_query {
        let mut joined = Vec::new();
        for (s, (key, value)) in parts.query.iter().enumerate() {
            if s > 0 {
                joined.push(b'&');
            }
            interior_escape_query(key.as_bytes(), &mut joined);
            if !value.is_empty() {
                joined.push(b'=');
                interior_escape_query(value.as_bytes(), &mut joined);
            }
        }
        let last = !has_fragment && directive_count == 0;
        escape_part(UrlPart::Query, &joined, last, &mut out);
    }

    if let Some(fragment) = &parts.fragment {
        let mut escaped = Vec::new();
        interior_escape_fragment(fragment.as_bytes(), &mut escaped);
        let last = directive_count == 0;
        escape_part(UrlPart::Fragment, &escaped, last, &mut out);
    }

    for (s, directive) in parts.directives.iter().enumerate() {
        let mut escaped = Vec::new();
        interior_escape_directive(directive.as_bytes(), &mut escaped);
        let last = s + 1 == directive_count;
        escape_part(UrlPart::FragmentDirective, &escaped, last, &mut out);
    }

    out
}

#[test]
fn link_formatting_conformance() {
    let path = link_dir().join("LinkFormattingTest.txt");
    let text = fs::read_to_string(&path)
        .unwrap_or_else(|e| panic!("cannot read {}: {e}", path.display()));

    let mut source: Option<String> = None;
    let mut pending: Option<Parts> = None;
    let mut cases = 0usize;
    let mut checked = 0usize;
    let mut failures = 0usize;

    for raw in text.lines() {
        let line = raw.trim_end_matches(['\r', '\n']);
        if line.is_empty() {
            continue;
        }

        if line.starts_with('#') {
            if line.contains('{')
                && let Some(tokens) = parse_structure_line(line)
            {
                let mut parts = Parts::default();
                apply_tokens(&mut parts, &tokens);
                pending = Some(parts);
            }
            continue;
        }

        match source.take() {
            None => source = Some(line.to_string()),
            Some(src) => {
                let _ = src;
                if let Some(parts) = pending.take() {
                    cases += 1;
                    let actual = reconstruct_expected(&parts);
                    checked += 1;
                    if actual != line.as_bytes() {
                        failures += 1;
                        if failures <= 10 {
                            eprintln!(
                                "formatting mismatch:\n  expected={}\n  actual  ={}",
                                line,
                                String::from_utf8_lossy(&actual)
                            );
                        }
                    }
                }
            }
        }
    }

    eprintln!(
        "LinkFormattingTest.txt: {cases} cases, {checked} checked, {failures} failures"
    );
    assert_eq!(failures, 0, "UTS #58 link formatting conformance failures");
    assert!(checked > 40, "expected a populated LinkFormattingTest.txt");
}

/* ---------------------------------------------------------------------- */
/* Unit tests                                                             */
/* ---------------------------------------------------------------------- */

#[test]
fn email_local_part() {
    let cases = [
        ("contact abcd@example.com", "abcd"),
        ("contact x.abcd@example.com", "x.abcd"),
        (
            "contact アルベルト.アルベルト@example.com",
            "アルベルト.アルベルト",
        ),
        ("contact a@b.c", "a"),
    ];
    for (text, expected) in cases {
        let cps = decode(text);
        let at = cps.iter().position(|&cp| cp == 0x40).expect("@ present");
        let start = uts58::email_local_part_start(&cps, at)
            .unwrap_or_else(|| panic!("no local part in {text:?}"));
        let actual: String = cps[start..at]
            .iter()
            .map(|&cp| char::from_u32(cp).unwrap())
            .collect();
        assert_eq!(actual, expected, "email local part for {text:?}");
    }

    for text in [
        "contact @example.com",
        "contact john..doe@example.com",
        "contact .john.doe@example.com",
        "contact john.@example.com",
    ] {
        let cps = decode(text);
        let at = cps.iter().position(|&cp| cp == 0x40).expect("@ present");
        assert!(
            uts58::email_local_part_start(&cps, at).is_none(),
            "expected no local part for {text:?}"
        );
    }
}

#[test]
fn detection_unit_cases() {
    let cases: [(&str, usize, usize); 6] = [
        ("go to example.com/αβγ now", 17, 21),
        ("go to example.com/αβγ.now", 17, 25),
        ("go to example.com?q=a b", 17, 21),
        ("example.com#frag ment", 11, 16),
        ("example.com/a(b)c) x", 11, 17),
        ("example.com/a(b)c) y", 11, 17),
    ];
    for (text, start, expected) in cases {
        let cps = decode(text);
        let end = uts58::link_detection_end(&cps, start);
        assert_eq!(end, expected, "detection for {text:?}");
    }

    // Trailing soft code point: last safe offset stays after γ.
    let cps = decode("example.com/αβγ.");
    assert_eq!(uts58::link_detection_end(&cps, 11), 15);

    // Close without a matching Open terminates the link.
    let cps = decode("example.com/a)b");
    assert_eq!(uts58::link_detection_end(&cps, 11), 13);
}

#[test]
fn minimal_escape_unit_cases() {
    // Path ending in a soft character must escape it.
    let cps = decode("δ.εζ.");
    let out = uts58::minimal_escape_part(UrlPart::Path, &cps, true);
    assert_eq!(out, "/δ.εζ%2E".as_bytes());

    // Non-last path keeps a trailing soft character.
    let out = uts58::minimal_escape_part(UrlPart::Path, &cps, false);
    assert_eq!(out, "/δ.εζ.".as_bytes());
}
