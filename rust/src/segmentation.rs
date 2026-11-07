//! Unicode text segmentation (UAX #29 and UAX #14).
//!
//! Grapheme cluster, word, line, and sentence boundaries are computed from the
//! generated Unicode 18.0.0 property tables exposed by [`crate::ucd`].  The
//! implementation follows the default rule sets:
//!
//! * grapheme clusters: UAX #29 GB1–GB13 / GB999,
//! * words: UAX #29 WB1–WB16 / WB999 (with WB4 folding),
//! * sentences: UAX #29 SB1–SB11 / SB998 (with SB5 folding),
//! * lines: UAX #14 LB1–LB31.
//!
//! The module requires an encoding system with a Unicode profile.

use crate::codec::Codec;
use crate::system::EncodingSystem;
use crate::types::{DecodeOutcome, Status};
use crate::ucd;

/// Boundary types supported by the segmenter.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
#[repr(u8)]
pub enum BoundaryType {
    Grapheme = 0,
    Word = 1,
    Line = 2,
    Sentence = 3,
}

/// One segment of a byte stream.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct Segment {
    pub byte_offset: usize,
    pub byte_length: usize,
    pub char_count: usize,
    pub is_boundary: bool,
}

impl Segment {
    pub fn end_offset(&self) -> usize {
        self.byte_offset + self.byte_length
    }
}

/// Iterator over the segments of one byte stream.
pub struct Segmenter<'a> {
    system: &'a dyn EncodingSystem,
    boundary_type: BoundaryType,
    scalars: Vec<u32>,
    offsets: Vec<usize>,
    index: usize,
}

impl<'a> Segmenter<'a> {
    /// Prepare a segmenter, decoding the whole input into scalar values.
    pub fn new(
        system: &'a dyn EncodingSystem,
        data: &[u8],
        boundary_type: BoundaryType,
    ) -> Result<Self, Status> {
        if !system.has_unicode_profile() {
            return Err(Status::Unsupported);
        }
        let profile = system.unicode_profile().ok_or(Status::Unsupported)?;

        let mut codec = Codec::new(system);
        let mut scalars = Vec::new();
        let mut offsets = Vec::new();
        let mut offset = 0usize;

        while offset < data.len() {
            match codec.decode_char(&data[offset..])? {
                DecodeOutcome::Char { ch, read } => {
                    if read == 0 {
                        return Err(Status::Internal);
                    }
                    let cp = profile.try_to_unicode(ch).ok_or(Status::UnmappableChar)?;
                    offsets.push(offset);
                    scalars.push(cp);
                    offset += read;
                }
                DecodeOutcome::NoCharacter { read } => {
                    if read == 0 {
                        return Err(Status::Internal);
                    }
                    offset += read;
                }
            }
        }
        offsets.push(data.len());

        Ok(Self {
            system,
            boundary_type,
            scalars,
            offsets,
            index: 0,
        })
    }

    /// Encoding system used by this segmenter.
    pub fn system(&self) -> &'a dyn EncodingSystem {
        self.system
    }

    pub fn boundary_type(&self) -> BoundaryType {
        self.boundary_type
    }

    /// Number of scalar values in the decoded input.
    pub fn scalar_count(&self) -> usize {
        self.scalars.len()
    }

    /// Retrieve the next segment, or `None` when the input is exhausted.
    pub fn next_segment(&mut self) -> Option<Segment> {
        if self.index >= self.scalars.len() {
            return None;
        }

        let start = self.index;
        let end = match self.boundary_type {
            BoundaryType::Grapheme => next_grapheme(&self.scalars, start),
            BoundaryType::Word => next_word(&self.scalars, start),
            BoundaryType::Line => next_line(&self.scalars, start),
            BoundaryType::Sentence => next_sentence(&self.scalars, start),
        };

        let end = end.max(start + 1).min(self.scalars.len());
        let byte_start = self.offsets[start];
        let byte_end = self.offsets[end];

        self.index = end;

        Some(Segment {
            byte_offset: byte_start,
            byte_length: byte_end - byte_start,
            char_count: end - start,
            is_boundary: true,
        })
    }
}

impl Iterator for Segmenter<'_> {
    type Item = Segment;

    fn next(&mut self) -> Option<Self::Item> {
        self.next_segment()
    }
}

/* ------------------------------------------------------------------------- */
/* UAX #29 grapheme clusters (Unicode 18.0.0)                               */
/* ------------------------------------------------------------------------- */

fn seg_is_control(cp: u32) -> bool {
    let g = ucd::grapheme_break(cp);
    g == ucd::gcb::CONTROL || g == ucd::gcb::CR || g == ucd::gcb::LF
}

/// Scalar-level extended grapheme cluster primitive (UAX #29).
///
/// Returns the index one past the last scalar of the cluster beginning at
/// `start`, or `scalars.len()` when `start` is out of range.
pub fn next_grapheme_cluster(scalars: &[u32], start: usize) -> usize {
    if start >= scalars.len() {
        return scalars.len();
    }
    next_grapheme(scalars, start)
}

