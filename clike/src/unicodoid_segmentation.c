#include "unicodoid/unicodoid_segmentation.h"
#include "unicodoid/unicodoid_properties.h"
#include "unicodoid/unicodoid_ucd.h"
#include <stdlib.h>
#include <string.h>

typedef struct seg_internal {
    uint32_t* cps;
    size_t*   offsets; /* count + 1 entries: start offsets and final end */
    size_t    count;
    size_t    index;
} seg_internal_t;

static void internal_free(seg_internal_t* in) {
    if (!in) return;
    free(in->cps);
    free(in->offsets);
    free(in);
}

unicodoid_status_t unicodoid_segmenter_init(
    unicodoid_segmenter_t* seg,
    const unicodoid_system_t* sys,
    const void* data,
    size_t length_bytes,
    unicodoid_boundary_type_t boundary_type
) {
    if (!seg || !sys) return UNICODOID_ERR_INVALID_PARAM;
    if (length_bytes > 0 && !data) return UNICODOID_ERR_INVALID_PARAM;
    if (!unicodoid_system_has_unicode_profile(sys)) return UNICODOID_ERR_UNSUPPORTED;

    memset(seg, 0, sizeof(*seg));
    seg->system = sys;
    seg->data = (const uint8_t*)data;
    seg->length_bytes = length_bytes;
    seg->boundary_type = boundary_type;

    unicodoid_status_t st = unicodoid_codec_init(&seg->codec, sys);
    if (st != UNICODOID_OK) return st;

    seg_internal_t* in = (seg_internal_t*)calloc(1, sizeof(seg_internal_t));
    if (!in) {
        unicodoid_codec_free(&seg->codec);
        return UNICODOID_ERR_ALLOCATION_FAILED;
    }

    size_t capacity = 32;
    in->cps = (uint32_t*)malloc(capacity * sizeof(uint32_t));
    in->offsets = (size_t*)malloc((capacity + 1) * sizeof(size_t));
    if (!in->cps || !in->offsets) {
        internal_free(in);
        unicodoid_codec_free(&seg->codec);
        return UNICODOID_ERR_ALLOCATION_FAILED;
    }

    size_t off = 0;
    while (off < length_bytes) {
        if (in->count == capacity) {
            capacity *= 2;
            uint32_t* nc = (uint32_t*)realloc(in->cps, capacity * sizeof(uint32_t));
            if (!nc) {
                internal_free(in);
                unicodoid_codec_free(&seg->codec);
                return UNICODOID_ERR_ALLOCATION_FAILED;
            }
            in->cps = nc;
            size_t* no = (size_t*)realloc(in->offsets, (capacity + 1) * sizeof(size_t));
            if (!no) {
                internal_free(in);
                unicodoid_codec_free(&seg->codec);
                return UNICODOID_ERR_ALLOCATION_FAILED;
            }
            in->offsets = no;
        }

        in->offsets[in->count] = off;
        unicodoid_char64_t native = 0;
        size_t read = 0;
        st = unicodoid_codec_decode_char(
            &seg->codec, (const uint8_t*)data + off, length_bytes - off, &native, &read
        );
        if (st != UNICODOID_OK) {
            internal_free(in);
            unicodoid_codec_free(&seg->codec);
            return st;
        }

        uint32_t cp = 0;
        st = sys->unicode_profile->to_unicode(sys->unicode_profile, native, &cp);
        if (st != UNICODOID_OK) {
            internal_free(in);
            unicodoid_codec_free(&seg->codec);
            return st;
        }
        in->cps[in->count] = cp;
        in->count++;
        off += read;
    }
    in->offsets[in->count] = length_bytes;
    in->index = 0;

    seg->internal = in;
    return UNICODOID_OK;
}

void unicodoid_segmenter_free(unicodoid_segmenter_t* seg) {
    if (!seg) return;
    internal_free((seg_internal_t*)seg->internal);
    unicodoid_codec_free(&seg->codec);
    memset(seg, 0, sizeof(*seg));
}

/* ------------------------------------------------------------------------- */
/* UAX #29 property-driven predicates (generated Unicode 18.0.0 data)       */
/* ------------------------------------------------------------------------- */

static uint32_t seg_gcb(uint32_t cp) {
    return (uint32_t)unicodoid_ucd_grapheme_break(cp);
}

static uint32_t seg_wb(uint32_t cp) {
    return (uint32_t)unicodoid_ucd_word_break(cp);
}

static uint32_t seg_sb(uint32_t cp) {
    return (uint32_t)unicodoid_ucd_sentence_break(cp);
}

static uint32_t seg_lb(uint32_t cp) {
    return (uint32_t)unicodoid_ucd_line_break(cp);
}

static uint32_t seg_incb(uint32_t cp) {
    return (uint32_t)unicodoid_ucd_indic_conjunct_break(cp);
}

static bool seg_is_control_cp(uint32_t cp) {
    uint32_t g = seg_gcb(cp);
    return g == (uint32_t)UNICODOID_GCB_CONTROL ||
           g == (uint32_t)UNICODOID_GCB_CR ||
           g == (uint32_t)UNICODOID_GCB_LF;
}

static bool seg_is_regional_indicator_cp(uint32_t cp) {
    return seg_gcb(cp) == (uint32_t)UNICODOID_GCB_RI;
}

static bool seg_is_extended_pictographic_cp(uint32_t cp) {
    return unicodoid_ucd_has_binary_property(cp, UNICODOID_BP_EXTENDED_PICTOGRAPHIC);
}

static bool is_word_char(uint32_t cp) {
    return unicodoid_unicode_is_alphabetic(cp) ||
           unicodoid_unicode_is_numeric(cp) ||
           cp == '_';
}

static bool is_space(uint32_t cp) {
    return unicodoid_unicode_is_whitespace(cp);
}

