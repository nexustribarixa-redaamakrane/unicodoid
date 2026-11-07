// Native Unicode text segmentation for the C++ API.
//
// The algorithms are implemented directly in C++ on top of the generated
// Unicode 18.0.0 data layer (`unicodoid_ucd_*`).  They deliberately do not
// forward to the C segmenter: the C++ implementation is an independent port of
// the UAX #29 and UAX #14 rule sets.
//
//   * grapheme clusters: UAX #29 GB1-GB13 / GB999,
//   * words:             UAX #29 WB1-WB16 / WB999 (with WB4 folding),
//   * sentences:         UAX #29 SB1-SB11 / SB998 (with SB5 folding),
//   * lines:             UAX #14 LB1-LB31.

#include "unicodoid/unicodoid.hpp"
#include "unicodoid/unicodoid_ucd.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace unicodoid {
namespace {

/* ------------------------------------------------------------------------- */
/* Shared scalar helpers                                                     */
/* ------------------------------------------------------------------------- */

bool is_control(uint32_t cp) {
    const uint8_t g = unicodoid_ucd_grapheme_break(cp);
    return g == UNICODOID_GCB_CONTROL || g == UNICODOID_GCB_CR ||
           g == UNICODOID_GCB_LF;
}

bool is_extended_pictographic(uint32_t cp) {
    return unicodoid_ucd_has_binary_property(
        cp, UNICODOID_BP_EXTENDED_PICTOGRAPHIC);
}

/* ------------------------------------------------------------------------- */
/* UAX #29 grapheme clusters (Unicode 18.0.0)                                */
/* ------------------------------------------------------------------------- */

size_t next_grapheme(const std::u32string& text, size_t start) {
    size_t i = start + 1;
    if (i > text.size()) return text.size();

    uint32_t prev = text[start];
    uint32_t prev_g = unicodoid_ucd_grapheme_break(prev);
    constexpr size_t kNoRi = static_cast<size_t>(-1);
    size_t prev_ri_pos = (prev_g == UNICODOID_GCB_RI) ? 0u : kNoRi;
    bool emoji_context = is_extended_pictographic(prev);
    bool emoji_zwj_pending = false;
    // GB9c (Unicode 18.0.0): ConjunctLinker ConjunctExtender* x LinkingConsonant.
    bool in_linker_conjunct =
        unicodoid_ucd_indic_conjunct_break(prev) == UNICODOID_INCB_LINKER;

    while (i < text.size()) {
        const uint32_t cur = text[i];
        const uint32_t cur_g = unicodoid_ucd_grapheme_break(cur);
        const uint32_t cur_incb = unicodoid_ucd_indic_conjunct_break(cur);
        bool boundary = true;

        // Grapheme cluster boundary rules GB3..GB13, applied in order.
        if (prev == 0x000D && cur == 0x000A) {
            // GB3: CR x LF (no boundary).
            boundary = false;
        } else if (is_control(prev) || is_control(cur)) {
            // GB4 / GB5: break before and after controls.
            boundary = true;
        } else if (prev_g == UNICODOID_GCB_L &&
                   (cur_g == UNICODOID_GCB_L || cur_g == UNICODOID_GCB_V ||
                    cur_g == UNICODOID_GCB_LV || cur_g == UNICODOID_GCB_LVT)) {
            // GB6: L x (L | V | LV | LVT).
            boundary = false;
        } else if ((prev_g == UNICODOID_GCB_LV || prev_g == UNICODOID_GCB_V) &&
                   (cur_g == UNICODOID_GCB_V || cur_g == UNICODOID_GCB_T)) {
            // GB7: (LV | V) x (V | T).
            boundary = false;
        } else if ((prev_g == UNICODOID_GCB_LVT || prev_g == UNICODOID_GCB_T) &&
                   cur_g == UNICODOID_GCB_T) {
            // GB8: (LVT | T) x T.
            boundary = false;
        } else if (cur_g == UNICODOID_GCB_EXTEND || cur_g == UNICODOID_GCB_ZWJ) {
            // GB9: x (Extend | ZWJ).
            boundary = false;
        } else if (cur_g == UNICODOID_GCB_SPACINGMARK) {
            // GB9a: x SpacingMark.
            boundary = false;
        } else if (prev_g == UNICODOID_GCB_PREPEND) {
            // GB9b: Prepend x.
            boundary = false;
        } else if (cur_incb == UNICODOID_INCB_CONSONANT && in_linker_conjunct) {
            // GB9c: InCB=Linker InCB=Extend* x InCB=Consonant.
            boundary = false;
        } else if (emoji_zwj_pending && is_extended_pictographic(cur)) {
            // GB11: ExtPict Extend* ZWJ x ExtPict.
            boundary = false;
        } else if (cur_g == UNICODOID_GCB_RI) {
            // GB12 / GB13: regional indicator pairing.
            const size_t cur_ri_pos =
                (prev_g == UNICODOID_GCB_RI) ? (prev_ri_pos + 1u) : 0u;
            if ((cur_ri_pos % 2u) == 1u) boundary = false;
        }

        // GB999: Any / Any (default break).
        if (boundary) return i;

        // Consume the current character and update cluster context.
        if (cur_g == UNICODOID_GCB_RI) {
            prev_ri_pos =
                (prev_g == UNICODOID_GCB_RI) ? (prev_ri_pos + 1u) : 0u;
        } else {
            prev_ri_pos = kNoRi;
        }

        if (cur == 0x200D) {
            if (emoji_context) emoji_zwj_pending = true;
        } else if (cur_g == UNICODOID_GCB_EXTEND) {
            // Extends stay inside the current emoji context.
        } else {
            emoji_context = is_extended_pictographic(cur);
            emoji_zwj_pending = false;
        }

        if (cur_incb == UNICODOID_INCB_LINKER) {
            in_linker_conjunct = true;
        } else if (cur_incb == UNICODOID_INCB_EXTEND) {
            // ConjunctExtender* keeps the linker context alive.
        } else {
            in_linker_conjunct = false;
        }

        prev = cur;
        prev_g = cur_g;
        ++i;
    }
    return text.size();
}

/* ------------------------------------------------------------------------- */
/* UAX #29 word breaking (Unicode 18.0.0)                                    */
/*                                                                           */
/* WB4 remaps every Extend / Format / ZWJ following a character that is not  */
/* CR, LF, or Newline to that base character and then ignores it.  This is   */
/* modelled by folding the base and its following Extend / Format / ZWJ run  */
/* into one unit.  All remaining rules are evaluated between units.          */
/* ------------------------------------------------------------------------- */

struct WbUnit {
    size_t start;
    size_t end;
    uint32_t cls;           // Word_Break of the base code point
    uint32_t last_cls;      // Word_Break of the last code point in the unit
    bool ends_with_zwj;     // last code point is ZWJ
    bool extpict;           // base code point is Extended_Pictographic
};

bool wb_is_newline(uint32_t cls) {
    return cls == UNICODOID_WB_CR || cls == UNICODOID_WB_LF ||
           cls == UNICODOID_WB_NEWLINE;
}

bool wb_is_ahletter(uint32_t cls) {
    return cls == UNICODOID_WB_ALETTER || cls == UNICODOID_WB_HEBREW_LETTER;
}

// MidLetter | MidNumLetQ (MidNumLet | Single_Quote).
bool wb_is_midletterq(uint32_t cls) {
    return cls == UNICODOID_WB_MIDLETTER || cls == UNICODOID_WB_MIDNUMLET ||
           cls == UNICODOID_WB_SINGLE_QUOTE;
}

// MidNum | MidNumLetQ (MidNumLet | Single_Quote).
bool wb_is_midnumq(uint32_t cls) {
    return cls == UNICODOID_WB_MIDNUM || cls == UNICODOID_WB_MIDNUMLET ||
           cls == UNICODOID_WB_SINGLE_QUOTE;
}

std::vector<WbUnit> wb_build_units(const std::u32string& text) {
    std::vector<WbUnit> units;
    units.reserve(text.size());

    size_t i = 0;
    while (i < text.size()) {
        const uint32_t cls = unicodoid_ucd_word_break(text[i]);
        WbUnit u{};
        u.start = i;
        u.cls = cls;
        u.last_cls = cls;
        u.extpict = is_extended_pictographic(text[i]);

        size_t j = i + 1;
        if (!wb_is_newline(cls)) {
            while (j < text.size()) {
                const uint32_t c2 = unicodoid_ucd_word_break(text[j]);
                if (c2 != UNICODOID_WB_EXTEND && c2 != UNICODOID_WB_FORMAT &&
                    c2 != UNICODOID_WB_ZWJ) {
                    break;
                }
                u.last_cls = c2;
                ++j;
            }
        }
        u.ends_with_zwj = u.last_cls == UNICODOID_WB_ZWJ;
        u.end = j;

        units.push_back(u);
        i = j;
    }
    return units;
}

bool wb_break_before(const std::vector<WbUnit>& u, size_t i) {
    if (i == 0) return false;        // WB1 (leading sot break is implicit)
    if (i >= u.size()) return true;  // WB2: / eot

    const WbUnit& prev = u[i - 1];
    const WbUnit& cur = u[i];
    const uint32_t p = prev.cls;
    const uint32_t c = cur.cls;

    // WB3: CR x LF
    if (p == UNICODOID_WB_CR && c == UNICODOID_WB_LF) return false;
    // WB3.1: (Newline | CR | LF) /
    if (wb_is_newline(p)) return true;
    // WB3.2: / (Newline | CR | LF)
    if (wb_is_newline(c)) return true;
    // WB3.3: ZWJ x ExtPict
    if (prev.ends_with_zwj && cur.extpict) return false;
    // WB3.4: WSegSpace x WSegSpace (evaluated on raw characters, i.e. before
    // WB4 folds any following Extend / Format / ZWJ into the space).
    if (prev.last_cls == UNICODOID_WB_WSEGSPACE && c == UNICODOID_WB_WSEGSPACE) {
        return false;
    }
    // WB5: AHLetter x AHLetter
    if (wb_is_ahletter(p) && wb_is_ahletter(c)) return false;
    // WB6: AHLetter x (MidLetter | MidNumLetQ) AHLetter
    if (wb_is_ahletter(p) && wb_is_midletterq(c) && i + 1 < u.size() &&
        wb_is_ahletter(u[i + 1].cls)) {
        return false;
    }
    // WB7: AHLetter (MidLetter | MidNumLetQ) x AHLetter
    if (wb_is_midletterq(p) && wb_is_ahletter(c) && i >= 2 &&
        wb_is_ahletter(u[i - 2].cls)) {
        return false;
    }
    // WB7.1: Hebrew_Letter x Single_Quote
    if (p == UNICODOID_WB_HEBREW_LETTER && c == UNICODOID_WB_SINGLE_QUOTE) {
        return false;
    }
    // WB7.2: Hebrew_Letter x Double_Quote Hebrew_Letter
    if (p == UNICODOID_WB_HEBREW_LETTER && c == UNICODOID_WB_DOUBLE_QUOTE &&
        i + 1 < u.size() && u[i + 1].cls == UNICODOID_WB_HEBREW_LETTER) {
        return false;
    }
    // WB7.3: Hebrew_Letter Double_Quote x Hebrew_Letter
    if (p == UNICODOID_WB_DOUBLE_QUOTE && c == UNICODOID_WB_HEBREW_LETTER &&
        i >= 2 && u[i - 2].cls == UNICODOID_WB_HEBREW_LETTER) {
        return false;
    }
    // WB8: Numeric x Numeric
    if (p == UNICODOID_WB_NUMERIC && c == UNICODOID_WB_NUMERIC) return false;
    // WB9: AHLetter x Numeric
    if (wb_is_ahletter(p) && c == UNICODOID_WB_NUMERIC) return false;
    // WB10: Numeric x AHLetter
    if (p == UNICODOID_WB_NUMERIC && wb_is_ahletter(c)) return false;
    // WB11: Numeric (MidNum | MidNumLetQ) x Numeric
    if (wb_is_midnumq(p) && c == UNICODOID_WB_NUMERIC && i >= 2 &&
        u[i - 2].cls == UNICODOID_WB_NUMERIC) {
        return false;
    }
    // WB12: Numeric x (MidNum | MidNumLetQ) Numeric
    if (p == UNICODOID_WB_NUMERIC && wb_is_midnumq(c) && i + 1 < u.size() &&
        u[i + 1].cls == UNICODOID_WB_NUMERIC) {
        return false;
    }
    // WB13: Katakana x Katakana
    if (p == UNICODOID_WB_KATAKANA && c == UNICODOID_WB_KATAKANA) return false;
    // WB13.1: (AHLetter | Numeric | Katakana | ExtendNumLet) x ExtendNumLet
    if ((wb_is_ahletter(p) || p == UNICODOID_WB_NUMERIC ||
         p == UNICODOID_WB_KATAKANA || p == UNICODOID_WB_EXTENDNUMLET) &&
        c == UNICODOID_WB_EXTENDNUMLET) {
        return false;
    }
    // WB13.2: ExtendNumLet x (AHLetter | Numeric | Katakana)
    if (p == UNICODOID_WB_EXTENDNUMLET &&
        (wb_is_ahletter(c) || c == UNICODOID_WB_NUMERIC ||
         c == UNICODOID_WB_KATAKANA)) {
        return false;
    }
    // WB15 / WB16: regional indicator pairs
    if (p == UNICODOID_WB_RI && c == UNICODOID_WB_RI) {
        size_t count = 0;
        size_t k = i;
        while (k > 0 && u[k - 1].cls == UNICODOID_WB_RI) {
            ++count;
            --k;
        }
        if ((count % 2u) == 1u) return false;
    }
    // WB999: Any / Any
    return true;
}

size_t next_word(const std::u32string& text, size_t start) {
    if (text.empty()) return 0;

    const std::vector<WbUnit> units = wb_build_units(text);

    size_t first = units.size();
    for (size_t k = 0; k < units.size(); ++k) {
        if (units[k].start == start) {
            first = k;
            break;
        }
    }

    size_t end = text.size();
    for (size_t k = first + 1; k <= units.size(); ++k) {
        if (wb_break_before(units, k)) {
            end = (k < units.size()) ? units[k].start : text.size();
            break;
        }
    }

    if (end <= start) end = start + 1;
    if (end > text.size()) end = text.size();
    return end;
}

/* ------------------------------------------------------------------------- */
/* UAX #14 line breaking (Unicode 18.0.0, revision 57)                       */
/*                                                                           */
/* LB9 is applied by folding every base character together with the          */
/* following CM / ZWJ run into a single "unit".  Remaining CM / ZWJ          */
/* characters (LB10) become one-character AL units.  All line break rules    */
/* are then evaluated between units, which is exactly the "ignore affected   */
/* CM / ZWJ" requirement of LB9.                                             */
/* ------------------------------------------------------------------------- */

struct LbUnit {
    size_t start;
    size_t end;
    uint32_t cls;          // resolved Line_Break class
    uint8_t gc;            // General_Category of the base code point
    bool east_asian;       // East_Asian_Width in {F, W, H}
    bool dotted;           // base code point is U+25CC DOTTED CIRCLE
    bool extpict_cn;       // Extended_Pictographic && General_Category=Cn
    bool ends_with_zwj;    // last code point of the unit is ZWJ
};

// LB1: resolve AI, SG, XX, SA, and CJ to their working classes.
uint32_t lb_resolve_class(uint32_t cp) {
    const uint32_t lb = unicodoid_ucd_line_break(cp);
    const uint8_t gc = unicodoid_ucd_general_category(cp);

    switch (lb) {
        case UNICODOID_LB_AI:
        case UNICODOID_LB_SG:
        case UNICODOID_LB_XX:
            return UNICODOID_LB_AL;
        case UNICODOID_LB_SA:
            return (gc == UNICODOID_GC_MN || gc == UNICODOID_GC_MC)
                       ? static_cast<uint32_t>(UNICODOID_LB_CM)
                       : static_cast<uint32_t>(UNICODOID_LB_AL);
        case UNICODOID_LB_CJ:
            return UNICODOID_LB_NS;
        default:
            return lb;
    }
}

bool lb_is_excluded_from_lb9(uint32_t cls) {
    return cls == UNICODOID_LB_BK || cls == UNICODOID_LB_CR ||
           cls == UNICODOID_LB_LF || cls == UNICODOID_LB_NL ||
           cls == UNICODOID_LB_SP || cls == UNICODOID_LB_ZW;
}

std::vector<LbUnit> lb_build_units(const std::u32string& text) {
    std::vector<LbUnit> units;
    units.reserve(text.size());

    size_t i = 0;
    while (i < text.size()) {
        const uint32_t cp = text[i];
        const uint32_t cls = lb_resolve_class(cp);
        const uint8_t gc = unicodoid_ucd_general_category(cp);
        const uint8_t eaw = unicodoid_ucd_east_asian_width(cp);

        LbUnit u{};
        u.start = i;
        u.gc = gc;
        u.east_asian = (eaw == 2u || eaw == 3u || eaw == 5u);
        u.dotted = (cp == 0x25CCu);
        u.extpict_cn = (gc == UNICODOID_GC_CN) && is_extended_pictographic(cp);

        size_t j = i + 1;
        if (cls == UNICODOID_LB_CM || cls == UNICODOID_LB_ZWJ) {
            // LB10: a remaining combining mark / ZWJ behaves like AL.
            // Following CM / ZWJ characters are also "remaining", so they
            // form their own AL units (the resulting break decisions are
            // identical because AL x AL is prohibited by LB28).
            u.cls = UNICODOID_LB_AL;
            u.ends_with_zwj = (cls == UNICODOID_LB_ZWJ);
        } else if (lb_is_excluded_from_lb9(cls)) {
            // LB9 does not absorb CM / ZWJ after these classes.
            u.cls = cls;
        } else {
            // LB9: treat X (CM | ZWJ)* as X.
            u.cls = cls;
            bool last_zwj = false;
            while (j < text.size()) {
                const uint32_t c2cls = lb_resolve_class(text[j]);
                if (c2cls != UNICODOID_LB_CM && c2cls != UNICODOID_LB_ZWJ) {
                    break;
                }
                last_zwj = (c2cls == UNICODOID_LB_ZWJ);
                ++j;
            }
            u.ends_with_zwj = last_zwj;
        }

        u.end = j;
        units.push_back(u);
        i = j;
    }
    return units;
}

// True when the units immediately before index k end with NU (SY | IS)*.
bool lb_nu_seq_before(const std::vector<LbUnit>& u, size_t k) {
    size_t j = k;
    while (j > 0 && (u[j - 1].cls == UNICODOID_LB_SY ||
                     u[j - 1].cls == UNICODOID_LB_IS)) {
        --j;
    }
    return j > 0 && u[j - 1].cls == UNICODOID_LB_NU;
}

// Decide whether a line break is allowed immediately before unit i.
bool lb_break_before(const std::vector<LbUnit>& u, size_t i) {
    if (i == 0) return false;        // LB2: sot x
    if (i >= u.size()) return true;  // LB3: ! eot

    const LbUnit& prev = u[i - 1];
    const LbUnit& cur = u[i];
    const uint32_t p = prev.cls;
    const uint32_t c = cur.cls;

    // LB4: BK !
    if (p == UNICODOID_LB_BK) return true;
    // LB5: CR x LF, then CR / LF / NL !
    if (p == UNICODOID_LB_CR && c == UNICODOID_LB_LF) return false;
    if (p == UNICODOID_LB_CR || p == UNICODOID_LB_LF || p == UNICODOID_LB_NL) {
        return true;
    }
    // LB6: x (BK | CR | LF | NL)
    if (c == UNICODOID_LB_BK || c == UNICODOID_LB_CR ||
        c == UNICODOID_LB_LF || c == UNICODOID_LB_NL) {
        return false;
    }
    // LB7: x SP, x ZW
    if (c == UNICODOID_LB_SP || c == UNICODOID_LB_ZW) return false;
    // LB8: ZW SP* !
    {
        size_t k = i;
        while (k > 0 && u[k - 1].cls == UNICODOID_LB_SP) --k;
        if (k > 0 && u[k - 1].cls == UNICODOID_LB_ZW) return true;
    }
    // LB8a: ZWJ x
    if (prev.ends_with_zwj) return false;
    // LB11: x WJ, WJ x
    if (c == UNICODOID_LB_WJ || p == UNICODOID_LB_WJ) return false;
    // LB12: GL x
    if (p == UNICODOID_LB_GL) return false;
    // LB12a: [^SP HY HH] x GL
    if (c == UNICODOID_LB_GL && p != UNICODOID_LB_SP &&
        p != UNICODOID_LB_HY && p != UNICODOID_LB_HH) {
        return false;
    }
    // LB13: x CL, x CP, x EX, x SY
    if (c == UNICODOID_LB_CL || c == UNICODOID_LB_CP ||
        c == UNICODOID_LB_EX || c == UNICODOID_LB_SY) {
        return false;
    }
    // LB14: OP SP* x
    {
        size_t k = i;
        while (k > 0 && u[k - 1].cls == UNICODOID_LB_SP) --k;
        if (k > 0 && u[k - 1].cls == UNICODOID_LB_OP) return false;
    }
    // LB15a: (sot | BK | CR | LF | NL | OP | QU | GL | SP | ZW) QU_Pi SP* x
    {
        size_t k = i;
        while (k > 0 && u[k - 1].cls == UNICODOID_LB_SP) --k;
        if (k > 0 && u[k - 1].cls == UNICODOID_LB_QU &&
            u[k - 1].gc == UNICODOID_GC_PI) {
            if (k - 1 == 0) return false;  // sot QU_Pi SP* x
            const uint32_t pc = u[k - 2].cls;
            if (pc == UNICODOID_LB_BK || pc == UNICODOID_LB_CR ||
                pc == UNICODOID_LB_LF || pc == UNICODOID_LB_NL ||
                pc == UNICODOID_LB_OP || pc == UNICODOID_LB_QU ||
                pc == UNICODOID_LB_GL || pc == UNICODOID_LB_SP ||
                pc == UNICODOID_LB_ZW) {
                return false;
            }
        }
    }
    // LB15b: x QU_Pf (SP | GL | WJ | CL | QU | CP | EX | IS | SY | BK | CR |
    //                LF | NL | ZW | eot)
    if (c == UNICODOID_LB_QU && cur.gc == UNICODOID_GC_PF) {
        if (i + 1 >= u.size()) return false;
        const uint32_t nc = u[i + 1].cls;
        if (nc == UNICODOID_LB_SP || nc == UNICODOID_LB_GL ||
            nc == UNICODOID_LB_WJ || nc == UNICODOID_LB_CL ||
            nc == UNICODOID_LB_QU || nc == UNICODOID_LB_CP ||
            nc == UNICODOID_LB_EX || nc == UNICODOID_LB_IS ||
            nc == UNICODOID_LB_SY || nc == UNICODOID_LB_BK ||
            nc == UNICODOID_LB_CR || nc == UNICODOID_LB_LF ||
            nc == UNICODOID_LB_NL || nc == UNICODOID_LB_ZW) {
            return false;
        }
    }
    // LB15c: SP ! IS NU
    if (p == UNICODOID_LB_SP && c == UNICODOID_LB_IS && i + 1 < u.size() &&
        u[i + 1].cls == UNICODOID_LB_NU) {
        return true;
    }
    // LB15d: x IS
    if (c == UNICODOID_LB_IS) return false;
    // LB16: (CL | CP) SP* x NS
    if (c == UNICODOID_LB_NS) {
        size_t k = i;
        while (k > 0 && u[k - 1].cls == UNICODOID_LB_SP) --k;
        if (k > 0 && (u[k - 1].cls == UNICODOID_LB_CL ||
                      u[k - 1].cls == UNICODOID_LB_CP)) {
            return false;
        }
    }
    // LB17: B2 SP* x B2
    if (c == UNICODOID_LB_B2) {
        size_t k = i;
        while (k > 0 && u[k - 1].cls == UNICODOID_LB_SP) --k;
        if (k > 0 && u[k - 1].cls == UNICODOID_LB_B2) return false;
    }
    // LB18: SP !
    if (p == UNICODOID_LB_SP) return true;
    // LB19: x QUmPi, QUmPf x
    if (c == UNICODOID_LB_QU && cur.gc != UNICODOID_GC_PI) return false;
    if (p == UNICODOID_LB_QU && prev.gc != UNICODOID_GC_PF) return false;
    // LB19a: East Asian quotation tailoring
    if (c == UNICODOID_LB_QU) {
        if (!prev.east_asian) return false;       // [^EA] x QU
        if (i + 1 >= u.size()) return false;      // x QU eot
        if (!u[i + 1].east_asian) return false;   // x QU [^EA]
    }
    if (p == UNICODOID_LB_QU && !cur.east_asian) return false;  // QU x [^EA]
    if (p == UNICODOID_LB_QU) {
        // (sot | [^EA]) QU x
        if (i - 1 == 0) return false;
        if (!u[i - 2].east_asian) return false;
    }
    // LB20: ! CB, CB !
    if (c == UNICODOID_LB_CB || p == UNICODOID_LB_CB) return true;
    // LB20a: (sot | BK | CR | LF | NL | SP | ZW | CB | GL) (HY | HH) x (AL | HL)
    if ((p == UNICODOID_LB_HY || p == UNICODOID_LB_HH) &&
        (c == UNICODOID_LB_AL || c == UNICODOID_LB_HL)) {
        if (i - 1 == 0) return false;
        const uint32_t pc = u[i - 2].cls;
        if (pc == UNICODOID_LB_BK || pc == UNICODOID_LB_CR ||
            pc == UNICODOID_LB_LF || pc == UNICODOID_LB_NL ||
            pc == UNICODOID_LB_SP || pc == UNICODOID_LB_ZW ||
            pc == UNICODOID_LB_CB || pc == UNICODOID_LB_GL) {
            return false;
        }
    }
    // LB21: x BA, x HH, x HY, x NS, BB x
    if (c == UNICODOID_LB_BA || c == UNICODOID_LB_HH ||
        c == UNICODOID_LB_HY || c == UNICODOID_LB_NS) {
        return false;
    }
    if (p == UNICODOID_LB_BB) return false;
    // LB21a: HL (HY | HH) x [^HL]
    if ((p == UNICODOID_LB_HY || p == UNICODOID_LB_HH) && i >= 2 &&
        u[i - 2].cls == UNICODOID_LB_HL && c != UNICODOID_LB_HL) {
        return false;
    }
    // LB21b: SY x HL
    if (p == UNICODOID_LB_SY && c == UNICODOID_LB_HL) return false;
    // LB22: x IN
    if (c == UNICODOID_LB_IN) return false;
    // LB23: (AL | HL) x NU, NU x (AL | HL)
    if ((p == UNICODOID_LB_AL || p == UNICODOID_LB_HL) &&
        c == UNICODOID_LB_NU) {
        return false;
    }
    if (p == UNICODOID_LB_NU &&
        (c == UNICODOID_LB_AL || c == UNICODOID_LB_HL)) {
        return false;
    }
    // LB23a: PR x (ID | EB | EM), (ID | EB | EM) x PO
    if (p == UNICODOID_LB_PR &&
        (c == UNICODOID_LB_ID || c == UNICODOID_LB_EB ||
         c == UNICODOID_LB_EM)) {
        return false;
    }
    if ((p == UNICODOID_LB_ID || p == UNICODOID_LB_EB ||
         p == UNICODOID_LB_EM) && c == UNICODOID_LB_PO) {
        return false;
    }
    // LB24: (PR | PO) x (AL | HL), (AL | HL) x (PR | PO)
    if ((p == UNICODOID_LB_PR || p == UNICODOID_LB_PO) &&
        (c == UNICODOID_LB_AL || c == UNICODOID_LB_HL)) {
        return false;
    }
    if ((p == UNICODOID_LB_AL || p == UNICODOID_LB_HL) &&
        (c == UNICODOID_LB_PR || c == UNICODOID_LB_PO)) {
        return false;
    }
    // LB25: numeric sequences
    if (c == UNICODOID_LB_PO || c == UNICODOID_LB_PR) {
        if ((p == UNICODOID_LB_CL || p == UNICODOID_LB_CP) &&
            lb_nu_seq_before(u, i - 1)) {
            return false;
        }
        if (lb_nu_seq_before(u, i)) return false;
    }
    if (p == UNICODOID_LB_PO || p == UNICODOID_LB_PR) {
        if (c == UNICODOID_LB_OP) {
            if (i + 1 < u.size() && u[i + 1].cls == UNICODOID_LB_NU) {
                return false;
            }
            if (i + 2 < u.size() && u[i + 1].cls == UNICODOID_LB_IS &&
                u[i + 2].cls == UNICODOID_LB_NU) {
                return false;
            }
        }
        if (c == UNICODOID_LB_NU) return false;
    }
    if (p == UNICODOID_LB_HY && c == UNICODOID_LB_NU) return false;
    if (p == UNICODOID_LB_IS && c == UNICODOID_LB_NU) return false;
    if (c == UNICODOID_LB_NU && lb_nu_seq_before(u, i)) return false;
    // LB26: Korean syllables
    if (p == UNICODOID_LB_JL &&
        (c == UNICODOID_LB_JL || c == UNICODOID_LB_JV ||
         c == UNICODOID_LB_H2 || c == UNICODOID_LB_H3)) {
        return false;
    }
    if ((p == UNICODOID_LB_JV || p == UNICODOID_LB_H2) &&
        (c == UNICODOID_LB_JV || c == UNICODOID_LB_JT)) {
        return false;
    }
    if ((p == UNICODOID_LB_JT || p == UNICODOID_LB_H3) &&
        c == UNICODOID_LB_JT) {
        return false;
    }
    // LB27: Korean syllable blocks behave like ID
    if ((p == UNICODOID_LB_JL || p == UNICODOID_LB_JV ||
         p == UNICODOID_LB_JT || p == UNICODOID_LB_H2 ||
         p == UNICODOID_LB_H3) && c == UNICODOID_LB_PO) {
        return false;
    }
    if (p == UNICODOID_LB_PR &&
        (c == UNICODOID_LB_JL || c == UNICODOID_LB_JV ||
         c == UNICODOID_LB_JT || c == UNICODOID_LB_H2 ||
         c == UNICODOID_LB_H3)) {
        return false;
    }
    // LB28: (AL | HL) x (AL | HL)
    if ((p == UNICODOID_LB_AL || p == UNICODOID_LB_HL) &&
        (c == UNICODOID_LB_AL || c == UNICODOID_LB_HL)) {
        return false;
    }
    // LB28a: Brahmic orthographic syllables
    {
        const bool prev_ak =
            p == UNICODOID_LB_AK || prev.dotted || p == UNICODOID_LB_AS;
        const bool cur_ak =
            c == UNICODOID_LB_AK || cur.dotted || c == UNICODOID_LB_AS;
        if (p == UNICODOID_LB_AP && cur_ak) return false;
        if (prev_ak && (c == UNICODOID_LB_VF || c == UNICODOID_LB_VI)) {
            return false;
        }
        if (p == UNICODOID_LB_VI && (c == UNICODOID_LB_AK || cur.dotted) &&
            i >= 2) {
            const LbUnit& p2 = u[i - 2];
            if (p2.cls == UNICODOID_LB_AK || p2.dotted ||
                p2.cls == UNICODOID_LB_AS) {
                return false;
            }
        }
        if (prev_ak && cur_ak && i + 1 < u.size() &&
            u[i + 1].cls == UNICODOID_LB_VF) {
            return false;
        }
    }
    // LB29: IS x (AL | HL)
    if (p == UNICODOID_LB_IS &&
        (c == UNICODOID_LB_AL || c == UNICODOID_LB_HL)) {
        return false;
    }
    // LB30: (AL | HL | NU) x OPmEastAsian, CPmEastAsian x (AL | HL | NU)
    if ((p == UNICODOID_LB_AL || p == UNICODOID_LB_HL ||
         p == UNICODOID_LB_NU) && c == UNICODOID_LB_OP && !cur.east_asian) {
        return false;
    }
    if (p == UNICODOID_LB_CP && !prev.east_asian &&
        (c == UNICODOID_LB_AL || c == UNICODOID_LB_HL ||
         c == UNICODOID_LB_NU)) {
        return false;
    }
    // LB30a: regional indicator pairs
    if (p == UNICODOID_LB_RI && c == UNICODOID_LB_RI) {
        size_t count = 0;
        size_t k = i;
        while (k > 0 && u[k - 1].cls == UNICODOID_LB_RI) {
            ++count;
            --k;
        }
        if ((count % 2u) == 1u) return false;
    }
    // LB30b: EB x EM, ExtPictUnassigned x EM
    if (c == UNICODOID_LB_EM &&
        (p == UNICODOID_LB_EB || prev.extpict_cn)) {
        return false;
    }
    // LB31: break everywhere else.
    return true;
}

size_t next_line(const std::u32string& text, size_t start) {
    if (text.empty()) return 0;

    const std::vector<LbUnit> units = lb_build_units(text);

    size_t first = units.size();
    for (size_t k = 0; k < units.size(); ++k) {
        if (units[k].start == start) {
            first = k;
            break;
        }
    }

    size_t end = text.size();
    for (size_t k = first + 1; k <= units.size(); ++k) {
        if (lb_break_before(units, k)) {
            end = (k < units.size()) ? units[k].start : text.size();
            break;
        }
    }

    if (end <= start) end = start + 1;
    if (end > text.size()) end = text.size();
    return end;
}

/* ------------------------------------------------------------------------- */
/* UAX #29 sentence breaking (Unicode 18.0.0)                                */
/*                                                                           */
/* SB5 ignores Extend / Format characters following a base character that    */
/* is not a paragraph separator.  Folding them into the base makes the SB    */
/* rules directly expressible between units.                                 */
/* ------------------------------------------------------------------------- */

struct SbUnit {
    size_t start;
    size_t end;
    uint32_t cls;
};

bool sb_is_para(uint32_t cls) {
    return cls == UNICODOID_SB_SEP || cls == UNICODOID_SB_CR ||
           cls == UNICODOID_SB_LF;
}

bool sb_is_saterm(uint32_t cls) {
    return cls == UNICODOID_SB_STERM || cls == UNICODOID_SB_ATERM;
}

std::vector<SbUnit> sb_build_units(const std::u32string& text) {
    std::vector<SbUnit> units;
    units.reserve(text.size());

    size_t i = 0;
    while (i < text.size()) {
        const uint32_t cls = unicodoid_ucd_sentence_break(text[i]);

        size_t j = i + 1;
        if (!sb_is_para(cls)) {
            // SB5: X (Extend | Format)* -> X.
            while (j < text.size()) {
                const uint32_t c2 = unicodoid_ucd_sentence_break(text[j]);
                if (c2 != UNICODOID_SB_EXTEND && c2 != UNICODOID_SB_FORMAT) {
                    break;
                }
                ++j;
            }
        }

        units.push_back(SbUnit{i, j, cls});
        i = j;
    }
    return units;
}

// Scan the left context ending immediately before unit i and test for
// "SATerm Close*" (allow_sp false) or "SATerm Close* Sp*" (allow_sp true).
bool sb_scan_saterm(const std::vector<SbUnit>& u, size_t i, bool allow_sp,
                    bool aterm_only) {
    size_t k = i;
    if (allow_sp) {
        while (k > 0 && u[k - 1].cls == UNICODOID_SB_SP) --k;
    }
    while (k > 0 && u[k - 1].cls == UNICODOID_SB_CLOSE) --k;
    if (k == 0) return false;
    const uint32_t t = u[k - 1].cls;
    if (aterm_only) return t == UNICODOID_SB_ATERM;
    return sb_is_saterm(t);
}

// SB8 right-hand side: (not (OLetter | Upper | Lower | ParaSep | SATerm))* Lower.
bool sb8_lookahead(const std::vector<SbUnit>& u, size_t i) {
    size_t k = i;
    while (k < u.size()) {
        const uint32_t c = u[k].cls;
        if (c == UNICODOID_SB_OLETTER || c == UNICODOID_SB_UPPER ||
            c == UNICODOID_SB_LOWER || sb_is_para(c) || sb_is_saterm(c)) {
            break;
        }
        ++k;
    }
    return k < u.size() && u[k].cls == UNICODOID_SB_LOWER;
}

bool sb_break_before(const std::vector<SbUnit>& u, size_t i) {
    if (i == 0) return false;        // SB1
    if (i >= u.size()) return true;  // SB2: Any / eot

    const SbUnit& prev = u[i - 1];
    const SbUnit& cur = u[i];
    const uint32_t p = prev.cls;
    const uint32_t c = cur.cls;

    // SB3: CR x LF
    if (p == UNICODOID_SB_CR && c == UNICODOID_SB_LF) return false;
    // SB4: ParaSep /
    if (sb_is_para(p)) return true;
    // SB6: ATerm x Numeric
    if (p == UNICODOID_SB_ATERM && c == UNICODOID_SB_NUMERIC) return false;
    // SB7: (Upper | Lower) ATerm x Upper
    if (p == UNICODOID_SB_ATERM && c == UNICODOID_SB_UPPER && i >= 2) {
        const uint32_t p2 = u[i - 2].cls;
        if (p2 == UNICODOID_SB_UPPER || p2 == UNICODOID_SB_LOWER) return false;
    }
    // SB8: ATerm Close* Sp* x (not (OLetter | Upper | Lower | ParaSep | SATerm))* Lower
    if (sb_scan_saterm(u, i, true, true) && sb8_lookahead(u, i)) return false;
    // SB8a: SATerm Close* Sp* x (SContinue | SATerm)
    if ((c == UNICODOID_SB_SCONTINUE || sb_is_saterm(c)) &&
        sb_scan_saterm(u, i, true, false)) {
        return false;
    }
    // SB9: SATerm Close* x (Close | Sp | ParaSep)
    if ((c == UNICODOID_SB_CLOSE || c == UNICODOID_SB_SP || sb_is_para(c)) &&
        sb_scan_saterm(u, i, false, false)) {
        return false;
    }
    // SB10: SATerm Close* Sp* x (Sp | ParaSep)
    if ((c == UNICODOID_SB_SP || sb_is_para(c)) &&
        sb_scan_saterm(u, i, true, false)) {
        return false;
    }
    // SB11: SATerm Close* Sp* ParaSep? /
    {
        size_t k = i;
        if (k > 0 && sb_is_para(u[k - 1].cls)) --k;
        while (k > 0 && u[k - 1].cls == UNICODOID_SB_SP) --k;
        while (k > 0 && u[k - 1].cls == UNICODOID_SB_CLOSE) --k;
        if (k > 0 && sb_is_saterm(u[k - 1].cls)) return true;
    }
    // SB998: Any x Any
    return false;
}

size_t next_sentence(const std::u32string& text, size_t start) {
    if (text.empty()) return 0;

    const std::vector<SbUnit> units = sb_build_units(text);

    size_t first = units.size();
    for (size_t k = 0; k < units.size(); ++k) {
        if (units[k].start == start) {
            first = k;
            break;
        }
    }

    size_t end = text.size();
    for (size_t k = first + 1; k <= units.size(); ++k) {
        if (sb_break_before(units, k)) {
            end = (k < units.size()) ? units[k].start : text.size();
            break;
        }
    }

    if (end <= start) end = start + 1;
    if (end > text.size()) end = text.size();
    return end;
}

size_t next_boundary(const std::u32string& text, size_t start,
                     BoundaryType type) {
    switch (type) {
        case BoundaryType::Grapheme:
            return next_grapheme(text, start);
        case BoundaryType::Word:
            return next_word(text, start);
        case BoundaryType::Line:
            return next_line(text, start);
        case BoundaryType::Sentence:
            return next_sentence(text, start);
    }
    return start + 1;
}

}  // namespace

std::vector<size_t> Segmenter::boundaries(const std::u32string& text,
                                          BoundaryType type) {
    std::vector<size_t> result;
    result.push_back(0);
    if (text.empty()) return result;

    size_t start = 0;
    while (start < text.size()) {
        size_t end = next_boundary(text, start, type);
        if (end <= start) end = start + 1;
        if (end > text.size()) end = text.size();
        result.push_back(end);
        start = end;
    }
    return result;
}

std::vector<std::pair<size_t, size_t>> Segmenter::segments(
    const std::u32string& text, BoundaryType type) {
    std::vector<std::pair<size_t, size_t>> result;
    const std::vector<size_t> marks = boundaries(text, type);
    for (size_t i = 1; i < marks.size(); ++i) {
        result.emplace_back(marks[i - 1], marks[i]);
    }
    return result;
}

}  // namespace unicodoid
