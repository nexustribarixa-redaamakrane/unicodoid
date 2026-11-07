/* UTS #10 Unicode Collation Algorithm, Unicode 18.0.0, Revision 55.
 * Default Unicode Collation Element Table, NON_IGNORABLE variable weighting.
 *
 * See the header for scope.  The generated tables provide single scalars,
 * contractions and @implicitweights; everything else is derived here.
 */

#include "unicodoid/uts/uts10_collation.h"
#include "unicodoid/unicodoid_ucd.h"

#include <string.h>

#define NFD_MAX      512   /* NFD expansion of the input is capped */
#define CE_MAX       2048  /* collation elements cap */
#define UNIT_MAX     4096  /* sort key units cap */
#define SEQ_MAX        64  /* contraction sequence cap */

/* ------------------------------------------------------------------------- */
/* NFD normalization (S1.1)                                                  */
/* ------------------------------------------------------------------------- */

#define HANGUL_SBASE 0xAC00u
#define HANGUL_LBASE 0x1100u
#define HANGUL_VBASE 0x1161u
#define HANGUL_TBASE 0x11A7u
#define HANGUL_LCOUNT 19u
#define HANGUL_VCOUNT 21u
#define HANGUL_TCOUNT 28u
#define HANGUL_NCOUNT (HANGUL_VCOUNT * HANGUL_TCOUNT)
#define HANGUL_SCOUNT (HANGUL_LCOUNT * HANGUL_NCOUNT)

typedef struct {
    uint32_t cps[NFD_MAX];
    size_t length;
    int ok;
} nfd_t;

static void nfd_push(nfd_t* out, uint32_t cp) {
    if (out->length < NFD_MAX) {
        out->cps[out->length++] = cp;
    } else {
        out->ok = 0;
    }
}

static void decompose(nfd_t* out, uint32_t cp) {
    if (cp >= HANGUL_SBASE && cp < HANGUL_SBASE + HANGUL_SCOUNT) {
        uint32_t sindex = cp - HANGUL_SBASE;
        nfd_push(out, HANGUL_LBASE + (sindex / HANGUL_NCOUNT));
        nfd_push(out, HANGUL_VBASE + ((sindex % HANGUL_NCOUNT) / HANGUL_TCOUNT));
        uint32_t t = HANGUL_TBASE + (sindex % HANGUL_TCOUNT);
        if (t != HANGUL_TBASE) nfd_push(out, t);
        return;
    }
    uint32_t parts[32];
    size_t n = unicodoid_ucd_decomposition(cp, false, parts, 32);
    if (n > 0 && n <= 32) {
        for (size_t i = 0; i < n; ++i) decompose(out, parts[i]);
        return;
    }
    nfd_push(out, cp);
}

static void nfd_normalize(const uint32_t* in, size_t in_len, nfd_t* out) {
    memset(out, 0, sizeof *out);
    out->ok = 1;
    /* Decompose into a scratch buffer, reorder, done. */
    nfd_t scratch;
    memset(&scratch, 0, sizeof scratch);
    scratch.ok = 1;
    for (size_t i = 0; i < in_len; ++i) decompose(&scratch, in[i]);
    if (!scratch.ok) { out->ok = 0; return; }

    /* Canonical ordering: stable insertion sort by combining class. */
    for (size_t i = 1; i < scratch.length; ++i) {
        uint32_t cp = scratch.cps[i];
        uint8_t cc = unicodoid_ucd_canonical_combining_class(cp);
        if (cc == 0) continue;
        size_t j = i;
        while (j > 0) {
            uint8_t prev = unicodoid_ucd_canonical_combining_class(scratch.cps[j - 1]);
            if (prev == 0 || prev <= cc) break;
            scratch.cps[j] = scratch.cps[j - 1];
            --j;
        }
        scratch.cps[j] = cp;
    }
    *out = scratch;
}

/* ------------------------------------------------------------------------- */
/* Collation elements (S2)                                                   */
/* ------------------------------------------------------------------------- */

typedef struct {
    uint16_t primary;
    uint16_t secondary;
    uint16_t tertiary;
} ce_t;

typedef struct {
    ce_t ces[CE_MAX];
    size_t count;
    int ok;
} ce_array_t;