static size_t next_grapheme(const seg_internal_t* in, size_t start) {
    size_t i = start + 1;
    if (i > in->count) return in->count;

    uint32_t prev = in->cps[start];
    uint32_t prev_g = seg_gcb(prev);
    size_t prev_ri_pos = seg_is_regional_indicator_cp(prev) ? 0u : (size_t)-1u;
    bool emoji_context = seg_is_extended_pictographic_cp(prev);
    bool emoji_zwj_pending = false;
    /* GB9c (Unicode 18.0.0): ConjunctLinker ConjunctExtender* x LinkingConsonant. */
    bool in_linker_conjunct = (seg_incb(prev) == (uint32_t)UNICODOID_INCB_LINKER);

    while (i < in->count) {
        uint32_t cur = in->cps[i];
        uint32_t cur_g = seg_gcb(cur);
        uint32_t cur_incb = seg_incb(cur);
        bool boundary = true;

        /* Grapheme cluster boundary rules GB3..GB13, applied in order. */
        if (prev == 0x000D && cur == 0x000A) {
            /* GB3: CR × LF (no boundary). */
            boundary = false;
        } else if (seg_is_control_cp(prev) || seg_is_control_cp(cur)) {
            /* GB4 / GB5: break before and after controls. */
            boundary = true;
        } else if (prev_g == (uint32_t)UNICODOID_GCB_L &&
                   (cur_g == (uint32_t)UNICODOID_GCB_L ||
                    cur_g == (uint32_t)UNICODOID_GCB_V ||
                    cur_g == (uint32_t)UNICODOID_GCB_LV ||
                    cur_g == (uint32_t)UNICODOID_GCB_LVT)) {
            /* GB6: L × (L | V | LV | LVT). */
            boundary = false;
        } else if ((prev_g == (uint32_t)UNICODOID_GCB_LV ||
                    prev_g == (uint32_t)UNICODOID_GCB_V) &&
                   (cur_g == (uint32_t)UNICODOID_GCB_V ||
                    cur_g == (uint32_t)UNICODOID_GCB_T)) {
            /* GB7: (LV | V) × (V | T). */
            boundary = false;
        } else if ((prev_g == (uint32_t)UNICODOID_GCB_LVT ||
                    prev_g == (uint32_t)UNICODOID_GCB_T) &&
                   cur_g == (uint32_t)UNICODOID_GCB_T) {
            /* GB8: (LVT | T) × T. */
            boundary = false;
        } else if (cur_g == (uint32_t)UNICODOID_GCB_EXTEND || cur_g == (uint32_t)UNICODOID_GCB_ZWJ) {
            /* GB9: × (Extend | ZWJ). */
            boundary = false;
        } else if (cur_g == (uint32_t)UNICODOID_GCB_SPACINGMARK) {
            /* GB9a: × SpacingMark. */
            boundary = false;
        } else if (prev_g == (uint32_t)UNICODOID_GCB_PREPEND) {
            /* GB9b: Prepend ×. */
            boundary = false;
        } else if (cur_incb == (uint32_t)UNICODOID_INCB_CONSONANT && in_linker_conjunct) {
            /* GB9c: InCB=Linker InCB=Extend* × InCB=Consonant. */
            boundary = false;
        } else if (emoji_zwj_pending && seg_is_extended_pictographic_cp(cur)) {
            /* GB11: ExtPict Extend* ZWJ × ExtPict. */
            boundary = false;
        } else if (cur_g == (uint32_t)UNICODOID_GCB_RI) {
            /* GB12 / GB13: regional indicator pairing. */
            size_t cur_ri_pos = (prev_g == (uint32_t)UNICODOID_GCB_RI) ? (prev_ri_pos + 1u) : 0u;
            if ((cur_ri_pos % 2u) == 1u) boundary = false;
        }

        /* GB999: Any ÷ Any (default break). */
        if (boundary) return i;

        /* Consume the current character and update cluster context. */
        if (cur_g == (uint32_t)UNICODOID_GCB_RI) {
            prev_ri_pos = (prev_g == (uint32_t)UNICODOID_GCB_RI) ? (prev_ri_pos + 1u) : 0u;
        } else {
            prev_ri_pos = (size_t)-1u;
        }

        if (cur == 0x200Du) {
            if (emoji_context) emoji_zwj_pending = true;
        } else if (cur_g == (uint32_t)UNICODOID_GCB_EXTEND) {
            /* Extends stay inside the current emoji context. */
        } else {
            emoji_context = seg_is_extended_pictographic_cp(cur);
            emoji_zwj_pending = false;
        }

        if (cur_incb == (uint32_t)UNICODOID_INCB_LINKER) {
            in_linker_conjunct = true;
        } else if (cur_incb == (uint32_t)UNICODOID_INCB_EXTEND) {
            /* ConjunctExtender* keeps the linker context alive. */
        } else {
            in_linker_conjunct = false;
        }

        prev = cur;
        prev_g = cur_g;
        ++i;
    }
    return in->count;
}

/* ------------------------------------------------------------------------- */
/* UAX #29 word breaking (Unicode 18.0.0)                                   */
/*                                                                           */
/* WB4 remaps every Extend / Format / ZWJ following a character that is not  */
/* CR, LF, or Newline to that base character and then ignores it.  This is   */
/* modelled by folding the base and its following Extend / Format / ZWJ run  */
/* into one unit.  All remaining rules are evaluated between units.          */
/* ------------------------------------------------------------------------- */

typedef struct wb_unit {
    size_t   start;
    size_t   end;
    uint32_t cls;          /* Word_Break of the base code point */
    uint32_t last_cls;     /* Word_Break of the last code point in the unit */
    bool     ends_with_zwj;/* last code point is ZWJ */
    bool     extpict;      /* base code point is Extended_Pictographic */
} wb_unit_t;

typedef struct wb_units {
    wb_unit_t* items;
    size_t     count;
} wb_units_t;

static bool wb_is_newline(uint32_t cls) {
    return cls == (uint32_t)UNICODOID_WB_CR ||
           cls == (uint32_t)UNICODOID_WB_LF ||
           cls == (uint32_t)UNICODOID_WB_NEWLINE;
}

static bool wb_is_ahletter(uint32_t cls) {
    return cls == (uint32_t)UNICODOID_WB_ALETTER ||
           cls == (uint32_t)UNICODOID_WB_HEBREW_LETTER;
}

/* MidLetter | MidNumLetQ (MidNumLet | Single_Quote). */
static bool wb_is_midletterq(uint32_t cls) {
    return cls == (uint32_t)UNICODOID_WB_MIDLETTER ||
           cls == (uint32_t)UNICODOID_WB_MIDNUMLET ||
           cls == (uint32_t)UNICODOID_WB_SINGLE_QUOTE;
}

/* MidNum | MidNumLetQ (MidNumLet | Single_Quote). */
static bool wb_is_midnumq(uint32_t cls) {
    return cls == (uint32_t)UNICODOID_WB_MIDNUM ||
           cls == (uint32_t)UNICODOID_WB_MIDNUMLET ||
           cls == (uint32_t)UNICODOID_WB_SINGLE_QUOTE;
}