fn next_grapheme(scalars: &[u32], start: usize) -> usize {
    let mut i = start + 1;
    if i > scalars.len() {
        return scalars.len();
    }

    let mut prev = scalars[start];
    let mut prev_g = ucd::grapheme_break(prev);
    let mut prev_ri_pos: Option<usize> = if prev_g == ucd::gcb::RI { Some(0) } else { None };
    let mut emoji_context = ucd::extended_pictographic(prev);
    let mut emoji_zwj_pending = false;
    // GB9c (Unicode 18.0.0): ConjunctLinker ConjunctExtender* x LinkingConsonant.
    let mut in_linker_conjunct = ucd::indic_conjunct_break(prev) == ucd::incb::LINKER;

    while i < scalars.len() {
        let cur = scalars[i];
        let cur_g = ucd::grapheme_break(cur);
        let cur_incb = ucd::indic_conjunct_break(cur);
        let mut boundary = true;

        if prev == 0x000D && cur == 0x000A {
            // GB3: CR x LF.
            boundary = false;
        } else if seg_is_control(prev) || seg_is_control(cur) {
            // GB4 / GB5: break before and after controls.
            boundary = true;
        } else if prev_g == ucd::gcb::L
            && (cur_g == ucd::gcb::L
                || cur_g == ucd::gcb::V
                || cur_g == ucd::gcb::LV
                || cur_g == ucd::gcb::LVT)
        {
            // GB6: L x (L | V | LV | LVT).
            boundary = false;
        } else if (prev_g == ucd::gcb::LV || prev_g == ucd::gcb::V)
            && (cur_g == ucd::gcb::V || cur_g == ucd::gcb::T)
        {
            // GB7: (LV | V) x (V | T).
            boundary = false;
        } else if (prev_g == ucd::gcb::LVT || prev_g == ucd::gcb::T) && cur_g == ucd::gcb::T {
            // GB8: (LVT | T) x T.
            boundary = false;
        } else if cur_g == ucd::gcb::EXTEND || cur_g == ucd::gcb::ZWJ {
            // GB9: x (Extend | ZWJ).
            boundary = false;
        } else if cur_g == ucd::gcb::SPACINGMARK {
            // GB9a: x SpacingMark.
            boundary = false;
        } else if prev_g == ucd::gcb::PREPEND {
            // GB9b: Prepend x.
            boundary = false;
        } else if cur_incb == ucd::incb::CONSONANT && in_linker_conjunct {
            // GB9c: InCB=Linker InCB=Extend* x InCB=Consonant.
            boundary = false;
        } else if emoji_zwj_pending && ucd::extended_pictographic(cur) {
            // GB11: ExtPict Extend* ZWJ x ExtPict.
            boundary = false;
        } else if cur_g == ucd::gcb::RI {
            // GB12 / GB13: regional indicator pairing.
            let cur_ri_pos = if prev_g == ucd::gcb::RI {
                prev_ri_pos.map_or(0, |p| p + 1)
            } else {
                0
            };
            if cur_ri_pos % 2 == 1 {
                boundary = false;
            }
        }

        // GB999: Any / Any (default break).
        if boundary {
            return i;
        }

        // Consume the current character and update cluster context.
        if cur_g == ucd::gcb::RI {
            prev_ri_pos = Some(if prev_g == ucd::gcb::RI {
                prev_ri_pos.map_or(0, |p| p + 1)
            } else {
                0
            });
        } else {
            prev_ri_pos = None;
        }

        if cur == 0x200D {
            if emoji_context {
                emoji_zwj_pending = true;
            }
        } else if cur_g == ucd::gcb::EXTEND {
            // Extends stay inside the current emoji context.
        } else {
            emoji_context = ucd::extended_pictographic(cur);
            emoji_zwj_pending = false;
        }

        if cur_incb == ucd::incb::LINKER {
            in_linker_conjunct = true;
        } else if cur_incb == ucd::incb::EXTEND {
            // ConjunctExtender* keeps the linker context alive.
        } else {
            in_linker_conjunct = false;
        }

        prev = cur;
        prev_g = cur_g;
        i += 1;
    }
    scalars.len()
}

/* ------------------------------------------------------------------------- */
/* UAX #29 word breaking (Unicode 18.0.0)                                   */
/*                                                                          */
/* WB4 remaps every Extend / Format / ZWJ following a character that is not */
/* CR, LF, or Newline to that base character and then ignores it.  This is  */
/* modelled by folding the base and its following Extend / Format / ZWJ run */
/* into one unit.  All remaining rules are evaluated between units.         */
/* ------------------------------------------------------------------------- */

#[derive(Debug, Clone, Copy)]
struct WbUnit {
    start: usize,
    end: usize,
    cls: u8,
    last_cls: u8,
    ends_with_zwj: bool,
    extpict: bool,
}

fn wb_is_newline(cls: u8) -> bool {
    cls == ucd::wb::CR || cls == ucd::wb::LF || cls == ucd::wb::NEWLINE
}

fn wb_is_ahletter(cls: u8) -> bool {
    cls == ucd::wb::ALETTER || cls == ucd::wb::HEBREW_LETTER
}

