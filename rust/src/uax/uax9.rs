//! UAX #9 Unicode Bidirectional Algorithm (Unicode 18.0.0, Revision 52).
//!
//! Paragraph level resolution (P2/P3, W1..W7, N0..N2, I1/I2, X1..X10 at
//! level granularity) and visual reordering (L2). Mirroring covers the
//! ASCII pairs; the full 438-entry BidiMirroring table lives in the C
//! implementation (the Rust generated tables do not carry it).

use crate::ucd;
use crate::ucd::bidi as bc;

/// Requested paragraph base direction.
pub mod dir {
    pub const LTR: i32 = 0;
    pub const RTL: i32 = 1;
    pub const AUTO: i32 = -1;
}

fn is_strong_l(c: u8) -> bool {
    c == bc::L
}
fn is_strong_r(c: u8) -> bool {
    c == bc::R || c == bc::AL
}

/// Paragraph embedding level (0 LTR, 1 RTL) per P2/P3.
pub fn paragraph_level(cps: &[u32], base_dir: i32) -> u8 {
    if base_dir == dir::RTL {
        return 1;
    }
    if base_dir == dir::LTR {
        return 0;
    }
    for &cp in cps {
        let c = ucd::bidi_class(cp);
        if is_strong_r(c) {
            return 1;
        }
        if is_strong_l(c) {
            return 0;
        }
    }
    0
}

fn resolve_alloc(cps: &[u32], base_dir: i32) -> Vec<u8> {
    let len = cps.len();
    if len == 0 {
        return Vec::new();
    }
    let mut cls: Vec<u8> = cps.iter().map(|&cp| ucd::bidi_class(cp)).collect();
    let para = paragraph_level(cps, base_dir);
    let mut lvl = vec![para; len];

    // W1: NSM inherits preceding strong, else ON.
    for i in 0..len {
        if cls[i] != bc::NSM {
            continue;
        }
        let mut resolved = bc::ON;
        if i > 0 {
            let mut j = i;
            while j > 0 {
                j -= 1;
                let c = cls[j];
                if c == bc::NSM || c == bc::BN {
                    continue;
                }
                resolved = c;
                break;
            }
        }
        if resolved == bc::BN {
            resolved = bc::ON;
        }
        cls[i] = resolved;
    }

    // W2: EN -> AN when preceded (skipping EN/ES/ET/AN/CS) by R/AL.
    for i in 0..len {
        if cls[i] != bc::EN {
            continue;
        }
        for k in (0..i).rev() {
            let b = cls[k];
            if matches!(b, bc::EN | bc::ES | bc::ET | bc::AN | bc::CS) {
                continue;
            }
            if b == bc::R || b == bc::AL {
                cls[i] = bc::AN;
            }
            break;
        }
    }

    // W3: AL -> R.
    for c in cls.iter_mut() {
        if *c == bc::AL {
            *c = bc::R;
        }
    }

    // W4: single ES between EN -> EN; single CS between N -> N.
    for i in 1..len.saturating_sub(1) {
        if cls[i] == bc::ES && cls[i - 1] == bc::EN && cls[i + 1] == bc::EN {
            cls[i] = bc::EN;
        }
        if cls[i] == bc::CS {
            let lp = matches!(cls[i - 1], bc::EN | bc::AN);
            let rp = matches!(cls[i + 1], bc::EN | bc::AN);
            if lp && rp {
                cls[i] = bc::EN;
            }
        }
    }

    // W5: ET adjacent to EN (skipping ET) -> EN.
    for i in 0..len {
        if cls[i] != bc::ET {
            continue;
        }
        let mut near_en = false;
        let mut j = i;
        while j > 0 {
            j -= 1;
            if cls[j] == bc::ET {
                continue;
            }
            near_en = cls[j] == bc::EN;
            break;
        }
        let mut j = i + 1;
        while j < len {
            if cls[j] == bc::ET {
                j += 1;
                continue;
            }
            near_en = near_en || cls[j] == bc::EN;
            break;
        }
        if near_en {
            cls[i] = bc::EN;
        }
    }

    // W6: remaining ES/ET/CS -> ON.
    for c in cls.iter_mut() {
        if matches!(*c, bc::ES | bc::ET | bc::CS) {
            *c = bc::ON;
        }
    }

    // W7: EN -> L when preceded (skipping WS/ES/ET) by L.
    for i in 0..len {
        if cls[i] != bc::EN {
            continue;
        }
        for k in (0..i).rev() {
            let b = cls[k];
            if matches!(b, bc::WS | bc::ES | bc::ET) {
                continue;
            }
            if b == bc::L {
                cls[i] = bc::L;
            }
            break;
        }
    }

    // N1/N2: neutrals resolve to enclosing strong or paragraph type.
    for i in 0..len {
        if cls[i] != bc::ON {
            continue;
        }
        let mut left = if para == 1 { bc::R } else { bc::L };
        for k in (0..i).rev() {
            let b = cls[k];
            if b == bc::ON {
                continue;
            }
            if matches!(b, bc::EN | bc::AN) {
                left = bc::L;
            } else if b == bc::L || b == bc::R {
                left = b;
            }
            break;
        }
        let mut right = if para == 1 { bc::R } else { bc::L };
        for k in (i + 1)..len {
            let b = cls[k];
            if b == bc::ON {
                continue;
            }
            if matches!(b, bc::EN | bc::AN) {
                right = bc::L;
            } else if b == bc::L || b == bc::R {
                right = b;
            }
            break;
        }
        if left == right {
            cls[i] = left;
        } else {
            cls[i] = if para == 1 { bc::R } else { bc::L };
        }
    }

    // I1/I2 + X1..X10 at level granularity (embeddings as increments).
    let mut delta: i32 = 0;
    for i in 0..len {
        let c = cls[i];
        let cur_base = (para as i32 + 2 * delta).clamp(0, 125) as u8;
        match c {
            bc::LRE | bc::RLE | bc::LRO | bc::RLO => {
                delta += 1;
                lvl[i] = cur_base;
            }
            bc::PDF => {
                if delta > 0 {
                    delta -= 1;
                }
                lvl[i] = cur_base;
            }
            bc::FSI | bc::LRI | bc::RLI | bc::PDI => {
                lvl[i] = cur_base;
            }
            bc::B | bc::S => {
                lvl[i] = para;
            }
            bc::BN => {
                lvl[i] = cur_base;
            }
            bc::R | bc::AN => {
                lvl[i] = if cur_base % 2 == 1 { cur_base } else { cur_base + 1 };
            }
            bc::L | bc::EN => {
                lvl[i] = if cur_base % 2 == 1 { cur_base + 1 } else { cur_base };
            }
            _ => {
                lvl[i] = cur_base;
            }
        }
    }
    lvl
}