static void ce_push(ce_array_t* a, uint16_t p, uint16_t s, uint16_t t) {
    if (a->count < CE_MAX) {
        a->ces[a->count].primary = p;
        a->ces[a->count].secondary = s;
        a->ces[a->count].tertiary = t;
        ++a->count;
    } else {
        a->ok = 0;
    }
}

static int block_index_of(const char* name) {
    for (size_t i = 0; i < unicodoid_ucd_block_names_count; ++i) {
        if (strcmp(unicodoid_ucd_block_names[i], name) == 0) return (int)i;
    }
    return -1;
}

/* Produce the CE sequence for one code point with no table mapping. */
static void ce_from_implicit(ce_array_t* a, uint32_t cp) {
    uint16_t base = 0;

    /* Siniform ideographic scripts: base from @implicitweights. */
    uint32_t offset = 0;
    if (unicodoid_ucd_implicit_weight(cp, &base, &offset)) {
        uint16_t bbb = (uint16_t)(offset | 0x8000u);
        ce_push(a, base, 0x0020, 0x0002);
        ce_push(a, bbb, 0x0000, 0x0000);
        return;
    }

    bool unified = unicodoid_ucd_has_binary_property(
        cp, UNICODOID_BP_UNIFIED_IDEOGRAPH);
    uint16_t aaa, bbb;
    if (unified) {
        static int cjk_unified = -2, cjk_compat = -2;
        if (cjk_unified == -2) cjk_unified = block_index_of("CJK Unified Ideographs");
        if (cjk_compat == -2) cjk_compat = block_index_of("CJK Compatibility Ideographs");
        int blk = (int)unicodoid_ucd_block(cp);
        if (blk == cjk_unified || blk == cjk_compat) {
            aaa = (uint16_t)(0xFB40u + (cp >> 15));
        } else {
            aaa = (uint16_t)(0xFB80u + (cp >> 15));
        }
    } else {
        aaa = (uint16_t)(0xFBC0u + (cp >> 15));
    }
    bbb = (uint16_t)((cp & 0x7FFFu) | 0x8000u);
    ce_push(a, aaa, 0x0020, 0x0002);
    ce_push(a, bbb, 0x0000, 0x0000);
}

static void ce_from_weights(ce_array_t* a, const unicodoid_ucd_weight_value_t* w,
                            size_t n) {
    for (size_t i = 0; i < n; ++i) {
        ce_push(a, w->primary, w->secondary, w->tertiary);
        ++w;
    }
}

/* Build the collation element array following UTS #10 §7.2 (S2.1-S2.5).
 *
 * The scan is driven by a `consumed` map: a code point absorbed into a
 * discontiguous contraction is removed from the string (S2.1.3), while
 * non-starters that could not be absorbed stay in place and are revisited by
 * the main loop (S2.5 proceeds past S only).  This is what allows a later
 * contraction to form across a completely ignorable code point, e.g.
 * 0FB2 0001 0F71 0F72: the 0F71 0F72 contraction is still found. */