static int wb_build_units(const seg_internal_t* in, wb_units_t* out) {
    size_t capacity = in->count ? in->count : 1;
    out->items = (wb_unit_t*)malloc(capacity * sizeof(wb_unit_t));
    if (!out->items) return -1;
    out->count = 0;

    size_t i = 0;
    while (i < in->count) {
        uint32_t cls = seg_wb(in->cps[i]);
        wb_unit_t u;
        memset(&u, 0, sizeof(u));
        u.start = i;
        u.cls = cls;
        u.last_cls = cls;
        u.extpict = seg_is_extended_pictographic_cp(in->cps[i]);

        size_t j = i + 1;
        if (!wb_is_newline(cls)) {
            while (j < in->count) {
                uint32_t c2 = seg_wb(in->cps[j]);
                if (c2 != (uint32_t)UNICODOID_WB_EXTEND &&
                    c2 != (uint32_t)UNICODOID_WB_FORMAT &&
                    c2 != (uint32_t)UNICODOID_WB_ZWJ) {
                    break;
                }
                u.last_cls = c2;
                ++j;
            }
        }
        u.ends_with_zwj = (u.last_cls == (uint32_t)UNICODOID_WB_ZWJ);

        u.end = j;
        out->items[out->count++] = u;
        i = j;
    }
    return 0;
}

static bool wb_break_before(const wb_units_t* u, size_t i) {
    if (i == 0) return false;             /* WB1 (leading sot break is implicit) */
    if (i >= u->count) return true;       /* WB2: ÷ eot */

    const wb_unit_t* prev = &u->items[i - 1];
    const wb_unit_t* cur  = &u->items[i];
    uint32_t p = prev->cls;
    uint32_t c = cur->cls;

    /* WB3: CR × LF */
    if (p == (uint32_t)UNICODOID_WB_CR && c == (uint32_t)UNICODOID_WB_LF) return false;
    /* WB3.1: (Newline | CR | LF) ÷ */
    if (wb_is_newline(p)) return true;
    /* WB3.2: ÷ (Newline | CR | LF) */
    if (wb_is_newline(c)) return true;
    /* WB3.3: ZWJ × ExtPict */
    if (prev->ends_with_zwj && cur->extpict) return false;
    /* WB3.4: WSegSpace × WSegSpace (evaluated on raw characters, i.e. before
       WB4 folds any following Extend / Format / ZWJ into the space). */
    if (prev->last_cls == (uint32_t)UNICODOID_WB_WSEGSPACE &&
        c == (uint32_t)UNICODOID_WB_WSEGSPACE) return false;
    /* WB5: AHLetter × AHLetter */
    if (wb_is_ahletter(p) && wb_is_ahletter(c)) return false;
    /* WB6: AHLetter × (MidLetter | MidNumLetQ) AHLetter */
    if (wb_is_ahletter(p) && wb_is_midletterq(c) &&
        i + 1 < u->count && wb_is_ahletter(u->items[i + 1].cls)) return false;
    /* WB7: AHLetter (MidLetter | MidNumLetQ) × AHLetter */
    if (wb_is_midletterq(p) && wb_is_ahletter(c) &&
        i >= 2 && wb_is_ahletter(u->items[i - 2].cls)) return false;
    /* WB7.1: Hebrew_Letter × Single_Quote */
    if (p == (uint32_t)UNICODOID_WB_HEBREW_LETTER &&
        c == (uint32_t)UNICODOID_WB_SINGLE_QUOTE) return false;
    /* WB7.2: Hebrew_Letter × Double_Quote Hebrew_Letter */
    if (p == (uint32_t)UNICODOID_WB_HEBREW_LETTER &&
        c == (uint32_t)UNICODOID_WB_DOUBLE_QUOTE &&
        i + 1 < u->count &&
        u->items[i + 1].cls == (uint32_t)UNICODOID_WB_HEBREW_LETTER) return false;
    /* WB7.3: Hebrew_Letter Double_Quote × Hebrew_Letter */
    if (p == (uint32_t)UNICODOID_WB_DOUBLE_QUOTE &&
        c == (uint32_t)UNICODOID_WB_HEBREW_LETTER &&
        i >= 2 &&
        u->items[i - 2].cls == (uint32_t)UNICODOID_WB_HEBREW_LETTER) return false;
    /* WB8: Numeric × Numeric */
    if (p == (uint32_t)UNICODOID_WB_NUMERIC &&
        c == (uint32_t)UNICODOID_WB_NUMERIC) return false;
    /* WB9: AHLetter × Numeric */
    if (wb_is_ahletter(p) && c == (uint32_t)UNICODOID_WB_NUMERIC) return false;
    /* WB10: Numeric × AHLetter */
    if (p == (uint32_t)UNICODOID_WB_NUMERIC && wb_is_ahletter(c)) return false;
    /* WB11: Numeric (MidNum | MidNumLetQ) × Numeric */
    if (wb_is_midnumq(p) && c == (uint32_t)UNICODOID_WB_NUMERIC &&
        i >= 2 && u->items[i - 2].cls == (uint32_t)UNICODOID_WB_NUMERIC) return false;
    /* WB12: Numeric × (MidNum | MidNumLetQ) Numeric */
    if (p == (uint32_t)UNICODOID_WB_NUMERIC && wb_is_midnumq(c) &&
        i + 1 < u->count &&
        u->items[i + 1].cls == (uint32_t)UNICODOID_WB_NUMERIC) return false;
    /* WB13: Katakana × Katakana */
    if (p == (uint32_t)UNICODOID_WB_KATAKANA &&
        c == (uint32_t)UNICODOID_WB_KATAKANA) return false;
    /* WB13.1: (AHLetter | Numeric | Katakana | ExtendNumLet) × ExtendNumLet */
    if ((wb_is_ahletter(p) || p == (uint32_t)UNICODOID_WB_NUMERIC ||
         p == (uint32_t)UNICODOID_WB_KATAKANA ||
         p == (uint32_t)UNICODOID_WB_EXTENDNUMLET) &&
        c == (uint32_t)UNICODOID_WB_EXTENDNUMLET) return false;
    /* WB13.2: ExtendNumLet × (AHLetter | Numeric | Katakana) */
    if (p == (uint32_t)UNICODOID_WB_EXTENDNUMLET &&
        (wb_is_ahletter(c) || c == (uint32_t)UNICODOID_WB_NUMERIC ||
         c == (uint32_t)UNICODOID_WB_KATAKANA)) return false;
    /* WB15 / WB16: regional indicator pairs */
    if (p == (uint32_t)UNICODOID_WB_RI && c == (uint32_t)UNICODOID_WB_RI) {
        size_t count = 0;
        size_t k = i;
        while (k > 0 && u->items[k - 1].cls == (uint32_t)UNICODOID_WB_RI) {
            ++count;
            --k;
        }
        if ((count % 2u) == 1u) return false;
    }
    /* WB999: Any ÷ Any */
    return true;
}

