//! UAX #14 Unicode Line Breaking Algorithm (Unicode 18.0.0, Revision 57).
//!
//! Scalar API over the generated Line_Break data. The full LB1..LB31 rules
//! live in the byte segmenter ([`crate::segmentation`], `BoundaryType::Line`);
//! these helpers reuse it via a UTF-8 round-trip, so both paths agree.

use crate::segmentation::{BoundaryType, Segmenter};
use crate::systems;
use crate::ucd;

/// Line_Break class of `cp`.
#[inline]
pub fn line_break(cp: u32) -> u8 {
    ucd::line_break(cp)
}

fn utf8_encode(cps: &[u32]) -> Option<(Vec<u8>, Vec<usize>)> {
    let mut bytes = Vec::with_capacity(cps.len() * 4);
    let mut offsets = Vec::with_capacity(cps.len() + 1);
    for &cp in cps {
        offsets.push(bytes.len());
        let ch = char::from_u32(cp)?;
        let mut buf = [0u8; 4];
        bytes.extend_from_slice(ch.encode_utf8(&mut buf).as_bytes());
    }
    offsets.push(bytes.len());
    Some((bytes, offsets))
}

fn decode_offsets(bytes: &[u8], count: usize) -> Vec<usize> {
    let mut offsets = Vec::with_capacity(count + 1);
    let mut b = 0;
    while b < bytes.len() {
        offsets.push(b);
        let b0 = bytes[b];
        let adv = if b0 < 0x80 {
            1
        } else if b0 < 0xE0 {
            2
        } else if b0 < 0xF0 {
            3
        } else {
            4
        };
        b += adv;
    }
    offsets.push(bytes.len());
    offsets
}

/// Find the end of the line segment starting at `start`.
pub fn next_break(cps: &[u32], start: usize) -> usize {
    if start >= cps.len() {
        return cps.len();
    }
    let Some((bytes, byte_offsets)) = utf8_encode(cps) else {
        return (start + 1).min(cps.len());
    };
    let system = systems::utf8::system();
    let Ok(mut seg) = Segmenter::new(system, &bytes, BoundaryType::Line) else {
        return (start + 1).min(cps.len());
    };
    let byte_start = byte_offsets[start];
    let mut byte_end = byte_offsets[cps.len()];
    while let Some(out) = seg.next_segment() {
        if out.byte_offset == byte_start {
            byte_end = out.byte_offset + out.byte_length;
            break;
        }
        if out.byte_offset > byte_start {
            byte_end = out.byte_offset;
            break;
        }
    }
    let scalar_offsets = decode_offsets(&bytes, cps.len());
    for (k, &off) in scalar_offsets.iter().enumerate() {
        if off >= byte_end {
            return k.max(start + 1).min(cps.len());
        }
    }
    cps.len()
}

/// True when a break occurs immediately before `index`.
pub fn is_break(cps: &[u32], index: usize) -> bool {
    if index == 0 || index >= cps.len() {
        return true;
    }
    let mut start = 0;
    while start < index {
        let next = next_break(cps, start);
        if next <= start {
            return false;
        }
        start = next;
    }
    start == index
}