/// MidLetter | MidNumLetQ (MidNumLet | Single_Quote).
fn wb_is_midletterq(cls: u8) -> bool {
    cls == ucd::wb::MIDLETTER || cls == ucd::wb::MIDNUMLET || cls == ucd::wb::SINGLE_QUOTE
}

/// MidNum | MidNumLetQ (MidNumLet | Single_Quote).
fn wb_is_midnumq(cls: u8) -> bool {
    cls == ucd::wb::MIDNUM || cls == ucd::wb::MIDNUMLET || cls == ucd::wb::SINGLE_QUOTE
}

fn wb_build_units(scalars: &[u32]) -> Vec<WbUnit> {
    let mut units = Vec::with_capacity(scalars.len());
    let mut i = 0;
    while i < scalars.len() {
        let cls = ucd::word_break(scalars[i]);
        let mut unit = WbUnit {
            start: i,
            end: i + 1,
            cls,
            last_cls: cls,
            ends_with_zwj: false,
            extpict: ucd::extended_pictographic(scalars[i]),
        };

        let mut j = i + 1;
        if !wb_is_newline(cls) {
            while j < scalars.len() {
                let c2 = ucd::word_break(scalars[j]);
                if c2 != ucd::wb::EXTEND && c2 != ucd::wb::FORMAT && c2 != ucd::wb::ZWJ {
                    break;
                }
                unit.last_cls = c2;
                j += 1;
            }
        }
        unit.ends_with_zwj = unit.last_cls == ucd::wb::ZWJ;
        unit.end = j;

        units.push(unit);
        i = j;
    }
    units
}

fn wb_break_before(u: &[WbUnit], i: usize) -> bool {
    if i == 0 {
        return false; // WB1 (leading sot break is implicit)
    }
    if i >= u.len() {
        return true; // WB2: / eot
    }

    let prev = &u[i - 1];
    let cur = &u[i];
    let p = prev.cls;
    let c = cur.cls;

    // WB3: CR x LF
    if p == ucd::wb::CR && c == ucd::wb::LF {
        return false;
    }
    // WB3.1: (Newline | CR | LF) /
    if wb_is_newline(p) {
        return true;
    }
    // WB3.2: / (Newline | CR | LF)
    if wb_is_newline(c) {
        return true;
    }
    // WB3.3: ZWJ x ExtPict
    if prev.ends_with_zwj && cur.extpict {
        return false;
    }
    // WB3.4: WSegSpace x WSegSpace (evaluated on raw characters, i.e. before
    // WB4 folds any following Extend / Format / ZWJ into the space).
    if prev.last_cls == ucd::wb::WSEGSPACE && c == ucd::wb::WSEGSPACE {
        return false;
    }
    // WB5: AHLetter x AHLetter
    if wb_is_ahletter(p) && wb_is_ahletter(c) {
        return false;
    }
    // WB6: AHLetter x (MidLetter | MidNumLetQ) AHLetter
    if wb_is_ahletter(p) && wb_is_midletterq(c) && i + 1 < u.len() && wb_is_ahletter(u[i + 1].cls)
    {
        return false;
    }
    // WB7: AHLetter (MidLetter | MidNumLetQ) x AHLetter
    if wb_is_midletterq(p) && wb_is_ahletter(c) && i >= 2 && wb_is_ahletter(u[i - 2].cls) {
        return false;
    }
    // WB7.1: Hebrew_Letter x Single_Quote
    if p == ucd::wb::HEBREW_LETTER && c == ucd::wb::SINGLE_QUOTE {
        return false;
    }
    // WB7.2: Hebrew_Letter x Double_Quote Hebrew_Letter
    if p == ucd::wb::HEBREW_LETTER
        && c == ucd::wb::DOUBLE_QUOTE
        && i + 1 < u.len()
        && u[i + 1].cls == ucd::wb::HEBREW_LETTER
    {
        return false;
    }
    // WB7.3: Hebrew_Letter Double_Quote x Hebrew_Letter
    if p == ucd::wb::DOUBLE_QUOTE
        && c == ucd::wb::HEBREW_LETTER
        && i >= 2
        && u[i - 2].cls == ucd::wb::HEBREW_LETTER
    {
        return false;
    }
    // WB8: Numeric x Numeric
    if p == ucd::wb::NUMERIC && c == ucd::wb::NUMERIC {
        return false;
    }
    // WB9: AHLetter x Numeric
    if wb_is_ahletter(p) && c == ucd::wb::NUMERIC {
        return false;
    }
    // WB10: Numeric x AHLetter
    if p == ucd::wb::NUMERIC && wb_is_ahletter(c) {
        return false;
    }
    // WB11: Numeric (MidNum | MidNumLetQ) x Numeric
    if wb_is_midnumq(p) && c == ucd::wb::NUMERIC && i >= 2 && u[i - 2].cls == ucd::wb::NUMERIC {
        return false;
    }
    // WB12: Numeric x (MidNum | MidNumLetQ) Numeric
    if p == ucd::wb::NUMERIC
        && wb_is_midnumq(c)
        && i + 1 < u.len()
        && u[i + 1].cls == ucd::wb::NUMERIC
    {
        return false;
    }
    // WB13: Katakana x Katakana
    if p == ucd::wb::KATAKANA && c == ucd::wb::KATAKANA {
        return false;
    }
    // WB13.1: (AHLetter | Numeric | Katakana | ExtendNumLet) x ExtendNumLet
    if (wb_is_ahletter(p)
        || p == ucd::wb::NUMERIC
        || p == ucd::wb::KATAKANA
        || p == ucd::wb::EXTENDNUMLET)
        && c == ucd::wb::EXTENDNUMLET
    {
        return false;
    }
    // WB13.2: ExtendNumLet x (AHLetter | Numeric | Katakana)
    if p == ucd::wb::EXTENDNUMLET
        && (wb_is_ahletter(c) || c == ucd::wb::NUMERIC || c == ucd::wb::KATAKANA)
    {
        return false;
    }
    // WB15 / WB16: regional indicator pairs
    if p == ucd::wb::RI && c == ucd::wb::RI {
        let mut count = 0usize;
        let mut k = i;
        while k > 0 && u[k - 1].cls == ucd::wb::RI {
            count += 1;
            k -= 1;
        }
        if count % 2 == 1 {
            return false;
        }
    }
    // WB999: Any / Any
    true
}