static size_t next_word(const seg_internal_t* in, size_t start) {
    if (in->count == 0) return 0;

    wb_units_t units;
    if (wb_build_units(in, &units) != 0) {
        return (start + 1 <= in->count) ? start + 1 : in->count;
    }

    size_t first = units.count;
    for (size_t k = 0; k < units.count; ++k) {
        if (units.items[k].start == start) { first = k; break; }
    }

    size_t end = in->count;
    for (size_t k = first + 1; k <= units.count; ++k) {
        if (wb_break_before(&units, k)) {
            end = (k < units.count) ? units.items[k].start : in->count;
            break;
        }
    }

    free(units.items);
    if (end <= start) end = start + 1;
    if (end > in->count) end = in->count;
    return end;
}

/* ------------------------------------------------------------------------- */
/* UAX #14 line breaking (Unicode 18.0.0, revision 57)                      */
/*                                                                           */
/* LB9 is applied by folding every base character together with the         */
/* following CM / ZWJ run into a single "unit".  Remaining CM / ZWJ         */
/* characters (LB10) become one-character AL units.  All line break rules    */
/* are then evaluated between units, which is exactly the "ignore affected  */
/* CM / ZWJ" requirement of LB9.                                            */
/* ------------------------------------------------------------------------- */

typedef struct lb_unit {
    size_t   start;        /* first code point index */
    size_t   end;          /* one past the last code point index */
    uint32_t cls;          /* resolved Line_Break class */
    uint8_t  gc;           /* General_Category of the base code point */
    uint8_t  eaw;          /* East_Asian_Width of the base code point */
    bool     east_asian;   /* EAW in {F, W, H} */
    bool     dotted;       /* base code point is U+25CC DOTTED CIRCLE */
    bool     extpict_cn;   /* Extended_Pictographic && General_Category=Cn */
    bool     ends_with_zwj;/* last code point of the unit is ZWJ */
} lb_unit_t;

typedef struct lb_units {
    lb_unit_t* items;
    size_t     count;
} lb_units_t;

/* LB1: resolve AI, SG, XX, SA, and CJ to their working classes. */
static uint32_t lb_resolve_class(uint32_t cp) {
    uint32_t lb = seg_lb(cp);
    uint8_t  gc = unicodoid_ucd_general_category(cp);

    switch (lb) {
        case UNICODOID_LB_AI:
        case UNICODOID_LB_SG:
        case UNICODOID_LB_XX:
            return (uint32_t)UNICODOID_LB_AL;
        case UNICODOID_LB_SA:
            return (gc == (uint8_t)UNICODOID_GC_MN || gc == (uint8_t)UNICODOID_GC_MC)
                       ? (uint32_t)UNICODOID_LB_CM
                       : (uint32_t)UNICODOID_LB_AL;
        case UNICODOID_LB_CJ:
            return (uint32_t)UNICODOID_LB_NS;
        default:
            return lb;
    }
}

static bool lb_is_excluded_from_lb9(uint32_t cls) {
    return cls == (uint32_t)UNICODOID_LB_BK ||
           cls == (uint32_t)UNICODOID_LB_CR ||
           cls == (uint32_t)UNICODOID_LB_LF ||
           cls == (uint32_t)UNICODOID_LB_NL ||
           cls == (uint32_t)UNICODOID_LB_SP ||
           cls == (uint32_t)UNICODOID_LB_ZW;
}

static int lb_build_units(const seg_internal_t* in, lb_units_t* out) {
    size_t capacity = in->count ? in->count : 1;
    out->items = (lb_unit_t*)malloc(capacity * sizeof(lb_unit_t));
    if (!out->items) return -1;
    out->count = 0;

    size_t i = 0;
    while (i < in->count) {
        uint32_t cp  = in->cps[i];
        uint32_t cls = lb_resolve_class(cp);
        lb_unit_t u;
        memset(&u, 0, sizeof(u));
        u.start = i;
        u.gc    = unicodoid_ucd_general_category(cp);
        u.eaw   = unicodoid_ucd_east_asian_width(cp);
        u.east_asian = (u.eaw == 2u || u.eaw == 3u || u.eaw == 5u);
        u.dotted = (cp == 0x25CCu);
        u.extpict_cn = (u.gc == (uint8_t)UNICODOID_GC_CN) &&
                       unicodoid_ucd_has_binary_property(
                           cp, UNICODOID_BP_EXTENDED_PICTOGRAPHIC);

        size_t j = i + 1;
        if (cls == (uint32_t)UNICODOID_LB_CM || cls == (uint32_t)UNICODOID_LB_ZWJ) {
            /* LB10: a remaining combining mark / ZWJ behaves like AL.
               Following CM / ZWJ characters are also "remaining", so they
               form their own AL units (the resulting break decisions are
               identical because AL x AL is prohibited by LB28). */
            u.cls = (uint32_t)UNICODOID_LB_AL;
            u.ends_with_zwj = (cls == (uint32_t)UNICODOID_LB_ZWJ);
        } else if (lb_is_excluded_from_lb9(cls)) {
            /* LB9 does not absorb CM / ZWJ after these classes. */
            u.cls = cls;
        } else {
            /* LB9: treat X (CM | ZWJ)* as X. */
            u.cls = cls;
            bool last_zwj = false;
            while (j < in->count) {
                uint32_t c2cls = lb_resolve_class(in->cps[j]);
                if (c2cls != (uint32_t)UNICODOID_LB_CM &&
                    c2cls != (uint32_t)UNICODOID_LB_ZWJ) {
                    break;
                }
                last_zwj = (c2cls == (uint32_t)UNICODOID_LB_ZWJ);
                ++j;
            }
            u.ends_with_zwj = last_zwj;
        }

        u.end = j;
        out->items[out->count++] = u;
        i = j;
    }
    return 0;
}

/* True when the units immediately before index k end with NU (SY | IS)*. */
static bool lb_nu_seq_before(const lb_units_t* u, size_t k) {
    size_t j = k;
    while (j > 0 &&
           (u->items[j - 1].cls == (uint32_t)UNICODOID_LB_SY ||
            u->items[j - 1].cls == (uint32_t)UNICODOID_LB_IS)) {
        --j;
    }
    return j > 0 && u->items[j - 1].cls == (uint32_t)UNICODOID_LB_NU;
}

