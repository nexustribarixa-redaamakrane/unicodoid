//! UTS #58 Link Detection and Formatting: URLs and Email Addresses
//! (Unicode 18.0.0, Revision 3).
//!
//! Implements the code point machinery that operates after the host has been
//! parsed:
//!
//! * `Link_Term` / `Link_Bracket` / `Link_Email` property lookups (backed by
//!   the generated Unicode 18.0.0 tables),
//! * the link termination algorithm (UTS58 §3.5.1),
//! * the URL minimal escaping algorithm (UTS58 §4.1),
//! * the email local-part backward scan (UTS58 §5.2).
//!
//! URL *initiation* (finding the Scheme, Host and optional Port, including
//! registrable-domain and TLD checks) is explicitly out of scope, as it is in
//! the specification itself (UTS58 §1.4 and §3.2).

use crate::ucd;

/// `Link_Term` values from the generated tables.
mod lt {
    pub const INCLUDE: u8 = 1;
    pub const HARD: u8 = 2;
    pub const SOFT: u8 = 3;
    pub const CLOSE: u8 = 4;
    pub const OPEN: u8 = 5;
}

const PART_NONE: i8 = -1;
const PART_PATH: i8 = 0;
const PART_QUERY: i8 = 1;
const PART_FRAGMENT: i8 = 2;
const PART_DIRECTIVE: i8 = 3;

/// URL parts for minimal escaping (UTS58 §4.1).
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum UrlPart {
    Path,
    Query,
    Fragment,
    FragmentDirective,
}

impl UrlPart {
    fn number(self) -> i8 {
        match self {
            UrlPart::Path => PART_PATH,
            UrlPart::Query => PART_QUERY,
            UrlPart::Fragment => PART_FRAGMENT,
            UrlPart::FragmentDirective => PART_DIRECTIVE,
        }
    }

    fn initiator(self) -> &'static [u8] {
        match self {
            UrlPart::Path => b"/",
            UrlPart::Query => b"?",
            UrlPart::Fragment => b"#",
            UrlPart::FragmentDirective => b":~:",
        }
    }
}

fn is_colon_tilde_colon(cps: &[u32], i: usize) -> bool {
    i + 2 < cps.len() && cps[i] == 0x003A && cps[i + 1] == 0x007E && cps[i + 2] == 0x003A
}

fn match_terminator(cps: &[u32], i: usize, part: i8) -> Option<usize> {
    let c = cps[i];
    match part {
        PART_PATH if c == 0x003F || c == 0x0023 => Some(1),
        PART_QUERY if c == 0x0023 => Some(1),
        PART_FRAGMENT if is_colon_tilde_colon(cps, i) => Some(3),
        _ => None,
    }
}

fn match_initiator(cps: &[u32], i: usize, previous_part: i8) -> Option<(usize, i8)> {
    let c = cps[i];
    if c == 0x002F {
        return Some((1, PART_PATH));
    }
    if c == 0x003F {
        return Some((1, PART_QUERY));
    }
    if c == 0x0023 {
        return Some((1, PART_FRAGMENT));
    }
    if is_colon_tilde_colon(cps, i) {
        if previous_part == PART_FRAGMENT || previous_part == PART_DIRECTIVE {
            return Some((3, PART_DIRECTIVE));
        }
    }
    None
}

fn match_clear_stack_open(cps: &[u32], i: usize, part: i8) -> Option<usize> {
    let c = cps[i];
    match part {
        PART_PATH if c == 0x002F => Some(1),
        PART_QUERY if c == 0x003D || c == 0x0026 => Some(1),
        PART_DIRECTIVE => {
            if c == 0x0026 || c == 0x002C {
                Some(1)
            } else if is_colon_tilde_colon(cps, i) {
                Some(3)
            } else {
                None
            }
        }
        _ => None,
    }
}

