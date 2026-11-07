// Official Unicode 18.0.0 conformance tests for the native C++ segmenter.
//
// The vectors are read directly from the UCD distribution; nothing is copied or
// curated.  The directory is provided by the build system through
// UNICODOID_UCD_AUX_DIR.

#include "unicodoid/unicodoid.hpp"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace {

struct TestCase {
    std::u32string cps;
    std::vector<size_t> boundaries;
};

bool is_hex_token(const std::string& token) {
    if (token.empty()) return false;
    for (char ch : token) {
        const bool hex = (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') ||
                         (ch >= 'A' && ch <= 'F');
        if (!hex) return false;
    }
    return true;
}

std::vector<TestCase> parse_break_test(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("cannot open " + path);
    }

    std::vector<TestCase> cases;
    std::string raw;
    int line_number = 0;

    while (std::getline(in, raw)) {
        ++line_number;

        const std::size_t hash = raw.find('#');
        std::string data = (hash == std::string::npos) ? raw : raw.substr(0, hash);

        std::istringstream tokens(data);
        std::vector<std::string> list;
        std::string token;
        while (tokens >> token) list.push_back(token);
        if (list.empty()) continue;

        TestCase tc;
        size_t pos = 0;
        std::size_t idx = 0;

        // The line break file marks the implicit start boundary with a
        // multiplication sign instead of a division sign.
        if (!is_hex_token(list[0])) {
            if (list[0] == u8"\u00F7") tc.boundaries.push_back(0);
            idx = 1;
        }

        while (idx < list.size()) {
            if (!is_hex_token(list[idx])) {
                throw std::runtime_error(path + ":" + std::to_string(line_number) +
                                         ": expected a code point, got '" +
                                         list[idx] + "'");
            }
            const unsigned long value = std::stoul(list[idx], nullptr, 16);
            tc.cps.push_back(static_cast<char32_t>(value));
            ++pos;
            ++idx;

            if (idx < list.size()) {
                if (!is_hex_token(list[idx]) && list[idx] == u8"\u00F7") {
                    tc.boundaries.push_back(pos);
                }
                ++idx;
            }
        }

        // Every boundary type has a start-of-text boundary.
        if (tc.boundaries.empty() || tc.boundaries.front() != 0) {
            tc.boundaries.insert(tc.boundaries.begin(), 0);
        }

        cases.push_back(std::move(tc));
    }

    return cases;
}

void run_suite(const std::string& file, unicodoid::BoundaryType type,
               int& failures, int& checks) {
    const std::string path = std::string(UNICODOID_UCD_AUX_DIR) + "/" + file;
    const std::vector<TestCase> cases = parse_break_test(path);

    if (cases.empty()) {
        std::fprintf(stderr, "FAIL %s: no test cases\n", file.c_str());
        ++failures;
        return;
    }

    int suite_failures = 0;
    for (std::size_t index = 0; index < cases.size(); ++index) {
        ++checks;
        const TestCase& tc = cases[index];
        const std::vector<size_t> actual =
            unicodoid::Segmenter::boundaries(tc.cps, type);

        if (actual == tc.boundaries) continue;

        ++suite_failures;
        ++failures;
        if (suite_failures <= 10) {
            std::fprintf(stderr, "%s case %zu: cps=[", file.c_str(), index);
            for (char32_t cp : tc.cps) {
                std::fprintf(stderr, "%sU+%04X",
                             (&cp == &tc.cps[0]) ? "" : ", ",
                             static_cast<unsigned>(cp));
            }
            std::fprintf(stderr, "]\n  expected=[");
            for (size_t i = 0; i < tc.boundaries.size(); ++i) {
                std::fprintf(stderr, "%s%zu", i ? ", " : "", tc.boundaries[i]);
            }
            std::fprintf(stderr, "]\n  actual  =[");
            for (size_t i = 0; i < actual.size(); ++i) {
                std::fprintf(stderr, "%s%zu", i ? ", " : "", actual[i]);
            }
            std::fprintf(stderr, "]\n");
        }
    }

    if (suite_failures == 0) {
        std::printf("%s: %zu cases passed\n", file.c_str(), cases.size());
    } else {
        std::fprintf(stderr, "%s: %d of %zu cases failed\n", file.c_str(),
                     suite_failures, cases.size());
    }
}

}  // namespace

int main() {
    int failures = 0;
    int checks = 0;

    try {
        run_suite("GraphemeBreakTest.txt", unicodoid::BoundaryType::Grapheme,
                  failures, checks);
        run_suite("WordBreakTest.txt", unicodoid::BoundaryType::Word, failures,
                  checks);
        run_suite("LineBreakTest.txt", unicodoid::BoundaryType::Line, failures,
                  checks);
        run_suite("SentenceBreakTest.txt", unicodoid::BoundaryType::Sentence,
                  failures, checks);
    } catch (const std::exception& ex) {
        std::fprintf(stderr, "unexpected exception: %s\n", ex.what());
        return 1;
    }

    if (failures == 0) {
        std::printf("test_cpp_conformance: %d cases passed\n", checks);
        return 0;
    }

    std::fprintf(stderr, "test_cpp_conformance: %d of %d cases failed\n",
                 failures, checks);
    return 1;
}