/* Decide whether a line break is allowed immediately before unit i. */
static bool lb_break_before(const lb_units_t* u, size_t i) {
    if (i == 0) return false;              /* LB2: sot x */
    if (i >= u->count) return true;        /* LB3: ! eot */

    const lb_unit_t* prev = &u->items[i - 1];
    const lb_unit_t* cur  = &u->items[i];
    uint32_t p = prev->cls;
    uint32_t c = cur->cls;

    /* LB4: BK ! */
    if (p == (uint32_t)UNICODOID_LB_BK) return true;
    /* LB5: CR x LF, then CR / LF / NL ! */
    if (p == (uint32_t)UNICODOID_LB_CR && c == (uint32_t)UNICODOID_LB_LF) return false;
    if (p == (uint32_t)UNICODOID_LB_CR ||
        p == (uint32_t)UNICODOID_LB_LF ||
        p == (uint32_t)UNICODOID_LB_NL) return true;
    /* LB6: x (BK | CR | LF | NL) */
    if (c == (uint32_t)UNICODOID_LB_BK || c == (uint32_t)UNICODOID_LB_CR ||
        c == (uint32_t)UNICODOID_LB_LF || c == (uint32_t)UNICODOID_LB_NL) return false;
    /* LB7: x SP, x ZW */
    if (c == (uint32_t)UNICODOID_LB_SP || c == (uint32_t)UNICODOID_LB_ZW) return false;
    /* LB8: ZW SP* ! */
    {
        size_t k = i;
        while (k > 0 && u->items[k - 1].cls == (uint32_t)UNICODOID_LB_SP) --k;
        if (k > 0 && u->items[k - 1].cls == (uint32_t)UNICODOID_LB_ZW) return true;
    }
    /* LB8a: ZWJ x */
    if (prev->ends_with_zwj) return false;
    /* LB11: x WJ, WJ x */
    if (c == (uint32_t)UNICODOID_LB_WJ || p == (uint32_t)UNICODOID_LB_WJ) return false;
    /* LB12: GL x */
    if (p == (uint32_t)UNICODOID_LB_GL) return false;
    /* LB12a: [^SP HY HH] x GL */
    if (c == (uint32_t)UNICODOID_LB_GL &&
        p != (uint32_t)UNICODOID_LB_SP &&
        p != (uint32_t)UNICODOID_LB_HY &&
        p != (uint32_t)UNICODOID_LB_HH) return false;
    /* LB13: x CL, x CP, x EX, x SY */
    if (c == (uint32_t)UNICODOID_LB_CL || c == (uint32_t)UNICODOID_LB_CP ||
        c == (uint32_t)UNICODOID_LB_EX || c == (uint32_t)UNICODOID_LB_SY) return false;
    /* LB14: OP SP* x */
    {
        size_t k = i;
        while (k > 0 && u->items[k - 1].cls == (uint32_t)UNICODOID_LB_SP) --k;
        if (k > 0 && u->items[k - 1].cls == (uint32_t)UNICODOID_LB_OP) return false;
    }
    /* LB15a: (sot | BK | CR | LF | NL | OP | QU | GL | SP | ZW) QU_Pi SP* x */
    {
        size_t k = i;
        while (k > 0 && u->items[k - 1].cls == (uint32_t)UNICODOID_LB_SP) --k;
        if (k > 0 &&
            u->items[k - 1].cls == (uint32_t)UNICODOID_LB_QU &&
            u->items[k - 1].gc == (uint8_t)UNICODOID_GC_PI) {
            if (k - 1 == 0) return false;  /* sot QU_Pi SP* x */
            uint32_t pc = u->items[k - 2].cls;
            if (pc == (uint32_t)UNICODOID_LB_BK || pc == (uint32_t)UNICODOID_LB_CR ||
                pc == (uint32_t)UNICODOID_LB_LF || pc == (uint32_t)UNICODOID_LB_NL ||
                pc == (uint32_t)UNICODOID_LB_OP || pc == (uint32_t)UNICODOID_LB_QU ||
                pc == (uint32_t)UNICODOID_LB_GL || pc == (uint32_t)UNICODOID_LB_SP ||
                pc == (uint32_t)UNICODOID_LB_ZW) return false;
        }
    }
    /* LB15b: x QU_Pf (SP | GL | WJ | CL | QU | CP | EX | IS | SY | BK | CR | LF | NL | ZW | eot) */
    if (c == (uint32_t)UNICODOID_LB_QU && cur->gc == (uint8_t)UNICODOID_GC_PF) {
        if (i + 1 >= u->count) return false;
        uint32_t nc = u->items[i + 1].cls;
        if (nc == (uint32_t)UNICODOID_LB_SP || nc == (uint32_t)UNICODOID_LB_GL ||
            nc == (uint32_t)UNICODOID_LB_WJ || nc == (uint32_t)UNICODOID_LB_CL ||
            nc == (uint32_t)UNICODOID_LB_QU || nc == (uint32_t)UNICODOID_LB_CP ||
            nc == (uint32_t)UNICODOID_LB_EX || nc == (uint32_t)UNICODOID_LB_IS ||
            nc == (uint32_t)UNICODOID_LB_SY || nc == (uint32_t)UNICODOID_LB_BK ||
            nc == (uint32_t)UNICODOID_LB_CR || nc == (uint32_t)UNICODOID_LB_LF ||
            nc == (uint32_t)UNICODOID_LB_NL || nc == (uint32_t)UNICODOID_LB_ZW) return false;
    }
    /* LB15c: SP ! IS NU */
    if (p == (uint32_t)UNICODOID_LB_SP && c == (uint32_t)UNICODOID_LB_IS &&
        i + 1 < u->count && u->items[i + 1].cls == (uint32_t)UNICODOID_LB_NU) return true;
    /* LB15d: x IS */
    if (c == (uint32_t)UNICODOID_LB_IS) return false;
    /* LB16: (CL | CP) SP* x NS */
    if (c == (uint32_t)UNICODOID_LB_NS) {
        size_t k = i;
        while (k > 0 && u->items[k - 1].cls == (uint32_t)UNICODOID_LB_SP) --k;
        if (k > 0 && (u->items[k - 1].cls == (uint32_t)UNICODOID_LB_CL ||
                      u->items[k - 1].cls == (uint32_t)UNICODOID_LB_CP)) return false;
    }
    /* LB17: B2 SP* x B2 */
    if (c == (uint32_t)UNICODOID_LB_B2) {
        size_t k = i;
        while (k > 0 && u->items[k - 1].cls == (uint32_t)UNICODOID_LB_SP) --k;
        if (k > 0 && u->items[k - 1].cls == (uint32_t)UNICODOID_LB_B2) return false;
    }
    /* LB18: SP ! */
    if (p == (uint32_t)UNICODOID_LB_SP) return true;
    /* LB19: x QUmPi, QUmPf x */
    if (c == (uint32_t)UNICODOID_LB_QU && cur->gc != (uint8_t)UNICODOID_GC_PI) return false;
    if (p == (uint32_t)UNICODOID_LB_QU && prev->gc != (uint8_t)UNICODOID_GC_PF) return false;
    /* LB19a: East Asian quotation tailoring */
    if (c == (uint32_t)UNICODOID_LB_QU) {
        if (!prev->east_asian) return false;              /* [^EA] x QU */
        if (i + 1 >= u->count) return false;              /* x QU eot */
        if (!u->items[i + 1].east_asian) return false;    /* x QU [^EA] */
    }
    if (p == (uint32_t)UNICODOID_LB_QU && !cur->east_asian) return false; /* QU x [^EA] */
    if (p == (uint32_t)UNICODOID_LB_QU) {                 /* (sot | [^EA]) QU x */
        if (i - 1 == 0) return false;
        if (!u->items[i - 2].east_asian) return false;
    }
    /* LB20: ! CB, CB ! */
    if (c == (uint32_t)UNICODOID_LB_CB || p == (uint32_t)UNICODOID_LB_CB) return true;
    /* LB20a: (sot | BK | CR | LF | NL | SP | ZW | CB | GL) (HY | HH) x (AL | HL) */
    if ((p == (uint32_t)UNICODOID_LB_HY || p == (uint32_t)UNICODOID_LB_HH) &&
        (c == (uint32_t)UNICODOID_LB_AL || c == (uint32_t)UNICODOID_LB_HL)) {
        if (i - 1 == 0) return false;
        uint32_t pc = u->items[i - 2].cls;
        if (pc == (uint32_t)UNICODOID_LB_BK || pc == (uint32_t)UNICODOID_LB_CR ||
            pc == (uint32_t)UNICODOID_LB_LF || pc == (uint32_t)UNICODOID_LB_NL ||
            pc == (uint32_t)UNICODOID_LB_SP || pc == (uint32_t)UNICODOID_LB_ZW ||
            pc == (uint32_t)UNICODOID_LB_CB || pc == (uint32_t)UNICODOID_LB_GL) return false;
    }
    /* LB21: x BA, x HH, x HY, x NS, BB x */
    if (c == (uint32_t)UNICODOID_LB_BA || c == (uint32_t)UNICODOID_LB_HH ||
        c == (uint32_t)UNICODOID_LB_HY || c == (uint32_t)UNICODOID_LB_NS) return false;
    if (p == (uint32_t)UNICODOID_LB_BB) return false;
    /* LB21a: HL (HY | HH) x [^HL] */
    if ((p == (uint32_t)UNICODOID_LB_HY || p == (uint32_t)UNICODOID_LB_HH) &&
        i >= 2 && u->items[i - 2].cls == (uint32_t)UNICODOID_LB_HL &&
        c != (uint32_t)UNICODOID_LB_HL) return false;
    /* LB21b: SY x HL */
    if (p == (uint32_t)UNICODOID_LB_SY && c == (uint32_t)UNICODOID_LB_HL) return false;
    /* LB22: x IN */
    if (c == (uint32_t)UNICODOID_LB_IN) return false;
    /* LB23: (AL | HL) x NU, NU x (AL | HL) */
    if ((p == (uint32_t)UNICODOID_LB_AL || p == (uint32_t)UNICODOID_LB_HL) &&
        c == (uint32_t)UNICODOID_LB_NU) return false;
    if (p == (uint32_t)UNICODOID_LB_NU &&
        (c == (uint32_t)UNICODOID_LB_AL || c == (uint32_t)UNICODOID_LB_HL)) return false;
    /* LB23a: PR x (ID | EB | EM), (ID | EB | EM) x PO */
    if (p == (uint32_t)UNICODOID_LB_PR &&
        (c == (uint32_t)UNICODOID_LB_ID || c == (uint32_t)UNICODOID_LB_EB ||
         c == (uint32_t)UNICODOID_LB_EM)) return false;
    if ((p == (uint32_t)UNICODOID_LB_ID || p == (uint32_t)UNICODOID_LB_EB ||
         p == (uint32_t)UNICODOID_LB_EM) && c == (uint32_t)UNICODOID_LB_PO) return false;
    /* LB24: (PR | PO) x (AL | HL), (AL | HL) x (PR | PO) */
    if ((p == (uint32_t)UNICODOID_LB_PR || p == (uint32_t)UNICODOID_LB_PO) &&
        (c == (uint32_t)UNICODOID_LB_AL || c == (uint32_t)UNICODOID_LB_HL)) return false;
    if ((p == (uint32_t)UNICODOID_LB_AL || p == (uint32_t)UNICODOID_LB_HL) &&
        (c == (uint32_t)UNICODOID_LB_PR || c == (uint32_t)UNICODOID_LB_PO)) return false;
    /* LB25: numeric sequences */
    if (c == (uint32_t)UNICODOID_LB_PO || c == (uint32_t)UNICODOID_LB_PR) {
        if ((p == (uint32_t)UNICODOID_LB_CL || p == (uint32_t)UNICODOID_LB_CP) &&
            lb_nu_seq_before(u, i - 1)) return false;
        if (lb_nu_seq_before(u, i)) return false;
    }
    if (p == (uint32_t)UNICODOID_LB_PO || p == (uint32_t)UNICODOID_LB_PR) {
        if (c == (uint32_t)UNICODOID_LB_OP) {
            if (i + 1 < u->count && u->items[i + 1].cls == (uint32_t)UNICODOID_LB_NU) return false;
            if (i + 2 < u->count &&
                u->items[i + 1].cls == (uint32_t)UNICODOID_LB_IS &&
                u->items[i + 2].cls == (uint32_t)UNICODOID_LB_NU) return false;
        }
        if (c == (uint32_t)UNICODOID_LB_NU) return false;
    }
    if (p == (uint32_t)UNICODOID_LB_HY && c == (uint32_t)UNICODOID_LB_NU) return false;
    if (p == (uint32_t)UNICODOID_LB_IS && c == (uint32_t)UNICODOID_LB_NU) return false;
    if (c == (uint32_t)UNICODOID_LB_NU && lb_nu_seq_before(u, i)) return false;
    /* LB26: Korean syllables */
    if (p == (uint32_t)UNICODOID_LB_JL &&
        (c == (uint32_t)UNICODOID_LB_JL || c == (uint32_t)UNICODOID_LB_JV ||
         c == (uint32_t)UNICODOID_LB_H2 || c == (uint32_t)UNICODOID_LB_H3)) return false;
    if ((p == (uint32_t)UNICODOID_LB_JV || p == (uint32_t)UNICODOID_LB_H2) &&
        (c == (uint32_t)UNICODOID_LB_JV || c == (uint32_t)UNICODOID_LB_JT)) return false;
    if ((p == (uint32_t)UNICODOID_LB_JT || p == (uint32_t)UNICODOID_LB_H3) &&
        c == (uint32_t)UNICODOID_LB_JT) return false;
    /* LB27: Korean syllable blocks behave like ID */
    if ((p == (uint32_t)UNICODOID_LB_JL || p == (uint32_t)UNICODOID_LB_JV ||
         p == (uint32_t)UNICODOID_LB_JT || p == (uint32_t)UNICODOID_LB_H2 ||
         p == (uint32_t)UNICODOID_LB_H3) && c == (uint32_t)UNICODOID_LB_PO) return false;
    if (p == (uint32_t)UNICODOID_LB_PR &&
        (c == (uint32_t)UNICODOID_LB_JL || c == (uint32_t)UNICODOID_LB_JV ||
         c == (uint32_t)UNICODOID_LB_JT || c == (uint32_t)UNICODOID_LB_H2 ||
         c == (uint32_t)UNICODOID_LB_H3)) return false;
    /* LB28: (AL | HL) x (AL | HL) */
    if ((p == (uint32_t)UNICODOID_LB_AL || p == (uint32_t)UNICODOID_LB_HL) &&
        (c == (uint32_t)UNICODOID_LB_AL || c == (uint32_t)UNICODOID_LB_HL)) return false;
    /* LB28a: Brahmic orthographic syllables */
    {
        bool prev_ak = (p == (uint32_t)UNICODOID_LB_AK || prev->dotted ||
                        p == (uint32_t)UNICODOID_LB_AS);
        bool cur_ak  = (c == (uint32_t)UNICODOID_LB_AK || cur->dotted ||
                        c == (uint32_t)UNICODOID_LB_AS);
        if (p == (uint32_t)UNICODOID_LB_AP && cur_ak) return false;
        if (prev_ak && (c == (uint32_t)UNICODOID_LB_VF ||
                        c == (uint32_t)UNICODOID_LB_VI)) return false;
        if (p == (uint32_t)UNICODOID_LB_VI &&
            (c == (uint32_t)UNICODOID_LB_AK || cur->dotted) && i >= 2) {
            const lb_unit_t* p2 = &u->items[i - 2];
            if (p2->cls == (uint32_t)UNICODOID_LB_AK || p2->dotted ||
                p2->cls == (uint32_t)UNICODOID_LB_AS) return false;
        }
        if (prev_ak && cur_ak && i + 1 < u->count &&
            u->items[i + 1].cls == (uint32_t)UNICODOID_LB_VF) return false;
    }
    /* LB29: IS x (AL | HL) */
    if (p == (uint32_t)UNICODOID_LB_IS &&
        (c == (uint32_t)UNICODOID_LB_AL || c == (uint32_t)UNICODOID_LB_HL)) return false;
    /* LB30: (AL | HL | NU) x OPmEastAsian, CPmEastAsian x (AL | HL | NU) */
    if ((p == (uint32_t)UNICODOID_LB_AL || p == (uint32_t)UNICODOID_LB_HL ||
         p == (uint32_t)UNICODOID_LB_NU) &&
        c == (uint32_t)UNICODOID_LB_OP && !cur->east_asian) return false;
    if (p == (uint32_t)UNICODOID_LB_CP && !prev->east_asian &&
        (c == (uint32_t)UNICODOID_LB_AL || c == (uint32_t)UNICODOID_LB_HL ||
         c == (uint32_t)UNICODOID_LB_NU)) return false;
    /* LB30a: regional indicator pairs */
    if (p == (uint32_t)UNICODOID_LB_RI && c == (uint32_t)UNICODOID_LB_RI) {
        size_t count = 0;
        size_t k = i;
        while (k > 0 && u->items[k - 1].cls == (uint32_t)UNICODOID_LB_RI) {
            ++count;
            --k;
        }
        if ((count % 2u) == 1u) return false;
    }
    /* LB30b: EB x EM, ExtPictUnassigned x EM */
    if (c == (uint32_t)UNICODOID_LB_EM &&
        (p == (uint32_t)UNICODOID_LB_EB || prev->extpict_cn)) return false;
    /* LB31: break everywhere else. */
    return true;
}

