// Native UTS #10 Unicode Collation Algorithm for the C++ API.
//
// The DUCET weights come from the generated Unicode 18.0.0 tables; the
// normalization, collation-element construction and sort-key formation are
// implemented here directly, independently of the C collation module.

#include "unicodoid/uts/uts10_collation.hpp"
#include "unicodoid/uax/uax15_normalization.hpp"
#include "unicodoid/unicodoid_ucd.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace unicodoid {
namespace uts10 {
namespace {

constexpr std::size_t kSeqMax = 64;
constexpr std::size_t kWeightCap = 24;

struct Element {
    uint16_t primary;
    uint16_t secondary;
    uint16_t tertiary;
};

int block_index_of(const char* name) {
    for (std::size_t i = 0; i < unicodoid_ucd_block_names_count; ++i) {
        if (std::strcmp(unicodoid_ucd_block_names[i], name) == 0) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void push_implicit(std::vector<Element>& out, uint32_t cp) {
    uint16_t base = 0;
    uint32_t offset = 0;
    if (unicodoid_ucd_implicit_weight(cp, &base, &offset)) {
        const uint16_t bbb = static_cast<uint16_t>(offset | 0x8000u);
        out.push_back({base, 0x0020, 0x0002});
        out.push_back({bbb, 0x0000, 0x0000});
        return;
    }

    const bool unified = unicodoid_ucd_has_binary_property(
        cp, UNICODOID_BP_UNIFIED_IDEOGRAPH);
    uint16_t aaa = 0;
    if (unified) {
        static const int cjk_unified = block_index_of("CJK Unified Ideographs");
        static const int cjk_compat =
            block_index_of("CJK Compatibility Ideographs");
        const int blk = static_cast<int>(unicodoid_ucd_block(cp));
        aaa = static_cast<uint16_t>(
            (blk == cjk_unified || blk == cjk_compat)
                ? (0xFB40u + (cp >> 15))
                : (0xFB80u + (cp >> 15)));
    } else {
        aaa = static_cast<uint16_t>(0xFBC0u + (cp >> 15));
    }
    const uint16_t bbb = static_cast<uint16_t>((cp & 0x7FFFu) | 0x8000u);
    out.push_back({aaa, 0x0020, 0x0002});
    out.push_back({bbb, 0x0000, 0x0000});
}

void push_weights(
    std::vector<Element>& out,
    const unicodoid_ucd_weight_value_t* weights,
    std::size_t n) {
    for (std::size_t i = 0; i < n; ++i) {
        out.push_back({weights[i].primary, weights[i].secondary,
                       weights[i].tertiary});
    }
}

std::vector<Element> build_elements(const std::u32string& nfd) {
    std::vector<Element> out;
    std::vector<bool> consumed(nfd.size(), false);

    std::size_t i = 0;
    while (i < nfd.size()) {
        if (consumed[i]) {
            ++i;
            continue;
        }

        // Gather the contiguous run of remaining code points starting at i.
        uint32_t seq[kSeqMax];
        std::size_t idx[kSeqMax];
        std::size_t count = 0;
        for (std::size_t k = i; k < nfd.size() && count < kSeqMax; ++k) {
            if (consumed[k]) continue;
            idx[count] = k;
            seq[count] = static_cast<uint32_t>(nfd[k]);
            ++count;
        }

        unicodoid_ucd_weight_value_t weights[kWeightCap];
        std::size_t matched = 0;
        const std::size_t matched_n = unicodoid_ucd_collation_contraction(
            seq, count, &matched, weights, kWeightCap);

        std::size_t match_len = 0;
        std::size_t match_n = 0;
        if (matched >= 2) {
            match_len = matched;
            match_n = matched_n;
        } else {
            const std::size_t n =
                unicodoid_ucd_collation_weights(seq[0], weights, kWeightCap);
            if (n > 0) {
                match_len = 1;
                match_n = n;
            }
        }

        if (match_len == 0) {
            push_implicit(out, static_cast<uint32_t>(nfd[i]));
            consumed[i] = true;
            ++i;
            continue;
        }

        // S2.1.1-S2.1.3: discontiguous non-starter extension.
        uint32_t cur[kSeqMax + 1];
        std::memcpy(cur, seq, match_len * sizeof(uint32_t));
        std::size_t cur_len = match_len;
        unicodoid_ucd_weight_value_t cur_w[kWeightCap];
        std::memcpy(cur_w, weights, sizeof cur_w);
        std::size_t cur_n = match_n;

        uint16_t blocking = 0;
        const std::size_t after =
            (match_len < count) ? idx[match_len] : nfd.size();
        std::size_t j = after;
        while (j < nfd.size()) {
            if (consumed[j]) {
                ++j;
                continue;
            }
            const uint32_t cp = static_cast<uint32_t>(nfd[j]);
            const uint8_t ccc = unicodoid_ucd_canonical_combining_class(cp);
            if (ccc == 0) break;
            if (ccc <= blocking) break;

            uint32_t trial[kSeqMax + 1];
            std::memcpy(trial, cur, cur_len * sizeof(uint32_t));
            trial[cur_len] = cp;
            std::size_t m = 0;
            unicodoid_ucd_weight_value_t tw[kWeightCap];
            const std::size_t tn = unicodoid_ucd_collation_contraction(
                trial, cur_len + 1, &m, tw, kWeightCap);
            if (tn > 0 && m == cur_len + 1) {
                cur[cur_len++] = cp;
                std::memcpy(cur_w, tw, sizeof cur_w);
                cur_n = tn;
                consumed[j] = true;
                ++j;
                continue;
            }

            if (ccc > blocking) blocking = ccc;
            ++j;
        }

        push_weights(out, cur_w, cur_n);
        for (std::size_t k = 0; k < match_len; ++k) consumed[idx[k]] = true;
        i = after;
    }

    return out;
}

} // namespace

std::vector<Unit> sort_key(const std::u32string& input) {
    const std::u32string nfd = uax15::normalize(input, uax15::Form::Nfd);
    const std::vector<Element> elements = build_elements(nfd);

    std::vector<Unit> units;
    units.reserve(elements.size() * 3 + 3);

    for (const Element& ce : elements) {
        if (ce.primary != 0) units.push_back(ce.primary);
    }
    units.push_back(0);
    for (const Element& ce : elements) {
        if (ce.secondary != 0) units.push_back(ce.secondary);
    }
    units.push_back(0);
    for (const Element& ce : elements) {
        if (ce.tertiary != 0) units.push_back(ce.tertiary);
    }
    return units;
}

int compare(const std::u32string& a, const std::u32string& b) {
    const std::vector<Unit> ka = sort_key(a);
    const std::vector<Unit> kb = sort_key(b);
    const std::size_t n = std::min(ka.size(), kb.size());
    for (std::size_t i = 0; i < n; ++i) {
        if (ka[i] < kb[i]) return -1;
        if (ka[i] > kb[i]) return 1;
    }
    if (ka.size() < kb.size()) return -1;
    if (ka.size() > kb.size()) return 1;
    return 0;
}

} // namespace uts10
} // namespace unicodoid
