// Native UAX #15 Unicode Normalization for the C++ API.
//
// The algorithm is implemented directly on top of the generated Unicode 18.0.0
// data layer; it does not forward to the C normalizer.

#include "unicodoid/uax/uax15_normalization.hpp"
#include "unicodoid/unicodoid_ucd.h"

#include <cstdint>
#include <vector>

namespace unicodoid {
namespace uax15 {
namespace {

constexpr uint32_t kHangulSBase = 0xAC00u;
constexpr uint32_t kHangulLBase = 0x1100u;
constexpr uint32_t kHangulVBase = 0x1161u;
constexpr uint32_t kHangulTBase = 0x11A7u;
constexpr uint32_t kHangulLCount = 19u;
constexpr uint32_t kHangulVCount = 21u;
constexpr uint32_t kHangulTCount = 28u;
constexpr uint32_t kHangulNCount = kHangulVCount * kHangulTCount;
constexpr uint32_t kHangulSCount = kHangulLCount * kHangulNCount;

void decompose_scalar(uint32_t cp, bool compatibility, std::u32string& out) {
    if (cp >= kHangulSBase && cp < kHangulSBase + kHangulSCount) {
        const uint32_t index = cp - kHangulSBase;
        out.push_back(static_cast<char32_t>(kHangulLBase + index / kHangulNCount));
        out.push_back(static_cast<char32_t>(
            kHangulVBase + (index % kHangulNCount) / kHangulTCount));
        const uint32_t t = kHangulTBase + index % kHangulTCount;
        if (t != kHangulTBase) out.push_back(static_cast<char32_t>(t));
        return;
    }

    uint32_t parts[32];
    const std::size_t n =
        unicodoid_ucd_decomposition(cp, compatibility, parts, 32);
    if (unicodoid_ucd_has_decomposition(cp, compatibility) && n > 0 && n <= 32) {
        for (std::size_t i = 0; i < n; ++i) {
            decompose_scalar(parts[i], compatibility, out);
        }
        return;
    }

    out.push_back(static_cast<char32_t>(cp));
}

void canonical_order(std::u32string& seq) {
    for (std::size_t i = 1; i < seq.size(); ++i) {
        const char32_t cp = seq[i];
        const uint8_t cc = unicodoid_ucd_canonical_combining_class(
            static_cast<uint32_t>(cp));
        if (cc == 0) continue;

        std::size_t j = i;
        while (j > 0) {
            const uint8_t prev = unicodoid_ucd_canonical_combining_class(
                static_cast<uint32_t>(seq[j - 1]));
            if (prev == 0 || prev <= cc) break;
            seq[j] = seq[j - 1];
            --j;
        }
        seq[j] = cp;
    }
}

bool compose_pair(uint32_t starter, uint32_t ch, uint32_t& out) {
    // Hangul LV composition.
    if (starter >= kHangulLBase && starter < kHangulLBase + kHangulLCount &&
        ch >= kHangulVBase && ch < kHangulVBase + kHangulVCount) {
        const uint32_t lindex = starter - kHangulLBase;
        const uint32_t vindex = ch - kHangulVBase;
        out = kHangulSBase + (lindex * kHangulVCount + vindex) * kHangulTCount;
        return true;
    }

    // Hangul LVT composition.
    if (starter >= kHangulSBase && starter < kHangulSBase + kHangulSCount &&
        ((starter - kHangulSBase) % kHangulTCount) == 0 &&
        ch > kHangulTBase && ch < kHangulTBase + kHangulTCount) {
        out = starter + (ch - kHangulTBase);
        return true;
    }

    uint32_t composed = 0;
    if (unicodoid_ucd_compose_pair(starter, ch, &composed)) {
        out = composed;
        return true;
    }
    return false;
}

std::u32string compose(const std::u32string& seq) {
    std::u32string out;
    if (seq.empty()) return out;
    out.reserve(seq.size());
    out.push_back(seq[0]);

    std::size_t starter_pos = 0;
    uint32_t starter = static_cast<uint32_t>(seq[0]);
    uint8_t last_cc = 0;

    for (std::size_t i = 1; i < seq.size(); ++i) {
        const uint32_t cp = static_cast<uint32_t>(seq[i]);
        const uint8_t cc = unicodoid_ucd_canonical_combining_class(cp);
        uint32_t composed = 0;

        if ((last_cc < cc || last_cc == 0) && compose_pair(starter, cp, composed)) {
            out[starter_pos] = static_cast<char32_t>(composed);
            starter = composed;
            continue;
        }

        if (cc == 0) {
            starter_pos = out.size();
            starter = cp;
        }
        out.push_back(static_cast<char32_t>(cp));
        last_cc = cc;
    }
    return out;
}

} // namespace

std::u32string normalize(const std::u32string& input, Form form) {
    const bool compatibility = form == Form::Nfkc || form == Form::Nfkd;
    const bool do_compose = form == Form::Nfc || form == Form::Nfkc;

    std::u32string decomposed;
    decomposed.reserve(input.size() + input.size() / 2 + 8);
    for (std::size_t k = 0; k < input.size(); ++k) {
        decompose_scalar(static_cast<uint32_t>(input[k]), compatibility, decomposed);
    }
    canonical_order(decomposed);

    if (!do_compose) return decomposed;
    return compose(decomposed);
}

bool is_normalized(const std::u32string& input, Form form) {
    const std::u32string normalized = normalize(input, form);
    return normalized.size() == input.size() &&
           std::u32string_view(normalized.data(), normalized.size()) ==
               std::u32string_view(input.data(), input.size());
}

} // namespace uax15
} // namespace unicodoid