static size_t next_line(const seg_internal_t* in, size_t start) {
    if (in->count == 0) return 0;

    lb_units_t units;
    if (lb_build_units(in, &units) != 0) {
        return (start + 1 <= in->count) ? start + 1 : in->count;
    }

    size_t first = units.count;
    for (size_t k = 0; k < units.count; ++k) {
        if (units.items[k].start == start) { first = k; break; }
    }

    size_t end = in->count;
    for (size_t k = first + 1; k <= units.count; ++k) {
        if (lb_break_before(&units, k)) {
            end = (k < units.count) ? units.items[k].start : in->count;
            break;
        }
    }

    free(units.items);
    if (end <= start) end = start + 1;
    if (end > in->count) end = in->count;
    return end;
}

static bool sb_is_para(uint32_t sb) {
    return sb == (uint32_t)UNICODOID_SB_SEP ||
           sb == (uint32_t)UNICODOID_SB_CR ||
           sb == (uint32_t)UNICODOID_SB_LF;
}

/* ------------------------------------------------------------------------- */
/* UAX #29 sentence breaking (Unicode 18.0.0)                               */
/*                                                                           */
/* SB5 ignores Extend / Format characters following a base character that is */
/* not a paragraph separator.  Folding them into the base makes the SB rules */
/* directly expressible between units.                                       */
/* ------------------------------------------------------------------------- */

