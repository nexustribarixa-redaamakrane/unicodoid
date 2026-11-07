//! UTS #51 Unicode Emoji sequence validation (Unicode 18.0.0, Revision 4).
//!
//! Implements the machine-readable RGI emoji sequence sets distributed in
//! `emoji-sequences.txt` and `emoji-zwj-sequences.txt`:
//!
//! * `Basic_Emoji`,
//! * `Emoji_Keycap_Sequence`,
//! * `RGI_Emoji_Modifier_Sequence`,
//! * `RGI_Emoji_Flag_Sequence`,
//! * `RGI_Emoji_Tag_Sequence`,
//! * `RGI_Emoji_ZWJ_Sequence`.
//!
//! Glyph selection, presentation and emoji-data derived properties are outside
//! this module; only sequence recognition and structural validation are
//! provided.

use crate::generated::ucd_data as data;
use crate::ucd;

/// Number of RGI emoji sequences in the generated data.
pub fn count() -> usize {
    ucd::emoji_seq_count()
}

/// RGI type name of sequence `index`, or `None` when out of range.
pub fn type_name(index: usize) -> Option<&'static str> {
    ucd::emoji_seq_type(index)
}

/// Code points of sequence `index`, or `None` when out of range.
pub fn sequence(index: usize) -> Option<&'static [u32]> {
    ucd::emoji_seq_cps(index)
}

fn valid_scalar(cp: u32) -> bool {
    cp <= 0x10FFFF && !(0xD800..=0xDFFF).contains(&cp)
}

fn is_regional_indicator(cp: u32) -> bool {
    (0x1F1E6..=0x1F1FF).contains(&cp)
}

fn is_emoji_modifier(cp: u32) -> bool {
    (0x1F3FB..=0x1F3FF).contains(&cp)
}

fn is_tag_char(cp: u32) -> bool {
    (0xE0020..=0xE007E).contains(&cp)
}

fn is_extended_pictographic(cp: u32) -> bool {
    ucd::extended_pictographic(cp)
}

fn is_emoji(cp: u32) -> bool {
    ucd::has_property(cp, data::PROP_EMOJI)
}

fn is_emoji_modifier_base(cp: u32) -> bool {
    ucd::has_property(cp, data::PROP_EMOJI_MODIFIER_BASE)
}

/// True when `cps` is an exact RGI emoji sequence; returns its RGI type name.
pub fn is_rgi(cps: &[u32]) -> Option<&'static str> {
    if cps.is_empty() || !cps.iter().all(|&cp| valid_scalar(cp)) {
        return None;
    }
    (0..count()).find_map(|index| {
        if sequence(index) == Some(cps) {
            type_name(index)
        } else {
            None
        }
    })
}

/// Structural validation independent of the RGI table.
///
/// Checks the UTS #51 shapes: keycap (base FE0F 20E3), flag (two regional
/// indicators), tag (1F3F4 tag-spec E007F), modifier (base modifier), and ZWJ
/// (Extended_Pictographic elements joined by U+200D).  This is intentionally
/// more permissive than RGI membership, which also depends on which base
/// characters have been assigned.
pub fn is_well_formed(cps: &[u32]) -> bool {
    if cps.is_empty() || !cps.iter().all(|&cp| valid_scalar(cp)) {
        return false;
    }

    // Emoji_Keycap_Sequence: [0-9#*] FE0F 20E3.
    if cps.len() == 3 && cps[1] == 0xFE0F && cps[2] == 0x20E3 {
        let base = cps[0];
        return (u32::from(b'0')..=u32::from(b'9')).contains(&base)
            || base == u32::from(b'#')
            || base == u32::from(b'*');
    }

    // RGI_Emoji_Flag_Sequence: exactly two regional indicators.
    if cps.len() == 2 && is_regional_indicator(cps[0]) && is_regional_indicator(cps[1]) {
        return true;
    }

    // RGI_Emoji_Tag_Sequence: 1F3F4 (tag chars)* E007F.
    if cps.len() >= 3 && cps[0] == 0x1F3F4 && cps[cps.len() - 1] == 0xE007F {
        return cps[1..cps.len() - 1].iter().all(|&cp| is_tag_char(cp));
    }

    // RGI_Emoji_Modifier_Sequence: modifier base + modifier.
    if cps.len() == 2 && is_emoji_modifier_base(cps[0]) && is_emoji_modifier(cps[1]) {
        return true;
    }

    // RGI_Emoji_ZWJ_Sequence: ExtPict (modifier/VS16)* (ZWJ ExtPict ...).
    if cps.contains(&0x200D) {
        if !is_extended_pictographic(cps[0]) {
            return false;
        }
        let mut i = 1usize;
        while i < cps.len() {
            if cps[i] == 0xFE0F || is_emoji_modifier(cps[i]) {
                i += 1;
                continue;
            }
            if cps[i] != 0x200D {
                return false;
            }
            i += 1;
            if i >= cps.len() || !is_extended_pictographic(cps[i]) {
                return false;
            }
            i += 1;
        }
        return true;
    }

    // Basic_Emoji: a single emoji code point, optionally followed by VS16.
    // Emoji modifiers are Basic_Emoji in their own right even though they are
    // not Extended_Pictographic.
    if cps.len() == 1 || (cps.len() == 2 && cps[1] == 0xFE0F) {
        let cp = cps[0];
        if is_extended_pictographic(cp) || is_emoji(cp) {
            return true;
        }
    }

    false
}