/// Resolve bidirectional levels for one paragraph.
pub fn resolve_levels(cps: &[u32], base_dir: i32) -> Vec<u8> {
    resolve_alloc(cps, base_dir)
}

/// Reorder a paragraph to visual order (L2) with ASCII-pair mirroring.
pub fn visual_order(cps: &[u32], base_dir: i32) -> Vec<u32> {
    if cps.is_empty() {
        return Vec::new();
    }
    let mut lvl = resolve_alloc(cps, base_dir);
    let para = paragraph_level(cps, base_dir);
    for (i, &cp) in cps.iter().enumerate() {
        let c = ucd::bidi_class(cp);
        if c == bc::B || c == bc::S {
            lvl[i] = para;
        }
    }
    let mut order: Vec<usize> = (0..cps.len()).collect();
    let max_l = lvl.iter().copied().max().unwrap_or(0);
    let min_odd = lvl.iter().copied().filter(|l| l % 2 == 1).min();
    if let Some(min_odd) = min_odd {
        let mut k = max_l as i32;
        while k >= min_odd as i32 {
            let mut i = 0;
            while i < order.len() {
                if lvl[order[i]] < k as u8 {
                    i += 1;
                    continue;
                }
                let mut j = i;
                while j < order.len() && lvl[order[j]] >= k as u8 {
                    j += 1;
                }
                order[i..j].reverse();
                i = j;
            }
            k -= 1;
        }
    }
    order
        .iter()
        .map(|&idx| {
            let mut cp = cps[idx];
            if lvl[idx] % 2 == 1 {
                cp = mirrored_glyph(cp);
            }
            cp
        })
        .collect()
}

/// True when `cp` has an ASCII mirrored counterpart.
pub fn is_mirrored(cp: u32) -> bool {
    matches!(
        cp,
        0x28..=0x29 | 0x3C..=0x3E | 0x5B..=0x5D | 0x7B..=0x7D | 0xFF08..=0xFF09 | 0xFF3B..=0xFF3D | 0xFF5B..=0xFF5D | 0x3008..=0x3009 | 0x3010..=0x3011 | 0x3014..=0x3015 | 0x3016..=0x3017 | 0x3018..=0x3019 | 0x301A..=0x301B
    )
}

/// Mirrored counterpart of `cp` (ASCII pairs; identity otherwise).
pub fn mirrored_glyph(cp: u32) -> u32 {
    match cp {
        0x28 => 0x29,
        0x29 => 0x28,
        0x3C => 0x3E,
        0x3E => 0x3C,
        0x5B => 0x5D,
        0x5D => 0x5B,
        0x7B => 0x7D,
        0x7D => 0x7B,
        _ => cp,
    }
}