typedef struct sb_unit {
    size_t   start;
    size_t   end;
    uint32_t cls;
} sb_unit_t;

typedef struct sb_units {
    sb_unit_t* items;
    size_t     count;
} sb_units_t;

static bool sb_is_saterm(uint32_t cls) {
    return cls == (uint32_t)UNICODOID_SB_STERM ||
           cls == (uint32_t)UNICODOID_SB_ATERM;
}

static int sb_build_units(const seg_internal_t* in, sb_units_t* out) {
    size_t capacity = in->count ? in->count : 1;
    out->items = (sb_unit_t*)malloc(capacity * sizeof(sb_unit_t));
    if (!out->items) return -1;
    out->count = 0;

    size_t i = 0;
    while (i < in->count) {
        uint32_t cls = seg_sb(in->cps[i]);
        sb_unit_t u;
        memset(&u, 0, sizeof(u));
        u.start = i;
        u.cls = cls;

        size_t j = i + 1;
        if (!sb_is_para(cls)) {
            /* SB5: X (Extend | Format)* -> X. */
            while (j < in->count) {
                uint32_t c2 = seg_sb(in->cps[j]);
                if (c2 != (uint32_t)UNICODOID_SB_EXTEND &&
                    c2 != (uint32_t)UNICODOID_SB_FORMAT) {
                    break;
                }
                ++j;
            }
        }

        u.end = j;
        out->items[out->count++] = u;
        i = j;
    }
    return 0;
}

