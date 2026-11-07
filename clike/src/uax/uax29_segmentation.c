#include "unicodoid/uax/uax29_segmentation.h"
#include "unicodoid/unicodoid_ucd.h"

/* UAX #29 extended grapheme cluster boundary rules, evaluated against the
 * generated Unicode 18.0.0 property tables.  The logic mirrors the byte
 * segmenter in unicodoid_segmentation.c so that both paths agree. */

static uint32_t gcb(uint32_t cp) {
    return (uint32_t)unicodoid_ucd_grapheme_break(cp);
}

static uint32_t incb(uint32_t cp) {
    return (uint32_t)unicodoid_ucd_indic_conjunct_break(cp);
}

static bool is_control(uint32_t cp) {
    uint32_t g = gcb(cp);
    return g == (uint32_t)UNICODOID_GCB_CONTROL ||
           g == (uint32_t)UNICODOID_GCB_CR ||
           g == (uint32_t)UNICODOID_GCB_LF;
}

static bool is_regional_indicator(uint32_t cp) {
    return gcb(cp) == (uint32_t)UNICODOID_GCB_RI;
}

static bool is_extended_pictographic(uint32_t cp) {
    return unicodoid_ucd_has_binary_property(
        cp, UNICODOID_BP_EXTENDED_PICTOGRAPHIC);
}

size_t unicodoid_uax29_next_grapheme(
    const uint32_t* cps, size_t length, size_t start) {
    if (!cps || start >= length) return length;

    size_t i = start + 1;
    if (i > length) return length;

    uint32_t prev = cps[start];
    uint32_t prev_g = gcb(prev);
    size_t prev_ri_pos = is_regional_indicator(prev) ? 0u : (size_t)-1u;
    bool emoji_context = is_extended_pictographic(prev);
    bool emoji_zwj_pending = false;
    bool in_linker_conjunct = (incb(prev) == (uint32_t)UNICODOID_INCB_LINKER);

    while (i < length) {
        uint32_t cur = cps[i];
        uint32_t cur_g = gcb(cur);
        uint32_t cur_incb = incb(cur);
        bool boundary = true;

        if (prev == 0x000D && cur == 0x000A) {
            /* GB3: CR x LF. */
            boundary = false;
        } else if (is_control(prev) || is_control(cur)) {
            /* GB4 / GB5: break before and after controls. */
            boundary = true;
        } else if (prev_g == (uint32_t)UNICODOID_GCB_L &&
                   (cur_g == (uint32_t)UNICODOID_GCB_L ||
                    cur_g == (uint32_t)UNICODOID_GCB_V ||
                    cur_g == (uint32_t)UNICODOID_GCB_LV ||
                    cur_g == (uint32_t)UNICODOID_GCB_LVT)) {
            /* GB6: L x (L | V | LV | LVT). */
            boundary = false;
        } else if ((prev_g == (uint32_t)UNICODOID_GCB_LV ||
                    prev_g == (uint32_t)UNICODOID_GCB_V) &&
                   (cur_g == (uint32_t)UNICODOID_GCB_V ||
                    cur_g == (uint32_t)UNICODOID_GCB_T)) {
            /* GB7: (LV | V) x (V | T). */
            boundary = false;
        } else if ((prev_g == (uint32_t)UNICODOID_GCB_LVT ||
                    prev_g == (uint32_t)UNICODOID_GCB_T) &&
                   cur_g == (uint32_t)UNICODOID_GCB_T) {
            /* GB8: (LVT | T) x T. */
            boundary = false;
        } else if (cur_g == (uint32_t)UNICODOID_GCB_EXTEND ||
                   cur_g == (uint32_t)UNICODOID_GCB_ZWJ) {
            /* GB9: x (Extend | ZWJ). */
            boundary = false;
        } else if (cur_g == (uint32_t)UNICODOID_GCB_SPACINGMARK) {
            /* GB9a: x SpacingMark. */
            boundary = false;
        } else if (prev_g == (uint32_t)UNICODOID_GCB_PREPEND) {
            /* GB9b: Prepend x. */
            boundary = false;
        } else if (cur_incb == (uint32_t)UNICODOID_INCB_CONSONANT &&
                   in_linker_conjunct) {
            /* GB9c: Linker Extend* x Consonant. */
            boundary = false;
        } else if (emoji_zwj_pending && is_extended_pictographic(cur)) {
            /* GB11: ExtPict Extend* ZWJ x ExtPict. */
            boundary = false;
        } else if (cur_g == (uint32_t)UNICODOID_GCB_RI) {
            /* GB12 / GB13: pair regional indicators. */
            size_t cur_ri_pos =
                (prev_g == (uint32_t)UNICODOID_GCB_RI) ? (prev_ri_pos + 1u) : 0u;
            if ((cur_ri_pos % 2u) == 1u) boundary = false;
        }

        /* GB999: Any / Any. */
        if (boundary) return i;

        if (cur_g == (uint32_t)UNICODOID_GCB_RI) {
            prev_ri_pos =
                (prev_g == (uint32_t)UNICODOID_GCB_RI) ? (prev_ri_pos + 1u) : 0u;
        } else {
            prev_ri_pos = (size_t)-1u;
        }

        if (cur == 0x200Du) {
            if (emoji_context) emoji_zwj_pending = true;
        } else if (cur_g == (uint32_t)UNICODOID_GCB_EXTEND) {
            /* Extend keeps the emoji context alive. */
        } else {
            emoji_context = is_extended_pictographic(cur);
            emoji_zwj_pending = false;
        }

        if (cur_incb == (uint32_t)UNICODOID_INCB_LINKER) {
            in_linker_conjunct = true;
        } else if (cur_incb == (uint32_t)UNICODOID_INCB_EXTEND) {
            /* ConjunctExtender* keeps the linker context. */
        } else {
            in_linker_conjunct = false;
        }

        prev = cur;
        prev_g = cur_g;
        ++i;
    }
    return length;
}

bool unicodoid_uax29_is_grapheme_boundary(
    const uint32_t* cps, size_t length, size_t index) {
    if (index == 0 || index >= length) return true;
    size_t start = 0;
    while (start < index) {
        start = unicodoid_uax29_next_grapheme(cps, length, start);
    }
    return start == index;
}