fn next_word(scalars: &[u32], start: usize) -> usize {
    if scalars.is_empty() {
        return 0;
    }

    let units = wb_build_units(scalars);

    let mut first = units.len();
    for (k, unit) in units.iter().enumerate() {
        if unit.start == start {
            first = k;
            break;
        }
    }

    let mut end = scalars.len();
    let mut k = first + 1;
    while k <= units.len() {
        if wb_break_before(&units, k) {
            end = if k < units.len() {
                units[k].start
            } else {
                scalars.len()
            };
            break;
        }
        k += 1;
    }

    if end <= start {
        end = start + 1;
    }
    end.min(scalars.len())
}

/* ------------------------------------------------------------------------- */
/* UAX #14 line breaking (Unicode 18.0.0, revision 57)                      */
/*                                                                          */
/* LB9 is applied by folding every base character together with the         */
/* following CM / ZWJ run into a single "unit".  Remaining CM / ZWJ         */
/* characters (LB10) become one-character AL units.  All line break rules   */
/* are then evaluated between units, which is exactly the "ignore affected  */
/* CM / ZWJ" requirement of LB9.                                            */
/* ------------------------------------------------------------------------- */

#[derive(Debug, Clone, Copy)]
struct LbUnit {
    start: usize,
    end: usize,
    cls: u8,
    gc: u8,
    east_asian: bool,
    dotted: bool,
    extpict_cn: bool,
    ends_with_zwj: bool,
}

/// LB1: resolve AI, SG, XX, SA, and CJ to their working classes.
fn lb_resolve_class(cp: u32) -> u8 {
    let lb = ucd::line_break(cp);
    let gc = ucd::general_category(cp);

    match lb {
        ucd::lb::AI | ucd::lb::SG | ucd::lb::XX => ucd::lb::AL,
        ucd::lb::SA => {
            if gc == ucd::gc::MN || gc == ucd::gc::MC {
                ucd::lb::CM
            } else {
                ucd::lb::AL
            }
        }
        ucd::lb::CJ => ucd::lb::NS,
        _ => lb,
    }
}

fn lb_is_excluded_from_lb9(cls: u8) -> bool {
    cls == ucd::lb::BK
        || cls == ucd::lb::CR
        || cls == ucd::lb::LF
        || cls == ucd::lb::NL
        || cls == ucd::lb::SP
        || cls == ucd::lb::ZW
}

fn lb_build_units(scalars: &[u32]) -> Vec<LbUnit> {
    let mut units = Vec::with_capacity(scalars.len());
    let mut i = 0;
    while i < scalars.len() {
        let cp = scalars[i];
        let cls = lb_resolve_class(cp);
        let gc = ucd::general_category(cp);
        let eaw = ucd::east_asian_width(cp);
        let mut unit = LbUnit {
            start: i,
            end: i + 1,
            cls,
            gc,
            east_asian: eaw == ucd::eaw::H || eaw == ucd::eaw::F || eaw == ucd::eaw::W,
            dotted: cp == 0x25CC,
            extpict_cn: gc == ucd::gc::CN && ucd::extended_pictographic(cp),
            ends_with_zwj: false,
        };

        let mut j = i + 1;
        if cls == ucd::lb::CM || cls == ucd::lb::ZWJ {
            // LB10: a remaining combining mark / ZWJ behaves like AL.
            // Following CM / ZWJ characters are also "remaining", so they
            // form their own AL units (the resulting break decisions are
            // identical because AL x AL is prohibited by LB28).
            unit.cls = ucd::lb::AL;
            unit.ends_with_zwj = cls == ucd::lb::ZWJ;
        } else if lb_is_excluded_from_lb9(cls) {
            // LB9 does not absorb CM / ZWJ after these classes.
            unit.cls = cls;
        } else {
            // LB9: treat X (CM | ZWJ)* as X.
            unit.cls = cls;
            let mut last_zwj = false;
            while j < scalars.len() {
                let c2cls = lb_resolve_class(scalars[j]);
                if c2cls != ucd::lb::CM && c2cls != ucd::lb::ZWJ {
                    break;
                }
                last_zwj = c2cls == ucd::lb::ZWJ;
                j += 1;
            }
            unit.ends_with_zwj = last_zwj;
        }

        unit.end = j;
        units.push(unit);
        i = j;
    }
    units
}

