// Native C++ conformance and algorithm tests for the UAX / UTS / UTR modules
// implemented over the native C++ data layer and algorithms.
//
// The Unicode version baseline, the official-data-driven property lookups and
// the specification algorithms are exercised here through the native C++
// namespaces (unicodoid::uax34, unicodoid::uax44, unicodoid::uax15,
// unicodoid::uts10, unicodoid::uts51, unicodoid::uts58, unicodoid::utr59).
//
// The vectors are read directly from the UCD distribution; nothing is copied
// or curated.  Directories are provided by the build system through the
// UNICODOID_UCD_* compile definitions.

#include "unicodoid/unicodoid.hpp"
#include "unicodoid/uax/uax15_normalization.hpp"
#include "unicodoid/uax/uax34_named_sequences.hpp"
#include "unicodoid/uax/uax44_properties.hpp"
#include "unicodoid/uts/uts10_collation.hpp"
#include "unicodoid/uts/uts51_emoji_sequences.hpp"
#include "unicodoid/uts/uts58_link_detection.hpp"
#include "unicodoid/utr/utr59_east_asian_spacing.hpp"

#include "unicodoid/unicodoid_ucd.h"

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace {

int g_failures = 0;
int g_checks = 0;

void check(bool condition, const char* what) {
    ++g_checks;
    if (condition) return;
    ++g_failures;
    std::fprintf(stderr, "FAIL: %s\n", what);
}

std::string read_file(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) throw std::runtime_error("cannot open " + path);
    std::stringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

std::vector<std::string> split_lines(const std::string& text) {
    std::vector<std::string> lines;
    std::string line;
    for (const char ch : text) {
        if (ch == '\n') {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            lines.push_back(line);
            line.clear();
        } else {
            line.push_back(ch);
        }
    }
    if (!line.empty()) lines.push_back(line);
    return lines;
}

std::vector<uint32_t> parse_scalars(const std::string& field) {
    std::vector<uint32_t> cps;
    std::istringstream tokens(field);
    std::string token;
    while (tokens >> token) {
        unsigned long cp = std::stoul(token, nullptr, 16);
        cps.push_back(static_cast<uint32_t>(cp));
    }
    return cps;
}

static std::u32string to_u32(const std::vector<uint32_t>& cps) {
    std::u32string s;
    s.reserve(cps.size());
    for (const uint32_t cp : cps) s.push_back(static_cast<char32_t>(cp));
    return s;
}

static std::u32string from_cp(uint32_t cp) {
    std::u32string s;
    s.push_back(static_cast<char32_t>(cp));
    return s;
}

std::string_view string_view(const std::string& text) {
    return std::string_view(text.data(), text.size());
}

void run_uax44() {
    check(unicodoid::uax44::general_category(0x41) ==
              static_cast<uint8_t>(UNICODOID_GC_LU),
          "UAX44 Latin capital is Lu");
    check(unicodoid::uax44::general_category(0x0301) ==
              static_cast<uint8_t>(UNICODOID_GC_MN),
          "UAX44 acute is Mn");
    check(unicodoid::uax44::canonical_combining_class(0x0301) == 230,
          "UAX44 canonical combining class of acute is 230");
    check(unicodoid::uax44::has_binary_property(0x41, UNICODOID_BP_ALPHABETIC),
          "UAX44 LATIN CAPITAL is alphabetic");
    check(unicodoid::uax44::has_binary_property(0x901, UNICODOID_BP_GRAPHEME_EXTEND),
          "UAX44 Devanagari sign is Grapheme_Extend");
    check(unicodoid::uax44::grapheme_break(0x901) == UNICODOID_GCB_EXTEND,
          "UAX44 Devanagari sign is GCB Extend");
    check(unicodoid::uax44::script(0x41) == 2, "UAX44 Latin script is Latn");
    check(std::string(unicodoid::uax44::grapheme_break_name(
              unicodoid::uax44::grapheme_break(0x20DD))) == "Extend",
          "UAX44 enclosing mark is Extend");
}

void run_uax15() {
    const std::u32string text = to_u32({0x65, 0x301}); // decomposed e + acute
    const std::u32string nfc = unicodoid::uax15::normalize(text, unicodoid::uax15::Form::Nfc);
    check(nfc.size() == 1 && static_cast<uint32_t>(nfc[0]) == 0xE9,
          "UAX15 NFC composes e + acute");
    const std::u32string nfd = unicodoid::uax15::normalize(text, unicodoid::uax15::Form::Nfd);
    check(nfd.size() == 2, "UAX15 NFD decomposes");
    check(unicodoid::uax15::is_normalized(text, unicodoid::uax15::Form::Nfd),
          "UAX15 decomposed e+acute is NFD");
    check(!unicodoid::uax15::is_normalized(text, unicodoid::uax15::Form::Nfc),
          "UAX15 decomposed e+acute is not NFC");
    check(unicodoid::uax15::is_normalized(nfc, unicodoid::uax15::Form::Nfc),
          "UAX15 composed e-acute is NFC");
}

void run_uax34() {
    const std::string path = std::string(UNICODOID_UCD_NAMEREP_SEQ_FILE);
    const std::string text = read_file(path);
    int lines = 0;

    for (const std::string& line : split_lines(text)) {
        if (line.empty() || line[0] == '#' || line[0] == '@') continue;
        const std::size_t semi = line.find(';');
        if (semi == std::string::npos) continue;
        const std::string name = line.substr(0, semi);
        std::vector<uint32_t> expected = parse_scalars(line.substr(semi + 1));

        const auto found = unicodoid::uax34::find(string_view(name));
        ++lines;
        if (!found.has_value()) {
            ++g_failures;
            if (g_failures <= 10) {
                std::fprintf(stderr, "UAX34: lookup mismatch for %s\n", name.c_str());
            }
        }
        if (!unicodoid::uax34::is_registered(to_u32(expected))) {
            ++g_failures;
            if (g_failures <= 10) {
                std::fprintf(stderr, "UAX34: sequence not registered for %s\n", name.c_str());
            }
        }
    }

    check(unicodoid::uax34::count() == static_cast<std::size_t>(lines),
          "UAX34 count matches data file");
    check(lines > 400, "NamedSequences.txt is populated");
}

void run_uts10() {
    const std::u32string a = to_u32({0x61, 0x70, 0x70, 0x6C, 0x65}); // "apple"
    check(unicodoid::uts10::compare(a, a) == 0, "UTS10 sort key of apple is self-equal");
    check(unicodoid::uts10::sort_key(a).size() > 0, "UTS10 sort key is populated");
}

void run_uts51() {
    const std::string dir = std::string(UNICODOID_UCD_EMOJI_DIR);
    const std::vector<std::string> files = {
        dir + "/emoji-sequences.txt",
        dir + "/emoji-zwj-sequences.txt",
    };

    int validated = 0;
    int failures = 0;
    for (const std::string& path : files) {
        const std::string text = read_file(path);
        for (const std::string& line : split_lines(text)) {
            const std::string trimmed = line;
            if (trimmed.empty() || trimmed[0] == '#' || trimmed[0] == '@') continue;
            const std::size_t semi = trimmed.find(';');
            if (semi == std::string::npos) continue;
            const std::string type = trimmed.substr(
                semi + 1, trimmed.find(';', semi + 1) - (semi + 1));
            const std::size_t type_start = type.find_first_not_of(" \t\r\n");
            const std::size_t type_end = type.find_last_not_of(" \t\r\n");
            std::string type_trimmed = type;
            if (type_start != std::string::npos) {
                type_trimmed = type.substr(type_start, type_end - type_start + 1);
            }
            const std::string field = trimmed.substr(0, semi);
            const std::size_t first_nonspace = field.find_first_not_of(" \t");
            std::string seq = field;
            if (first_nonspace != std::string::npos) {
                seq = field.substr(first_nonspace);
            }
            while (!seq.empty() && (seq.back() == ' ' || seq.back() == '\t')) {
                seq.pop_back();
            }

            std::size_t range_end = seq.find("..");
            if (range_end != std::string::npos) {
                const unsigned long first =
                    std::stoul(seq.substr(0, range_end), nullptr, 16);
                const unsigned long last =
                    std::stoul(seq.substr(range_end + 2), nullptr, 16);
                for (unsigned long cp = first; cp <= last; ++cp) {
                    const std::u32string cps = from_cp(static_cast<uint32_t>(cp));
                    const auto found = unicodoid::uts51::is_rgi(cps);
                    ++validated;
                    if (!found.has_value() || *found != std::string_view(type_trimmed)) {
                        ++failures;
                        if (failures <= 10) {
                            std::fprintf(stderr, "UTS51: RGI mismatch at U+%04lX\n", cp);
                        }
                    }
                }
            } else {
                const std::vector<uint32_t> cps = parse_scalars(seq);
                if (cps.empty()) continue;
                const std::u32string cps_str = to_u32(cps);
                const auto found = unicodoid::uts51::is_rgi(cps_str);
                ++validated;
                if (!found.has_value() || *found != std::string_view(type_trimmed)) {
                    ++failures;
                    if (failures <= 10) {
                        std::fprintf(stderr, "UTS51: RGI mismatch for sequence\n");
                    }
                }
            }
        }
    }

    check(unicodoid::uts51::count() > 3000, "UTS51 generated data is populated");
    check(failures == 0, "UTS51 RGI conformance failures");
    std::fprintf(stderr, "UTS51: %d sequences validated\n", validated);
}

void run_uts58() {
    // Email local part: valid ASCII.
    {
        std::vector<uint32_t> cps;
        for (const uint32_t cp : std::vector<uint32_t>{0x63, 0x6F, 0x6E, 0x74, 0x61,
                                                       0x63, 0x74, 0x20, 0x61, 0x62,
                                                       0x63, 0x64, 0x40, 0x65,
                                                       0x78, 0x61, 0x6D, 0x70, 0x6C, 0x65})
        {
            cps.push_back(cp);
        }
        const std::u32string text = to_u32(cps);
        std::size_t at = text.size();
        for (std::size_t i = 0; i < text.size(); ++i) {
            if (static_cast<uint32_t>(text[i]) == U'@') { at = i; break; }
        }
        const auto start = unicodoid::uts58::email_local_part_start(text, at);
        check(start.has_value() && *start == 8, "UTS58 email local part start");
    }

    // Link detection: soft vs hard termination.
    {
        const std::u32string text = to_u32({0x65, 0x78, 0x61, 0x6D, 0x70, 0x6C, 0x65,
                                          0x2E, 0x63, 0x6F, 0x6D, 0x2F, 0x61});
        check(unicodoid::uts58::link_detection_end(text, 11) == 13,
              "UTS58 link detection end");
    }

    // Minimal escaping of a trailing soft character.
    {
        const std::u32string text = to_u32({0x64, 0x2E, 0x65, 0x3F});
        const std::vector<uint8_t> escaped =
            unicodoid::uts58::minimal_escape_part(
                unicodoid::uts58::UrlPart::Path, text, true);
        check(escaped.size() == 7, "UTS58 minimal escape size");
    }

    // Official LinkDetectionTest.txt.
    const std::string text = read_file(std::string(UNICODOID_UCD_LINK_DIR) + "/LinkDetectionTest.txt");
    int cases = 0;
    int failures = 0;
    int checked = 0;
    for (const std::string& line : split_lines(text)) {
        if (line.empty() || line[0] == '#') continue;
        ++cases;

        std::vector<uint32_t> cps;
        std::size_t i = 0;
        while (i < line.size()) {
            const unsigned char b0 = static_cast<unsigned char>(line[i]);
            uint32_t cp = 0;
            std::size_t adv = 1;
            if (b0 < 0x80) { cp = b0; adv = 1; }
            else if (b0 < 0xE0) { cp = ((b0 & 0x1F) << 6) | (static_cast<unsigned char>(line[i + 1]) & 0x3F); adv = 2; }
            else if (b0 < 0xF0) {
                cp = ((b0 & 0x0F) << 12) | ((static_cast<unsigned char>(line[i + 1]) & 0x3F) << 6) | (static_cast<unsigned char>(line[i + 2]) & 0x3F);
                adv = 3;
            } else {
                cp = ((b0 & 0x07) << 18) | ((static_cast<unsigned char>(line[i + 1]) & 0x3F) << 12) |
                     ((static_cast<unsigned char>(line[i + 2]) & 0x3F) << 6) | (static_cast<unsigned char>(line[i + 3]) & 0x3F);
                adv = 4;
            }
            cps.push_back(cp);
            i += adv;
        }

        std::size_t j = 0;
        while (j < cps.size()) {
            if (cps[j] != 0x2E20) { ++j; continue; }
            std::size_t open = j;
            std::size_t close = cps.size();
            for (std::size_t k = open + 1; k < cps.size(); ++k) {
                if (cps[k] == 0x2E21) { close = k; break; }
            }
            if (close == cps.size()) break;
            std::size_t init = (std::size_t)-1;
            for (std::size_t k = open + 1; k < close; ++k) {
                if (cps[k] == 0x2F || cps[k] == 0x3F || cps[k] == 0x23) { init = k; break; }
            }
            if (init != (std::size_t)-1) {
                const std::size_t end = unicodoid::uts58::link_detection_end(to_u32(cps), init);
                ++checked;
                if (end != close) {
                    ++failures;
                    if (failures <= 10) {
                        std::fprintf(stderr, "detection mismatch: span=[%zu,%zu) end=%zu\n",
                                     open + 1, close, end);
                    }
                }
            }
            j = close + 1;
        }
    }

    check(failures == 0, "UTS58 link detection conformance failures");
    check(checked > 100, "UTS58 detection harness is populated");
    std::fprintf(stderr, "LinkDetectionTest.txt: %d cases, %d checked\n", cases, checked);
}

void run_utr59() {
    const std::string text = read_file(std::string(UNICODOID_UCD_TR59_FILE));
    int entries = 0;
    int failures = 0;
    for (const std::string& line : split_lines(text)) {
        if (line.empty() || line[0] == '#' || line[0] == '@') continue;
        const std::size_t semi = line.find(';');
        if (semi == std::string::npos) continue;
        const std::string value_field = line.substr(semi + 1);
        const std::size_t vstart = value_field.find_first_not_of(" \t\r\n");
        if (vstart == std::string::npos) continue;
        const std::size_t vend = value_field.find_last_not_of(" \t\r\n");
        const std::string value_trimmed =
            value_field.substr(vstart, vend - vstart + 1);
        const char first = value_trimmed[0];
        int expected = -1;
        if (first == 'W') expected = static_cast<int>(unicodoid::utr59::Eas::W);
        else if (first == 'N') expected = static_cast<int>(unicodoid::utr59::Eas::N);
        else if (first == 'O') expected = static_cast<int>(unicodoid::utr59::Eas::O);
        else if (first == 'C') expected = static_cast<int>(unicodoid::utr59::Eas::C);
        if (expected < 0) continue;

        const std::string range = line.substr(0, semi);
        std::size_t dots = range.find("..");
        unsigned long first_cp = 0;
        unsigned long last_cp = 0;
        if (dots != std::string::npos) {
            first_cp = std::stoul(range.substr(0, dots), nullptr, 16);
            last_cp = std::stoul(range.substr(dots + 2), nullptr, 16);
        } else {
            first_cp = std::stoul(range, nullptr, 16);
            last_cp = first_cp;
        }
        for (unsigned long cp = first_cp; cp <= last_cp; ++cp) {
            if (unicodoid::utr59::east_asian_spacing(static_cast<uint32_t>(cp)) !=
                static_cast<uint8_t>(expected)) {
                ++failures;
                if (failures <= 10) {
                    std::fprintf(stderr, "UTR59: U+%04lX mismatch\n", cp);
                }
            }
        }
        ++entries;
    }

    check(entries > 1000, "east-asian-spacing.txt is populated");
    check(failures == 0, "UTR59 property conformance failures");

    check(unicodoid::utr59::east_asian_spacing(0x4E00u) ==
              static_cast<uint8_t>(unicodoid::utr59::Eas::W),
          "UTR59 HAN is Wide");
    check(unicodoid::utr59::east_asian_spacing(0x0041u) ==
              static_cast<uint8_t>(unicodoid::utr59::Eas::N),
          "UTR59 LATIN A is Narrow");
    check(unicodoid::utr59::cluster_value(from_cp(0x6F22), false, false) ==
              static_cast<uint8_t>(unicodoid::utr59::Eas::W),
          "UTR59 cluster value for Han");
    check(unicodoid::utr59::cluster_value(from_cp(0x61), false, false) ==
              static_cast<uint8_t>(unicodoid::utr59::Eas::N),
          "UTR59 cluster value for Latin");

    const std::u32string han_a = to_u32({0x6F22, 0x41}); // HAN + LATIN A
    const std::vector<std::size_t> offsets =
        unicodoid::utr59::spacing_offsets(han_a, false, false);
    check(offsets.size() == 1 && offsets[0] == 1,
          "UTR59 Han+Latin inserts spacing");
}

} // namespace

int main() {
    try {
        check(std::string(unicodoid_ucd_version()).compare(0, 6, "18.0.0") == 0,
              "generated data version is Unicode 18.0.0");
        check(std::string(unicodoid_ucd_generator_version()) == "1.2.0",
              "generated data generator version is 1.2.0");

        const unicodoid::EncodingSystem utf8_system = unicodoid::Registry::utf8();
        const unicodoid::UnicodeProfile profile = utf8_system.unicode_profile();
        check(profile.major_version() == 18 &&
                  profile.minor_version() == 0 &&
                  profile.patch_version() == 0,
              "built-in Unicode profile is 18.0.0");

        run_uax44();
        run_uax15();
        run_uax34();
        run_uts10();
        run_uts51();
        run_uts58();
        run_utr59();
    } catch (const std::exception& ex) {
        std::fprintf(stderr, "unexpected exception: %s\n", ex.what());
        return 1;
    }

    std::fprintf(stderr, "test_cpp_specs: %d checks, %d failures\n", g_checks, g_failures);
    if (g_failures == 0) {
        std::printf("test_cpp_specs: PASS\n");
        return 0;
    }
    std::printf("test_cpp_specs: FAIL (%d failures)\n", g_failures);
    return 1;
}