/// Link detection termination algorithm (UTS58 §3.5.1).
///
/// `start` is the index immediately after the host, at the beginning of the
/// Path, Query or Fragment, if any.  Returns the offset one past the end of
/// the final URL part, or `start` when no URL part is found.
pub fn link_detection_end(cps: &[u32], start: usize) -> usize {
    let mut last_safe = start;
    let mut part = PART_NONE;
    let mut previous_part = PART_NONE;
    let mut i = start;
    let mut stack: Vec<u32> = Vec::new();

    while i < cps.len() {
        // Step (a): a terminator ends the current part.
        if part != PART_NONE {
            if let Some(_advance) = match_terminator(cps, i, part) {
                previous_part = part;
                part = PART_NONE;
                // Do not advance i; the code point is reconsidered below.
            }
        }

        // Step (b): with no part active, try to start one.
        if part == PART_NONE {
            match match_initiator(cps, i, previous_part) {
                Some((advance, new_part)) => {
                    part = new_part;
                    i += advance;
                    last_safe = i;
                    stack.clear();
                    continue;
                }
                None => return last_safe,
            }
        }

        // Step (c): a clearStackOpen element clears the bracket stack.
        if let Some(advance) = match_clear_stack_open(cps, i, part) {
            i += advance;
            last_safe = i;
            stack.clear();
            continue;
        }

        // Steps (d)-(i): classify the code point with Link_Term.
        match ucd::link_term(cps[i]) {
            lt::INCLUDE => {
                last_safe = i + 1;
                i += 1;
            }
            lt::SOFT => {
                i += 1;
            }
            lt::HARD => return last_safe,
            lt::OPEN => {
                if stack.len() == 125 {
                    return last_safe;
                }
                stack.push(cps[i]);
                last_safe = i + 1;
                i += 1;
            }
            lt::CLOSE => {
                let last_open = match stack.pop() {
                    Some(open) => open,
                    None => return last_safe,
                };
                match ucd::link_bracket(cps[i]) {
                    Some(expected) if expected == last_open => {
                        last_safe = i + 1;
                        i += 1;
                    }
                    _ => return last_safe,
                }
            }
            _ => return last_safe,
        }
    }

    last_safe
}

/// Email local-part detection (UTS58 §5.2).
///
/// Scans backwards from the `@` at `at_index` and returns the offset of the
/// beginning of the local part, optionally including a `mailto:` prefix.
/// Returns `None` when there is no valid local part.
pub fn email_local_part_start(cps: &[u32], at_index: usize) -> Option<usize> {
    if at_index == 0 || at_index > cps.len() {
        return None;
    }

    // A '.' immediately before '@' fails.
    if cps[at_index - 1] == 0x002E {
        return None;
    }

    let mut start = at_index;
    for i in (0..at_index).rev() {
        let cp = cps[i];
        if cp == 0x002E {
            if i + 1 < at_index && cps[i + 1] == 0x002E {
                return None;
            }
            continue; // medial dot: keep scanning
        }
        if !ucd::link_email(cp) {
            start = i + 1;
            break;
        }
        start = i;
    }

    // The local part may not begin with '.' or be empty.
    if start < at_index && cps[start] == 0x002E {
        return None;
    }
    if start == at_index {
        return None;
    }

    // Include a leading "mailto:" if present.
    const MAILTO: [u32; 7] = [
        b'm' as u32,
        b'a' as u32,
        b'i' as u32,
        b'l' as u32,
        b't' as u32,
        b'o' as u32,
        b':' as u32,
    ];
    if start >= 7 && cps[start - 7..start] == MAILTO {
        start -= 7;
    }

    Some(start)
}