/// True when the units immediately before index `k` end with NU (SY | IS)*.
fn lb_nu_seq_before(u: &[LbUnit], k: usize) -> bool {
    let mut j = k;
    while j > 0 && (u[j - 1].cls == ucd::lb::SY || u[j - 1].cls == ucd::lb::IS) {
        j -= 1;
    }
    j > 0 && u[j - 1].cls == ucd::lb::NU
}

/// Decide whether a line break is allowed immediately before unit `i`.
fn lb_break_before(u: &[LbUnit], i: usize) -> bool {
    if i == 0 {
        return false; // LB2: sot x
    }
    if i >= u.len() {
        return true; // LB3: ! eot
    }

    let prev = &u[i - 1];
    let cur = &u[i];
    let p = prev.cls;
    let c = cur.cls;

    // LB4: BK !
    if p == ucd::lb::BK {
        return true;
    }
    // LB5: CR x LF, then CR / LF / NL !
    if p == ucd::lb::CR && c == ucd::lb::LF {
        return false;
    }
    if p == ucd::lb::CR || p == ucd::lb::LF || p == ucd::lb::NL {
        return true;
    }
    // LB6: x (BK | CR | LF | NL)
    if c == ucd::lb::BK || c == ucd::lb::CR || c == ucd::lb::LF || c == ucd::lb::NL {
        return false;
    }
    // LB7: x SP, x ZW
    if c == ucd::lb::SP || c == ucd::lb::ZW {
        return false;
    }
    // LB8: ZW SP* !
    {
        let mut k = i;
        while k > 0 && u[k - 1].cls == ucd::lb::SP {
            k -= 1;
        }
        if k > 0 && u[k - 1].cls == ucd::lb::ZW {
            return true;
        }
    }
    // LB8a: ZWJ x
    if prev.ends_with_zwj {
        return false;
    }
    // LB11: x WJ, WJ x
    if c == ucd::lb::WJ || p == ucd::lb::WJ {
        return false;
    }
    // LB12: GL x
    if p == ucd::lb::GL {
        return false;
    }
    // LB12a: [^SP HY HH] x GL
    if c == ucd::lb::GL && p != ucd::lb::SP && p != ucd::lb::HY && p != ucd::lb::HH {
        return false;
    }
    // LB13: x CL, x CP, x EX, x SY
    if c == ucd::lb::CL || c == ucd::lb::CP || c == ucd::lb::EX || c == ucd::lb::SY {
        return false;
    }
    // LB14: OP SP* x
    {
        let mut k = i;
        while k > 0 && u[k - 1].cls == ucd::lb::SP {
            k -= 1;
        }
        if k > 0 && u[k - 1].cls == ucd::lb::OP {
            return false;
        }
    }
    // LB15a: (sot | BK | CR | LF | NL | OP | QU | GL | SP | ZW) QU_Pi SP* x
    {
        let mut k = i;
        while k > 0 && u[k - 1].cls == ucd::lb::SP {
            k -= 1;
        }
        if k > 0 && u[k - 1].cls == ucd::lb::QU && u[k - 1].gc == ucd::gc::PI {
            if k - 1 == 0 {
                return false; // sot QU_Pi SP* x
            }
            let pc = u[k - 2].cls;
            if pc == ucd::lb::BK
                || pc == ucd::lb::CR
                || pc == ucd::lb::LF
                || pc == ucd::lb::NL
                || pc == ucd::lb::OP
                || pc == ucd::lb::QU
                || pc == ucd::lb::GL
                || pc == ucd::lb::SP
                || pc == ucd::lb::ZW
            {
                return false;
            }
        }
    }
    // LB15b: x QU_Pf (SP | GL | WJ | CL | QU | CP | EX | IS | SY | BK | CR | LF | NL | ZW | eot)
    if c == ucd::lb::QU && cur.gc == ucd::gc::PF {
        if i + 1 >= u.len() {
            return false;
        }
        let nc = u[i + 1].cls;
        if nc == ucd::lb::SP
            || nc == ucd::lb::GL
            || nc == ucd::lb::WJ
            || nc == ucd::lb::CL
            || nc == ucd::lb::QU
            || nc == ucd::lb::CP
            || nc == ucd::lb::EX
            || nc == ucd::lb::IS
            || nc == ucd::lb::SY
            || nc == ucd::lb::BK
            || nc == ucd::lb::CR
            || nc == ucd::lb::LF
            || nc == ucd::lb::NL
            || nc == ucd::lb::ZW
        {
            return false;
        }
    }
    // LB15c: SP ! IS NU
    if p == ucd::lb::SP && c == ucd::lb::IS && i + 1 < u.len() && u[i + 1].cls == ucd::lb::NU {
        return true;
    }
    // LB15d: x IS
    if c == ucd::lb::IS {
        return false;
    }
    // LB16: (CL | CP) SP* x NS
    if c == ucd::lb::NS {
        let mut k = i;
        while k > 0 && u[k - 1].cls == ucd::lb::SP {
            k -= 1;
        }
        if k > 0 && (u[k - 1].cls == ucd::lb::CL || u[k - 1].cls == ucd::lb::CP) {
            return false;
        }
    }
    // LB17: B2 SP* x B2
    if c == ucd::lb::B2 {
        let mut k = i;
        while k > 0 && u[k - 1].cls == ucd::lb::SP {
            k -= 1;
        }
        if k > 0 && u[k - 1].cls == ucd::lb::B2 {
            return false;
        }
    }
    // LB18: SP !
    if p == ucd::lb::SP {
        return true;
    }
    // LB19: x QUmPi, QUmPf x
    if c == ucd::lb::QU && cur.gc != ucd::gc::PI {
        return false;
    }
    if p == ucd::lb::QU && prev.gc != ucd::gc::PF {
        return false;
    }
    // LB19a: East Asian quotation tailoring
    if c == ucd::lb::QU {
        if !prev.east_asian {
            return false; // [^EA] x QU
        }
        if i + 1 >= u.len() {
            return false; // x QU eot
        }
        if !u[i + 1].east_asian {
            return false; // x QU [^EA]
        }
    }
    if p == ucd::lb::QU && !cur.east_asian {
        return false; // QU x [^EA]
    }
    if p == ucd::lb::QU {
        // (sot | [^EA]) QU x
        if i - 1 == 0 {
            return false;
        }
        if !u[i - 2].east_asian {
            return false;
        }
    }
    // LB20: ! CB, CB !
    if c == ucd::lb::CB || p == ucd::lb::CB {
        return true;
    }
    // LB20a: (sot | BK | CR | LF | NL | SP | ZW | CB | GL) (HY | HH) x (AL | HL)
    if (p == ucd::lb::HY || p == ucd::lb::HH)
        && (c == ucd::lb::AL || c == ucd::lb::HL)
    {
        if i - 1 == 0 {
            return false;
        }
        let pc = u[i - 2].cls;
        if pc == ucd::lb::BK
            || pc == ucd::lb::CR
            || pc == ucd::lb::LF
            || pc == ucd::lb::NL
            || pc == ucd::lb::SP
            || pc == ucd::lb::ZW
            || pc == ucd::lb::CB
            || pc == ucd::lb::GL
        {
            return false;
        }
    }
    // LB21: x BA, x HH, x HY, x NS, BB x
    if c == ucd::lb::BA || c == ucd::lb::HH || c == ucd::lb::HY || c == ucd::lb::NS {
        return false;
    }
    if p == ucd::lb::BB {
        return false;
    }
    // LB21a: HL (HY | HH) x [^HL]
    if (p == ucd::lb::HY || p == ucd::lb::HH)
        && i >= 2
        && u[i - 2].cls == ucd::lb::HL
        && c != ucd::lb::HL
    {
        return false;
    }
    // LB21b: SY x HL
    if p == ucd::lb::SY && c == ucd::lb::HL {
        return false;
    }
    // LB22: x IN
    if c == ucd::lb::IN {
        return false;
    }
    // LB23: (AL | HL) x NU, NU x (AL | HL)
    if (p == ucd::lb::AL || p == ucd::lb::HL) && c == ucd::lb::NU {
        return false;
    }
    if p == ucd::lb::NU && (c == ucd::lb::AL || c == ucd::lb::HL) {
        return false;
    }
    // LB23a: PR x (ID | EB | EM), (ID | EB | EM) x PO
    if p == ucd::lb::PR && (c == ucd::lb::ID || c == ucd::lb::EB || c == ucd::lb::EM) {
        return false;
    }
    if (p == ucd::lb::ID || p == ucd::lb::EB || p == ucd::lb::EM) && c == ucd::lb::PO {
        return false;
    }
    // LB24: (PR | PO) x (AL | HL), (AL | HL) x (PR | PO)
    if (p == ucd::lb::PR || p == ucd::lb::PO) && (c == ucd::lb::AL || c == ucd::lb::HL) {
        return false;
    }
    if (p == ucd::lb::AL || p == ucd::lb::HL) && (c == ucd::lb::PR || c == ucd::lb::PO) {
        return false;
    }
    // LB25: numeric sequences
    if c == ucd::lb::PO || c == ucd::lb::PR {
        if (p == ucd::lb::CL || p == ucd::lb::CP) && lb_nu_seq_before(u, i - 1) {
            return false;
        }
        if lb_nu_seq_before(u, i) {
            return false;
        }
    }
    if p == ucd::lb::PO || p == ucd::lb::PR {
        if c == ucd::lb::OP {
            if i + 1 < u.len() && u[i + 1].cls == ucd::lb::NU {
                return false;
            }
            if i + 2 < u.len()
                && u[i + 1].cls == ucd::lb::IS
                && u[i + 2].cls == ucd::lb::NU
            {
                return false;
            }
        }
        if c == ucd::lb::NU {
            return false;
        }
    }
    if p == ucd::lb::HY && c == ucd::lb::NU {
        return false;
    }
    if p == ucd::lb::IS && c == ucd::lb::NU {
        return false;
    }
    if c == ucd::lb::NU && lb_nu_seq_before(u, i) {
        return false;
    }
    // LB26: Korean syllables
    if p == ucd::lb::JL
        && (c == ucd::lb::JL
            || c == ucd::lb::JV
            || c == ucd::lb::H2
            || c == ucd::lb::H3)
    {
        return false;
    }
    if (p == ucd::lb::JV || p == ucd::lb::H2) && (c == ucd::lb::JV || c == ucd::lb::JT) {
        return false;
    }
    if (p == ucd::lb::JT || p == ucd::lb::H3) && c == ucd::lb::JT {
        return false;
    }
    // LB27: Korean syllable blocks behave like ID
    if (p == ucd::lb::JL
        || p == ucd::lb::JV
        || p == ucd::lb::JT
        || p == ucd::lb::H2
        || p == ucd::lb::H3)
        && c == ucd::lb::PO
    {
        return false;
    }
    if p == ucd::lb::PR
        && (c == ucd::lb::JL
            || c == ucd::lb::JV
            || c == ucd::lb::JT
            || c == ucd::lb::H2
            || c == ucd::lb::H3)
    {
        return false;
    }
    // LB28: (AL | HL) x (AL | HL)
    if (p == ucd::lb::AL || p == ucd::lb::HL) && (c == ucd::lb::AL || c == ucd::lb::HL) {
        return false;
    }
    // LB28a: Brahmic orthographic syllables
    {
        let prev_ak = p == ucd::lb::AK || prev.dotted || p == ucd::lb::AS;
        let cur_ak = c == ucd::lb::AK || cur.dotted || c == ucd::lb::AS;
        if p == ucd::lb::AP && cur_ak {
            return false;
        }
        if prev_ak && (c == ucd::lb::VF || c == ucd::lb::VI) {
            return false;
        }
        if p == ucd::lb::VI && (c == ucd::lb::AK || cur.dotted) && i >= 2 {
            let p2 = &u[i - 2];
            if p2.cls == ucd::lb::AK || p2.dotted || p2.cls == ucd::lb::AS {
                return false;
            }
        }
        if prev_ak && cur_ak && i + 1 < u.len() && u[i + 1].cls == ucd::lb::VF {
            return false;
        }
    }
    // LB29: IS x (AL | HL)
    if p == ucd::lb::IS && (c == ucd::lb::AL || c == ucd::lb::HL) {
        return false;
    }
    // LB30: (AL | HL | NU) x OPmEastAsian, CPmEastAsian x (AL | HL | NU)
    if (p == ucd::lb::AL || p == ucd::lb::HL || p == ucd::lb::NU)
        && c == ucd::lb::OP
        && !cur.east_asian
    {
        return false;
    }
    if p == ucd::lb::CP
        && !prev.east_asian
        && (c == ucd::lb::AL || c == ucd::lb::HL || c == ucd::lb::NU)
    {
        return false;
    }
    // LB30a: regional indicator pairs
    if p == ucd::lb::RI && c == ucd::lb::RI {
        let mut count = 0usize;
        let mut k = i;
        while k > 0 && u[k - 1].cls == ucd::lb::RI {
            count += 1;
            k -= 1;
        }
        if count % 2 == 1 {
            return false;
        }
    }
    // LB30b: EB x EM, ExtPictUnassigned x EM
    if c == ucd::lb::EM && (p == ucd::lb::EB || prev.extpict_cn) {
        return false;
    }
    // LB31: break everywhere else.
    true
}