static void build_ces(const nfd_t* nfd, ce_array_t* out) {
    memset(out, 0, sizeof *out);
    out->ok = 1;

    uint8_t consumed[NFD_MAX];
    memset(consumed, 0, nfd->length);

    size_t i = 0;
    while (i < nfd->length) {
        if (consumed[i]) { ++i; continue; }

        /* Gather the contiguous run of remaining code points starting at i. */
        uint32_t seq[SEQ_MAX];
        size_t idx[SEQ_MAX];
        size_t count = 0;
        for (size_t k = i; k < nfd->length && count < SEQ_MAX; ++k) {
            if (consumed[k]) continue;
            idx[count] = k;
            seq[count] = nfd->cps[k];
            ++count;
        }

        /* S2.1: longest contiguous match.  Contractions take precedence over
         * a single mapping; a single with no table entry falls back to the
         * implicit weights of S2.2. */
        unicodoid_ucd_weight_value_t weights[24];
        size_t matched = 0;
        size_t matched_n = unicodoid_ucd_collation_contraction(
            seq, count, &matched, weights, 24);
        size_t match_len;
        size_t match_n;
        if (matched >= 2) {
            match_len = matched;
            match_n = matched_n;
        } else {
            size_t n = unicodoid_ucd_collation_weights(seq[0], weights, 24);
            if (n > 0) { match_len = 1; match_n = n; }
            else { match_len = 0; match_n = 0; }
        }

        if (match_len == 0) {
            ce_from_implicit(out, nfd->cps[i]);
            consumed[i] = 1;
            ++i;
            continue;
        }

        /* S2.1.1-S2.1.3: discontiguous non-starter extension.  Only
         * unblocked non-starters are considered; the lookahead stops at the
         * first starter or the first blocked non-starter.  Absorbed code
         * points are removed; the rest remain for later iterations. */
        uint32_t cur[SEQ_MAX + 1];
        size_t cur_len = match_len;
        memcpy(cur, seq, match_len * sizeof(uint32_t));
        unicodoid_ucd_weight_value_t cur_w[24];
        memcpy(cur_w, weights, sizeof cur_w);
        size_t cur_n = match_n;

        uint16_t blocking = 0;
        size_t after = (match_len < count) ? idx[match_len] : nfd->length;
        size_t j = after;
        while (j < nfd->length) {
            if (consumed[j]) { ++j; continue; }
            uint32_t cp = nfd->cps[j];
            uint8_t ccc = unicodoid_ucd_canonical_combining_class(cp);
            if (ccc == 0) break;        /* starter terminates the lookahead */
            if (ccc <= blocking) break; /* blocked non-starter terminates it */

            uint32_t trial[SEQ_MAX + 1];
            memcpy(trial, cur, cur_len * sizeof(uint32_t));
            trial[cur_len] = cp;
            size_t m = 0;
            unicodoid_ucd_weight_value_t tw[24];
            size_t tn = unicodoid_ucd_collation_contraction(
                trial, cur_len + 1, &m, tw, 24);
            if (tn > 0 && m == cur_len + 1) {
                cur[cur_len++] = cp;
                memcpy(cur_w, tw, sizeof cur_w);
                cur_n = tn;
                consumed[j] = 1;        /* S2.1.3: remove C */
                ++j;
                continue;               /* absorbed C does not block */
            }
            /* Not an extension: keep it for a later iteration and let it
             * contribute to the blocking context. */
            if (ccc > blocking) blocking = ccc;
            ++j;
        }

        ce_from_weights(out, cur_w, cur_n);
        for (size_t k = 0; k < match_len; ++k) consumed[idx[k]] = 1;
        i = after;
    }
}

/* ------------------------------------------------------------------------- */
/* Sort key formation (S3)                                                   */
/* ------------------------------------------------------------------------- */

size_t unicodoid_uts10_sortkey(
    const uint32_t* cps, size_t length,
    unicodoid_uts10_unit_t* out, size_t cap) {
    nfd_t nfd;
    nfd_normalize(cps, length, &nfd);
    if (!nfd.ok) return 0;

    ce_array_t ces;
    build_ces(&nfd, &ces);
    if (!ces.ok) return 0;

    unicodoid_uts10_unit_t units[UNIT_MAX];
    size_t u = 0;

    /* Level 1. */
    for (size_t k = 0; k < ces.count; ++k) {
        uint16_t p = ces.ces[k].primary;
        if (p != 0 && u < UNIT_MAX) units[u++] = p;
    }
    if (u < UNIT_MAX) units[u++] = 0;
    /* Level 2. */
    for (size_t k = 0; k < ces.count; ++k) {
        uint16_t s = ces.ces[k].secondary;
        if (s != 0 && u < UNIT_MAX) units[u++] = s;
    }
    if (u < UNIT_MAX) units[u++] = 0;
    /* Level 3. */
    for (size_t k = 0; k < ces.count; ++k) {
        uint16_t t = ces.ces[k].tertiary;
        if (t != 0 && u < UNIT_MAX) units[u++] = t;
    }

    if (out && cap >= u) {
        memcpy(out, units, u * sizeof(units[0]));
    }
    return u;
}

int unicodoid_uts10_compare(
    const uint32_t* a, size_t a_len,
    const uint32_t* b, size_t b_len) {
    unicodoid_uts10_unit_t ka[UNIT_MAX], kb[UNIT_MAX];
    size_t na = unicodoid_uts10_sortkey(a, a_len, ka, UNIT_MAX);
    size_t nb = unicodoid_uts10_sortkey(b, b_len, kb, UNIT_MAX);

    size_t n = na < nb ? na : nb;
    for (size_t i = 0; i < n; ++i) {
        if (ka[i] < kb[i]) return -1;
        if (ka[i] > kb[i]) return 1;
    }
    if (na < nb) return -1;
    if (na > nb) return 1;
    return 0;
}