/* Scan the left context ending immediately before unit i and test for
   "SATerm Close*" (allow_sp == false) or "SATerm Close* Sp*" (allow_sp). */
static bool sb_scan_saterm(const sb_units_t* u, size_t i,
                           bool allow_sp, bool aterm_only) {
    size_t k = i;
    if (allow_sp) {
        while (k > 0 && u->items[k - 1].cls == (uint32_t)UNICODOID_SB_SP) --k;
    }
    while (k > 0 && u->items[k - 1].cls == (uint32_t)UNICODOID_SB_CLOSE) --k;
    if (k == 0) return false;
    uint32_t t = u->items[k - 1].cls;
    if (aterm_only) return t == (uint32_t)UNICODOID_SB_ATERM;
    return sb_is_saterm(t);
}

/* SB8 right-hand side: (not (OLetter | Upper | Lower | ParaSep | SATerm))* Lower. */
static bool sb8_lookahead(const sb_units_t* u, size_t i) {
    size_t k = i;
    while (k < u->count) {
        uint32_t c = u->items[k].cls;
        if (c == (uint32_t)UNICODOID_SB_OLETTER ||
            c == (uint32_t)UNICODOID_SB_UPPER ||
            c == (uint32_t)UNICODOID_SB_LOWER ||
            sb_is_para(c) || sb_is_saterm(c)) {
            break;
        }
        ++k;
    }
    return k < u->count && u->items[k].cls == (uint32_t)UNICODOID_SB_LOWER;
}

static bool sb_break_before(const sb_units_t* u, size_t i) {
    if (i == 0) return false;              /* SB1 */
    if (i >= u->count) return true;        /* SB2: Any / eot */

    const sb_unit_t* prev = &u->items[i - 1];
    const sb_unit_t* cur  = &u->items[i];
    uint32_t p = prev->cls;
    uint32_t c = cur->cls;

    /* SB3: CR x LF */
    if (p == (uint32_t)UNICODOID_SB_CR && c == (uint32_t)UNICODOID_SB_LF) return false;
    /* SB4: ParaSep / */
    if (sb_is_para(p)) return true;
    /* SB6: ATerm x Numeric */
    if (p == (uint32_t)UNICODOID_SB_ATERM &&
        c == (uint32_t)UNICODOID_SB_NUMERIC) return false;
    /* SB7: (Upper | Lower) ATerm x Upper */
    if (p == (uint32_t)UNICODOID_SB_ATERM &&
        c == (uint32_t)UNICODOID_SB_UPPER && i >= 2) {
        uint32_t p2 = u->items[i - 2].cls;
        if (p2 == (uint32_t)UNICODOID_SB_UPPER ||
            p2 == (uint32_t)UNICODOID_SB_LOWER) return false;
    }
    /* SB8: ATerm Close* Sp* x (not (OLetter | Upper | Lower | ParaSep | SATerm))* Lower */
    if (sb_scan_saterm(u, i, true, true) && sb8_lookahead(u, i)) return false;
    /* SB8a: SATerm Close* Sp* x (SContinue | SATerm) */
    if ((c == (uint32_t)UNICODOID_SB_SCONTINUE || sb_is_saterm(c)) &&
        sb_scan_saterm(u, i, true, false)) return false;
    /* SB9: SATerm Close* x (Close | Sp | ParaSep) */
    if ((c == (uint32_t)UNICODOID_SB_CLOSE ||
         c == (uint32_t)UNICODOID_SB_SP || sb_is_para(c)) &&
        sb_scan_saterm(u, i, false, false)) return false;
    /* SB10: SATerm Close* Sp* x (Sp | ParaSep) */
    if ((c == (uint32_t)UNICODOID_SB_SP || sb_is_para(c)) &&
        sb_scan_saterm(u, i, true, false)) return false;
    /* SB11: SATerm Close* Sp* ParaSep? / */
    {
        size_t k = i;
        if (k > 0 && sb_is_para(u->items[k - 1].cls)) --k;
        while (k > 0 && u->items[k - 1].cls == (uint32_t)UNICODOID_SB_SP) --k;
        while (k > 0 && u->items[k - 1].cls == (uint32_t)UNICODOID_SB_CLOSE) --k;
        if (k > 0 && sb_is_saterm(u->items[k - 1].cls)) return true;
    }
    /* SB998: Any x Any */
    return false;
}

static size_t next_sentence(const seg_internal_t* in, size_t start) {
    if (in->count == 0) return 0;

    sb_units_t units;
    if (sb_build_units(in, &units) != 0) {
        return (start + 1 <= in->count) ? start + 1 : in->count;
    }

    size_t first = units.count;
    for (size_t k = 0; k < units.count; ++k) {
        if (units.items[k].start == start) { first = k; break; }
    }

    size_t end = in->count;
    for (size_t k = first + 1; k <= units.count; ++k) {
        if (sb_break_before(&units, k)) {
            end = (k < units.count) ? units.items[k].start : in->count;
            break;
        }
    }

    free(units.items);
    if (end <= start) end = start + 1;
    if (end > in->count) end = in->count;
    return end;
}


bool unicodoid_segmenter_next(
    unicodoid_segmenter_t* seg,
    unicodoid_segment_t* out_segment
) {
    if (!seg || !seg->internal || !out_segment) return false;
    seg_internal_t* in = (seg_internal_t*)seg->internal;
    if (in->index >= in->count) return false;

    size_t start = in->index;
    size_t end = start;

    switch (seg->boundary_type) {
        case UNICODOID_BOUNDARY_GRAPHEME: end = next_grapheme(in, start); break;
        case UNICODOID_BOUNDARY_WORD:     end = next_word(in, start); break;
        case UNICODOID_BOUNDARY_LINE:     end = next_line(in, start); break;
        case UNICODOID_BOUNDARY_SENTENCE: end = next_sentence(in, start); break;
        default: end = start + 1; break;
    }

    if (end <= start) end = start + 1;
    if (end > in->count) end = in->count;

    size_t byte_start = in->offsets[start];
    size_t byte_end = in->offsets[end];

    out_segment->byte_offset = byte_start;
    out_segment->byte_length = byte_end - byte_start;
    out_segment->char_count = end - start;
    out_segment->is_boundary = true;

    in->index = end;
    seg->current_offset = byte_end;
    return true;
}