fn next_line(scalars: &[u32], start: usize) -> usize {
    if scalars.is_empty() {
        return 0;
    }

    let units = lb_build_units(scalars);

    let mut first = units.len();
    for (k, unit) in units.iter().enumerate() {
        if unit.start == start {
            first = k;
            break;
        }
    }

    let mut end = scalars.len();
    let mut k = first + 1;
    while k <= units.len() {
        if lb_break_before(&units, k) {
            end = if k < units.len() {
                units[k].start
            } else {
                scalars.len()
            };
            break;
        }
        k += 1;
    }

    if end <= start {
        end = start + 1;
    }
    end.min(scalars.len())
}

/* ------------------------------------------------------------------------- */
/* UAX #29 sentence breaking (Unicode 18.0.0)                               */
/*                                                                          */
/* SB5 ignores Extend / Format characters following a base character that   */
/* is not a paragraph separator.  Folding them into the base makes the SB   */
/* rules directly expressible between units.                                */
/* ------------------------------------------------------------------------- */

#[derive(Debug, Clone, Copy)]
struct SbUnit {
    start: usize,
    end: usize,
    cls: u8,
}

fn sb_is_para(cls: u8) -> bool {
    cls == ucd::sb::SEP || cls == ucd::sb::CR || cls == ucd::sb::LF
}