fn utf8_encode_cp(cp: u32, out: &mut [u8; 4]) -> usize {
    if cp < 0x80 {
        out[0] = cp as u8;
        1
    } else if cp < 0x800 {
        out[0] = 0xC0 | (cp >> 6) as u8;
        out[1] = 0x80 | (cp & 0x3F) as u8;
        2
    } else if cp < 0x10000 {
        out[0] = 0xE0 | (cp >> 12) as u8;
        out[1] = 0x80 | ((cp >> 6) & 0x3F) as u8;
        out[2] = 0x80 | (cp & 0x3F) as u8;
        3
    } else {
        out[0] = 0xF0 | (cp >> 18) as u8;
        out[1] = 0x80 | ((cp >> 12) & 0x3F) as u8;
        out[2] = 0x80 | ((cp >> 6) & 0x3F) as u8;
        out[3] = 0x80 | (cp & 0x3F) as u8;
        4
    }
}

fn push_cp(out: &mut Vec<u8>, cp: u32) {
    let mut buf = [0u8; 4];
    let n = utf8_encode_cp(cp, &mut buf);
    out.extend_from_slice(&buf[..n]);
}

fn push_percent(out: &mut Vec<u8>, cp: u32) {
    let mut buf = [0u8; 4];
    let n = utf8_encode_cp(cp, &mut buf);
    const HEX: &[u8; 16] = b"0123456789ABCDEF";
    for &byte in &buf[..n] {
        out.push(b'%');
        out.push(HEX[(byte >> 4) as usize]);
        out.push(HEX[(byte & 0x0F) as usize]);
    }
}

/// URL Minimal Escaping Algorithm (UTS58 §4.1).
///
/// `cps` are the decoded code points of the part *without* the initiator
/// (which the algorithm adds).  When `is_last` is true a final Soft code point
/// of the whole URL is percent-escaped.
pub fn minimal_escape_part(part: UrlPart, cps: &[u32], is_last: bool) -> Vec<u8> {
    let mut out: Vec<u8> = Vec::new();
    out.extend_from_slice(part.initiator());

    let mut copied_already = 0usize;
    let p = part.number();
    let mut stack: Vec<u32> = Vec::new();

    for i in 0..cps.len() {
        let is_terminator = match_terminator(cps, i, p).is_some();
        let term = if is_terminator {
            lt::HARD
        } else {
            ucd::link_term(cps[i])
        };

        if match_clear_stack_open(cps, i, p).is_some() {
            stack.clear();
        }

        match term {
            lt::INCLUDE => {
                for &cp in &cps[copied_already..i] {
                    push_cp(&mut out, cp);
                }
                push_cp(&mut out, cps[i]);
                copied_already = i + 1;
            }
            lt::HARD => {
                for &cp in &cps[copied_already..i] {
                    push_cp(&mut out, cp);
                }
                push_percent(&mut out, cps[i]);
                copied_already = i + 1;
            }
            lt::SOFT => {}
            lt::OPEN => {
                if stack.len() == 125 {
                    for &cp in &cps[copied_already..i] {
                        push_cp(&mut out, cp);
                    }
                    push_percent(&mut out, cps[i]);
                } else {
                    stack.push(cps[i]);
                    for &cp in &cps[copied_already..i] {
                        push_cp(&mut out, cp);
                    }
                    push_cp(&mut out, cps[i]);
                }
                copied_already = i + 1;
            }
            lt::CLOSE => {
                let matched = stack
                    .last()
                    .copied()
                    .and_then(|last_open| ucd::link_bracket(cps[i]).map(|e| e == last_open))
                    .unwrap_or(false);
                if matched {
                    stack.pop();
                    for &cp in &cps[copied_already..i] {
                        push_cp(&mut out, cp);
                    }
                    push_cp(&mut out, cps[i]);
                } else {
                    for &cp in &cps[copied_already..i] {
                        push_cp(&mut out, cp);
                    }
                    push_percent(&mut out, cps[i]);
                }
                copied_already = i + 1;
            }
            _ => {}
        }
    }

    if !is_last {
        for &cp in &cps[copied_already..] {
            push_cp(&mut out, cp);
        }
    } else if copied_already < cps.len() {
        for &cp in &cps[copied_already..cps.len() - 1] {
            push_cp(&mut out, cp);
        }
        push_percent(&mut out, cps[cps.len() - 1]);
    }

    out
}
