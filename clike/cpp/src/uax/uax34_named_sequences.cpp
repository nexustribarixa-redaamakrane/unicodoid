// Native UAX #34 Unicode Named Character Sequences for the C++ API.

#include "unicodoid/uax/uax34_named_sequences.hpp"
#include "unicodoid/unicodoid_ucd.h"

#include <cstdint>
#include <string>

namespace unicodoid {
namespace uax34 {
namespace {

bool valid_scalar(uint32_t cp) {
    return cp <= 0x10FFFF && !(cp >= 0xD800 && cp <= 0xDFFF);
}

std::vector<char32_t> fetch(std::size_t index) {
    const std::size_t n = unicodoid_ucd_named_seq(index, nullptr, 0);
    if (n == static_cast<std::size_t>(-1)) return {};
    std::vector<uint32_t> buffer(n);
    if (n > 0) unicodoid_ucd_named_seq(index, buffer.data(), buffer.size());
    std::vector<char32_t> out;
    out.reserve(n);
    for (const uint32_t cp : buffer) out.push_back(static_cast<char32_t>(cp));
    return out;
}

} // namespace

std::size_t count() {
    return unicodoid_ucd_named_sequences_count();
}

std::string_view name(std::size_t index) {
    const char* value = unicodoid_ucd_named_seq_name(index);
    return value ? std::string_view(value) : std::string_view();
}

std::optional<std::vector<char32_t>> sequence(std::size_t index) {
    if (index >= count()) return std::nullopt;
    return fetch(index);
}

std::optional<std::vector<char32_t>> find(std::string_view name) {
    const std::string key(name);
    std::size_t index = 0;
    if (!unicodoid_ucd_named_seq_find(key.c_str(), &index)) return std::nullopt;
    return fetch(index);
}

bool is_registered(const std::u32string& cps) {
    if (cps.empty()) return false;
    for (std::size_t k = 0; k < cps.size(); ++k) {
        if (!valid_scalar(static_cast<uint32_t>(cps[k]))) return false;
    }

    const std::size_t total = count();
    std::vector<uint32_t> buffer;
    for (std::size_t i = 0; i < total; ++i) {
        const std::size_t n = unicodoid_ucd_named_seq(i, nullptr, 0);
        if (n != cps.size()) continue;
        buffer.resize(n);
        if (n > 0) unicodoid_ucd_named_seq(i, buffer.data(), buffer.size());
        bool equal = true;
        for (std::size_t k = 0; k < n; ++k) {
            if (buffer[k] != static_cast<uint32_t>(cps[k])) {
                equal = false;
                break;
            }
        }
        if (equal) return true;
    }
    return false;
}

} // namespace uax34
} // namespace unicodoid