fn sb_is_saterm(cls: u8) -> bool {
    cls == ucd::sb::STERM || cls == ucd::sb::ATERM
}

fn sb_build_units(scalars: &[u32]) -> Vec<SbUnit> {
    let mut units = Vec::with_capacity(scalars.len());
    let mut i = 0;
    while i < scalars.len() {
        let cls = ucd::sentence_break(scalars[i]);
        let mut unit = SbUnit {
            start: i,
            end: i + 1,
            cls,
        };

        let mut j = i + 1;
        if !sb_is_para(cls) {
            // SB5: X (Extend | Format)* -> X.
            while j < scalars.len() {
                let c2 = ucd::sentence_break(scalars[j]);
                if c2 != ucd::sb::EXTEND && c2 != ucd::sb::FORMAT {
                    break;
                }
                j += 1;
            }
        }

        unit.end = j;
        units.push(unit);
        i = j;
    }
    units
}

/// Scan the left context ending immediately before unit `i` and test for
/// `SATerm Close*` (`allow_sp == false`) or `SATerm Close* Sp*` (`allow_sp`).
fn sb_scan_saterm(u: &[SbUnit], i: usize, allow_sp: bool, aterm_only: bool) -> bool {
    let mut k = i;
    if allow_sp {
        while k > 0 && u[k - 1].cls == ucd::sb::SP {
            k -= 1;
        }
    }
    while k > 0 && u[k - 1].cls == ucd::sb::CLOSE {
        k -= 1;
    }
    if k == 0 {
        return false;
    }
    let t = u[k - 1].cls;
    if aterm_only {
        return t == ucd::sb::ATERM;
    }
    sb_is_saterm(t)
}

/// SB8 right-hand side: `(not (OLetter | Upper | Lower | ParaSep | SATerm))* Lower`.
fn sb8_lookahead(u: &[SbUnit], i: usize) -> bool {
    let mut k = i;
    while k < u.len() {
        let c = u[k].cls;
        if c == ucd::sb::OLETTER
            || c == ucd::sb::UPPER
            || c == ucd::sb::LOWER
            || sb_is_para(c)
            || sb_is_saterm(c)
        {
            break;
        }
        k += 1;
    }
    k < u.len() && u[k].cls == ucd::sb::LOWER
}

fn sb_break_before(u: &[SbUnit], i: usize) -> bool {
    if i == 0 {
        return false; // SB1
    }
    if i >= u.len() {
        return true; // SB2: Any / eot
    }

    let prev = &u[i - 1];
    let cur = &u[i];
    let p = prev.cls;
    let c = cur.cls;

    // SB3: CR x LF
    if p == ucd::sb::CR && c == ucd::sb::LF {
        return false;
    }
    // SB4: ParaSep /
    if sb_is_para(p) {
        return true;
    }
    // SB6: ATerm x Numeric
    if p == ucd::sb::ATERM && c == ucd::sb::NUMERIC {
        return false;
    }
    // SB7: (Upper | Lower) ATerm x Upper
    if p == ucd::sb::ATERM && c == ucd::sb::UPPER && i >= 2 {
        let p2 = u[i - 2].cls;
        if p2 == ucd::sb::UPPER || p2 == ucd::sb::LOWER {
            return false;
        }
    }
    // SB8: ATerm Close* Sp* x (not (OLetter | Upper | Lower | ParaSep | SATerm))* Lower
    if sb_scan_saterm(u, i, true, true) && sb8_lookahead(u, i) {
        return false;
    }
    // SB8a: SATerm Close* Sp* x (SContinue | SATerm)
    if (c == ucd::sb::SCONTINUE || sb_is_saterm(c)) && sb_scan_saterm(u, i, true, false) {
        return false;
    }
    // SB9: SATerm Close* x (Close | Sp | ParaSep)
    if (c == ucd::sb::CLOSE || c == ucd::sb::SP || sb_is_para(c))
        && sb_scan_saterm(u, i, false, false)
    {
        return false;
    }
    // SB10: SATerm Close* Sp* x (Sp | ParaSep)
    if (c == ucd::sb::SP || sb_is_para(c)) && sb_scan_saterm(u, i, true, false) {
        return false;
    }
    // SB11: SATerm Close* Sp* ParaSep? /
    {
        let mut k = i;
        if k > 0 && sb_is_para(u[k - 1].cls) {
            k -= 1;
        }
        while k > 0 && u[k - 1].cls == ucd::sb::SP {
            k -= 1;
        }
        while k > 0 && u[k - 1].cls == ucd::sb::CLOSE {
            k -= 1;
        }
        if k > 0 && sb_is_saterm(u[k - 1].cls) {
            return true;
        }
    }
    // SB998: Any x Any
    false
}

fn next_sentence(scalars: &[u32], start: usize) -> usize {
    if scalars.is_empty() {
        return 0;
    }

    let units = sb_build_units(scalars);

    let mut first = units.len();
    for (k, unit) in units.iter().enumerate() {
        if unit.start == start {
            first = k;
            break;
        }
    }

    let mut end = scalars.len();
    let mut k = first + 1;
    while k <= units.len() {
        if sb_break_before(&units, k) {
            end = if k < units.len() {
                units[k].start
            } else {
                scalars.len()
            };
            break;
        }
        k += 1;
    }

    if end <= start {
        end = start + 1;
    }
    end.min(scalars.len())
}
